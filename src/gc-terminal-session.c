#include "gc-terminal-session.h"

#include "gc-context.h"
#include "gc-shell-state.h"

#include <pango/pango.h>
#include <string.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <vte/vte.h>

struct _GcTerminalSession {
    GtkWidget *root;
    VteTerminal *terminal;
    char *working_directory;
    char *profile_id;
    char *title;
    char *status;
    GcShellState shell_state;
    GcTerminalSessionChangedFunc changed;
    GcTerminalSessionPasteRequestedFunc paste_requested;
    GcTerminalSessionOpenRequestedFunc open_requested;
    gint url_match_tag;
    gint path_match_tag;
    guint notify_idle_id;
    gpointer user_data;
};

static char **
build_spawn_environment(const GcProfile *profile)
{
    char **environment = g_get_environ();
    g_auto(GStrv) overrides = gc_profile_dup_environment(profile);

    for (guint i = 0;
         overrides != NULL && overrides[i] != NULL;
         i++) {
        const char *equals = strchr(overrides[i], '=');
        g_autofree char *name = NULL;

        if (equals == NULL) {
            continue;
        }

        name = g_strndup(
            overrides[i],
            (gsize) (equals - overrides[i])
        );
        environment = g_environ_setenv(
            environment,
            name,
            equals + 1,
            TRUE
        );
    }

    return environment;
}

static void
notify_changed(GcTerminalSession *session)
{
    if (session->changed != NULL) {
        session->changed(session, session->user_data);
    }
}

static void
set_text(char **target, const char *value)
{
    g_free(*target);
    *target = g_strdup(value != NULL ? value : "");
}

static gboolean
notify_changed_idle(gpointer user_data)
{
    GcTerminalSession *session = user_data;

    session->notify_idle_id = 0;
    notify_changed(session);
    return G_SOURCE_REMOVE;
}

static void
queue_notify_changed(GcTerminalSession *session)
{
    if (session->notify_idle_id == 0) {
        session->notify_idle_id = g_idle_add(notify_changed_idle, session);
    }
}

#if VTE_CHECK_VERSION(0, 78, 0)
static void
sync_shell_status(GcTerminalSession *session)
{
    g_autofree char *status = gc_shell_state_dup_status(&session->shell_state);

    set_text(&session->status, status);
    queue_notify_changed(session);
}
#endif

static char *
dup_current_directory_uri(VteTerminal *terminal)
{
#if VTE_CHECK_VERSION(0, 78, 0)
    GUri *uri = vte_terminal_ref_termprop_uri(
        terminal,
        VTE_TERMPROP_CURRENT_DIRECTORY_URI
    );
    char *value = NULL;

    if (uri != NULL) {
        value = g_uri_to_string(uri);
        g_uri_unref(uri);
    }

    return value;
#else
    return g_strdup(vte_terminal_get_current_directory_uri(terminal));
#endif
}

static char *
dup_window_title(VteTerminal *terminal)
{
#if VTE_CHECK_VERSION(0, 78, 0)
    return vte_terminal_dup_termprop_string(
        terminal,
        VTE_TERMPROP_XTERM_TITLE,
        NULL
    );
#else
    return g_strdup(vte_terminal_get_window_title(terminal));
#endif
}

static void
update_working_directory(VteTerminal *terminal, gpointer user_data)
{
    GcTerminalSession *session = user_data;
    g_autofree char *uri = dup_current_directory_uri(terminal);
    g_autofree char *path = NULL;

    if (uri != NULL) {
        path = g_filename_from_uri(uri, NULL, NULL);
    }

    if (path != NULL && *path != '\0') {
        set_text(&session->working_directory, path);
    }

    notify_changed(session);
}

static void
update_window_title(VteTerminal *terminal, gpointer user_data)
{
    GcTerminalSession *session = user_data;
    g_autofree char *title = dup_window_title(terminal);

    set_text(&session->title, title != NULL && *title != '\0' ? title : "Terminal");
    notify_changed(session);
}

#if VTE_CHECK_VERSION(0, 78, 0)
static void
on_termprop_changed(
    VteTerminal *terminal,
    const char *property,
    gpointer user_data
)
{
    GcTerminalSession *session = user_data;

    if (g_strcmp0(property, VTE_TERMPROP_CURRENT_DIRECTORY_URI) == 0) {
        g_autofree char *uri = dup_current_directory_uri(terminal);
        g_autofree char *path = NULL;

        if (uri != NULL) {
            path = g_filename_from_uri(uri, NULL, NULL);
        }

        if (path != NULL && *path != '\0') {
            set_text(&session->working_directory, path);
        }

        queue_notify_changed(session);
    } else if (g_strcmp0(property, VTE_TERMPROP_XTERM_TITLE) == 0) {
        g_autofree char *title = dup_window_title(terminal);

        set_text(
            &session->title,
            title != NULL && *title != '\0' ? title : "Terminal"
        );
        queue_notify_changed(session);
    } else if (g_strcmp0(property, VTE_TERMPROP_SHELL_PRECMD) == 0) {
        g_autoptr(GVariant) value =
            vte_terminal_ref_termprop_variant(terminal, property);

        if (value != NULL) {
            gc_shell_state_mark_prompt(&session->shell_state);
            sync_shell_status(session);
        }
    } else if (g_strcmp0(property, VTE_TERMPROP_SHELL_PREEXEC) == 0) {
        g_autoptr(GVariant) value =
            vte_terminal_ref_termprop_variant(terminal, property);

        if (value != NULL) {
            gc_shell_state_mark_preexec(&session->shell_state);
            sync_shell_status(session);
        }
    } else if (g_strcmp0(property, VTE_TERMPROP_SHELL_POSTEXEC) == 0) {
        guint64 exit_status = 0;

        if (vte_terminal_get_termprop_uint(
                terminal,
                property,
                &exit_status
            )) {
            gboolean valid_exit_status = exit_status <= 255;

            gc_shell_state_mark_postexec(
                &session->shell_state,
                valid_exit_status,
                exit_status
            );
            sync_shell_status(session);
        }
    }
}
#endif

static void
on_focus_changed(GObject *object, GParamSpec *pspec, gpointer user_data)
{
    GcTerminalSession *session = user_data;
    (void) object;
    (void) pspec;

    notify_changed(session);
}

static void
on_paste_clipboard(VteTerminal *terminal, gpointer user_data)
{
    GcTerminalSession *session = user_data;

    g_signal_stop_emission_by_name(terminal, "paste-clipboard");

    gc_terminal_session_request_paste(
        session,
        GC_TERMINAL_PASTE_CLIPBOARD
    );
}

static void
on_primary_paste_pressed(
    GtkGestureClick *gesture,
    gint n_press,
    gdouble x,
    gdouble y,
    gpointer user_data
)
{
    GcTerminalSession *session = user_data;
    (void) x;
    (void) y;

    if (n_press != 1 || session->paste_requested == NULL) {
        return;
    }

    gc_terminal_session_request_paste(
        session,
        GC_TERMINAL_PASTE_PRIMARY
    );
    gtk_gesture_set_state(
        GTK_GESTURE(gesture),
        GTK_EVENT_SEQUENCE_CLAIMED
    );
}

static gint
add_match_regex(VteTerminal *terminal, const char *pattern)
{
    GError *error = NULL;
    VteRegex *regex = vte_regex_new_for_match(
        pattern,
        -1,
        VTE_REGEX_FLAGS_DEFAULT | PCRE2_MULTILINE,
        &error
    );
    gint tag;

    if (regex == NULL) {
        g_warning("Unable to compile terminal match regex: %s", error->message);
        g_clear_error(&error);
        return -1;
    }

    tag = vte_terminal_match_add_regex(terminal, regex, 0);
    vte_regex_unref(regex);

    if (tag >= 0) {
        vte_terminal_match_set_cursor_name(terminal, tag, "pointer");
    }

    return tag;
}

static void
on_terminal_pressed(
    GtkGestureClick *gesture,
    gint n_press,
    gdouble x,
    gdouble y,
    gpointer user_data
)
{
    GcTerminalSession *session = user_data;
    GdkModifierType modifiers;
    GcLinkTargetKind kind = GC_LINK_TARGET_URI;
    gint tag = -1;
    char *target;

    if (n_press != 1 || session->open_requested == NULL) {
        return;
    }

    modifiers = gtk_event_controller_get_current_event_state(
        GTK_EVENT_CONTROLLER(gesture)
    );
    if ((modifiers & GDK_CONTROL_MASK) == 0) {
        return;
    }

    target = vte_terminal_check_hyperlink_at(session->terminal, x, y);

    if (target == NULL) {
        target = vte_terminal_check_match_at(session->terminal, x, y, &tag);

        if (target == NULL) {
            return;
        }

        if (tag == session->url_match_tag) {
            kind = GC_LINK_TARGET_URI;
        } else if (tag == session->path_match_tag) {
            kind = GC_LINK_TARGET_PATH;
        } else {
            g_free(target);
            return;
        }
    }

    session->open_requested(session, kind, target, session->user_data);
    gtk_gesture_set_state(GTK_GESTURE(gesture), GTK_EVENT_SEQUENCE_CLAIMED);
    g_free(target);
}

static void
on_child_exited(VteTerminal *terminal, gint status, gpointer user_data)
{
    GcTerminalSession *session = user_data;
    (void) terminal;
    (void) status;

    set_text(&session->status, "Shell exited");
    notify_changed(session);
}

static void
on_spawn_finished(VteTerminal *terminal, GPid pid, GError *error, gpointer user_data)
{
    GcTerminalSession *session = user_data;
    (void) terminal;
    (void) pid;

    if (error != NULL) {
        g_autofree char *message = g_strdup_printf("Shell failed: %s", error->message);
        set_text(&session->status, message);
    } else {
        set_text(&session->status, "Session active");
    }

    notify_changed(session);
}

static void
session_free(gpointer data)
{
    GcTerminalSession *session = data;

    if (session->notify_idle_id != 0) {
        g_source_remove(session->notify_idle_id);
    }

    g_free(session->working_directory);
    g_free(session->profile_id);
    g_free(session->title);
    g_free(session->status);
    g_free(session);
}

GcTerminalSession *
gc_terminal_session_new_with_profile(
    const GcProfile *profile,
    const char *working_directory,
    GcTerminalSessionChangedFunc changed,
    GcTerminalSessionPasteRequestedFunc paste_requested,
    GcTerminalSessionOpenRequestedFunc open_requested,
    gpointer user_data
)
{
    GcTerminalSession *session = g_new0(GcTerminalSession, 1);
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *terminal_widget = vte_terminal_new();
    GdkRGBA foreground = {0};
    GdkRGBA background = {0};
    PangoFontDescription *font;
    const char *configured_shell = gc_profile_get_shell(profile);
    const char *shell = gc_context_shell();
    g_autofree char *profile_directory = NULL;
    const char *initial_directory;
    g_auto(GStrv) environment = NULL;
    char *argv[2];

    if (configured_shell != NULL &&
        g_file_test(configured_shell, G_FILE_TEST_IS_EXECUTABLE)) {
        shell = configured_shell;
    } else if (configured_shell != NULL) {
        g_warning(
            "Profile shell is not executable, using the login shell instead: %s",
            configured_shell
        );
    }

    if (working_directory != NULL && *working_directory != '\0') {
        profile_directory = g_strdup(working_directory);
    } else {
        profile_directory = gc_profile_dup_effective_working_directory(
            profile,
            g_get_home_dir()
        );
    }
    initial_directory = profile_directory;
    argv[0] = (char *) shell;
    argv[1] = NULL;
    environment = build_spawn_environment(profile);

    session->root = scroller;
    session->terminal = VTE_TERMINAL(terminal_widget);
    session->working_directory = g_strdup(initial_directory);
    session->profile_id = g_strdup(gc_profile_get_id(profile));
    session->title = g_strdup("Terminal");
    session->status = g_strdup("Starting shell");
    gc_shell_state_init(&session->shell_state);
    session->changed = changed;
    session->paste_requested = paste_requested;
    session->open_requested = open_requested;
    session->url_match_tag = -1;
    session->path_match_tag = -1;
    session->user_data = user_data;

    g_object_set_data_full(
        G_OBJECT(scroller),
        "goreecloud-terminal-session",
        session,
        session_free
    );

    gtk_widget_add_css_class(scroller, "terminal-frame");
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroller),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC
    );
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), terminal_widget);
    gtk_widget_set_hexpand(scroller, TRUE);
    gtk_widget_set_vexpand(scroller, TRUE);

    if (!gdk_rgba_parse(&foreground, gc_profile_get_foreground(profile))) {
        gdk_rgba_parse(&foreground, "#dceaff");
    }
    if (!gdk_rgba_parse(&background, gc_profile_get_background(profile))) {
        gdk_rgba_parse(&background, "#050d18");
    }
    vte_terminal_set_colors(session->terminal, &foreground, &background, NULL, 0);
    vte_terminal_set_scrollback_lines(session->terminal, 10000);
    vte_terminal_set_allow_hyperlink(session->terminal, TRUE);
#if VTE_CHECK_VERSION(0, 78, 0)
    vte_terminal_set_enable_legacy_osc777(session->terminal, TRUE);
#endif
    vte_terminal_set_bold_is_bright(session->terminal, TRUE);
    vte_terminal_set_cursor_blink_mode(session->terminal, VTE_CURSOR_BLINK_SYSTEM);
    vte_terminal_search_set_wrap_around(session->terminal, TRUE);

    font = pango_font_description_from_string(gc_profile_get_font(profile));
    vte_terminal_set_font(session->terminal, font);
    pango_font_description_free(font);

    session->url_match_tag = add_match_regex(
        session->terminal,
        "(?:https?://|mailto:|file://)[^[:space:]<>\\\"']+"
    );
    session->path_match_tag = add_match_regex(
        session->terminal,
        "(?<![[:alnum:]:/])(?:~|\\.{1,2})?/[^[:space:]<>\\\"']+"
    );

    {
        GtkGesture *click = gtk_gesture_click_new();

        gtk_gesture_single_set_button(
            GTK_GESTURE_SINGLE(click),
            GDK_BUTTON_PRIMARY
        );
        gtk_event_controller_set_propagation_phase(
            GTK_EVENT_CONTROLLER(click),
            GTK_PHASE_CAPTURE
        );
        g_signal_connect(
            click,
            "pressed",
            G_CALLBACK(on_terminal_pressed),
            session
        );
        gtk_widget_add_controller(
            GTK_WIDGET(session->terminal),
            GTK_EVENT_CONTROLLER(click)
        );
    }

    {
        GtkGesture *middle_click = gtk_gesture_click_new();

        gtk_gesture_single_set_button(
            GTK_GESTURE_SINGLE(middle_click),
            GDK_BUTTON_MIDDLE
        );
        gtk_event_controller_set_propagation_phase(
            GTK_EVENT_CONTROLLER(middle_click),
            GTK_PHASE_CAPTURE
        );
        g_signal_connect(
            middle_click,
            "pressed",
            G_CALLBACK(on_primary_paste_pressed),
            session
        );
        gtk_widget_add_controller(
            GTK_WIDGET(session->terminal),
            GTK_EVENT_CONTROLLER(middle_click)
        );
    }

#if VTE_CHECK_VERSION(0, 78, 0)
    g_signal_connect(
        session->terminal,
        "termprop-changed",
        G_CALLBACK(on_termprop_changed),
        session
    );
#else
    g_signal_connect(
        session->terminal,
        "current-directory-uri-changed",
        G_CALLBACK(update_working_directory),
        session
    );
    g_signal_connect(
        session->terminal,
        "window-title-changed",
        G_CALLBACK(update_window_title),
        session
    );
#endif
    g_signal_connect(
        session->terminal,
        "notify::has-focus",
        G_CALLBACK(on_focus_changed),
        session
    );
    g_signal_connect(
        session->terminal,
        "paste-clipboard",
        G_CALLBACK(on_paste_clipboard),
        session
    );
    g_signal_connect(
        session->terminal,
        "child-exited",
        G_CALLBACK(on_child_exited),
        session
    );

    vte_terminal_spawn_async(
        session->terminal,
        VTE_PTY_DEFAULT,
        initial_directory,
        argv,
        environment,
        G_SPAWN_DEFAULT,
        NULL,
        NULL,
        NULL,
        -1,
        NULL,
        on_spawn_finished,
        session
    );

    return session;
}

GcTerminalSession *
gc_terminal_session_new(
    const char *working_directory,
    GcTerminalSessionChangedFunc changed,
    GcTerminalSessionPasteRequestedFunc paste_requested,
    GcTerminalSessionOpenRequestedFunc open_requested,
    gpointer user_data
)
{
    return gc_terminal_session_new_with_profile(
        NULL,
        working_directory,
        changed,
        paste_requested,
        open_requested,
        user_data
    );
}

GtkWidget *
gc_terminal_session_get_widget(GcTerminalSession *session)
{
    return session->root;
}

const char *
gc_terminal_session_get_status(GcTerminalSession *session)
{
    return session->status;
}

char *
gc_terminal_session_dup_working_directory(GcTerminalSession *session)
{
    return g_strdup(
        session->working_directory != NULL && *session->working_directory != '\0'
            ? session->working_directory
            : g_get_home_dir()
    );
}

char *
gc_terminal_session_dup_display_title(GcTerminalSession *session)
{
    if (session->title != NULL && *session->title != '\0' &&
        g_strcmp0(session->title, "Terminal") != 0) {
        return g_strdup(session->title);
    }

    if (session->working_directory != NULL && *session->working_directory != '\0') {
        g_autofree char *basename = g_path_get_basename(session->working_directory);
        if (basename != NULL && *basename != '\0') {
            return g_strdup(basename);
        }
    }

    return g_strdup("Terminal");
}
const char *
gc_terminal_session_get_profile_id(GcTerminalSession *session)
{
    return session != NULL && session->profile_id != NULL
        ? session->profile_id
        : "default";
}


gboolean
gc_terminal_session_has_focus(GcTerminalSession *session)
{
    return gtk_widget_has_focus(GTK_WIDGET(session->terminal));
}

gboolean
gc_terminal_session_has_shell_integration(GcTerminalSession *session)
{
    return gc_shell_state_is_integrated(&session->shell_state);
}

gboolean
gc_terminal_session_is_command_running(GcTerminalSession *session)
{
    return gc_shell_state_is_running(&session->shell_state);
}

gboolean
gc_terminal_session_get_last_exit_status(
    GcTerminalSession *session,
    guint64 *exit_status
)
{
    return gc_shell_state_get_last_exit_status(
        &session->shell_state,
        exit_status
    );
}

void
gc_terminal_session_copy(GcTerminalSession *session)
{
    if (vte_terminal_get_has_selection(session->terminal)) {
        vte_terminal_copy_clipboard_format(session->terminal, VTE_FORMAT_TEXT);
    }
}

void
gc_terminal_session_paste(GcTerminalSession *session)
{
    vte_terminal_paste_clipboard(session->terminal);
}

void
gc_terminal_session_request_paste(
    GcTerminalSession *session,
    GcTerminalPasteSource source
)
{
    if (session->paste_requested != NULL) {
        session->paste_requested(
            session,
            source,
            session->user_data
        );
    }
}

void
gc_terminal_session_paste_text(GcTerminalSession *session, const char *text)
{
    if (text == NULL || *text == '\0') {
        return;
    }

    vte_terminal_paste_text(session->terminal, text);
}

void
gc_terminal_session_focus(GcTerminalSession *session)
{
    gtk_widget_grab_focus(GTK_WIDGET(session->terminal));
}

gboolean
gc_terminal_session_set_search(
    GcTerminalSession *session,
    const char *pattern,
    gboolean regex_enabled,
    gboolean case_sensitive,
    GError **error
)
{
    g_autofree char *escaped = NULL;
    g_autofree char *compiled_pattern = NULL;
    VteRegex *regex;

    if (pattern == NULL || *pattern == '\0') {
        gc_terminal_session_clear_search(session);
        return TRUE;
    }

    escaped = regex_enabled
        ? g_strdup(pattern)
        : g_regex_escape_string(pattern, -1);

    compiled_pattern = g_strdup_printf(
        case_sensitive ? "(?m:%s)" : "(?im:%s)",
        escaped
    );

    regex = vte_regex_new_for_search(compiled_pattern, -1, 0, error);
    if (regex == NULL) {
        return FALSE;
    }

    vte_terminal_search_set_regex(session->terminal, regex, 0);
    vte_regex_unref(regex);
    return TRUE;
}

void
gc_terminal_session_clear_search(GcTerminalSession *session)
{
    vte_terminal_search_set_regex(session->terminal, NULL, 0);
}

gboolean
gc_terminal_session_search_next(GcTerminalSession *session)
{
    return vte_terminal_search_find_next(session->terminal);
}

gboolean
gc_terminal_session_search_previous(GcTerminalSession *session)
{
    return vte_terminal_search_find_previous(session->terminal);
}
