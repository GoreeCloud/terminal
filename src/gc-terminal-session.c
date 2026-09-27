#include "gc-terminal-session.h"

#include "gc-context.h"

#include <pango/pango.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <vte/vte.h>

struct _GcTerminalSession {
    GtkWidget *root;
    VteTerminal *terminal;
    char *working_directory;
    char *title;
    char *status;
    GcTerminalSessionChangedFunc changed;
    GcTerminalSessionPasteRequestedFunc paste_requested;
    GcTerminalSessionOpenRequestedFunc open_requested;
    gint url_match_tag;
    gint path_match_tag;
    gpointer user_data;
};

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

static void
update_working_directory(VteTerminal *terminal, gpointer user_data)
{
    GcTerminalSession *session = user_data;
    const char *uri = vte_terminal_get_current_directory_uri(terminal);
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
    const char *title = vte_terminal_get_window_title(terminal);

    set_text(&session->title, title != NULL && *title != '\0' ? title : "Terminal");
    notify_changed(session);
}

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

    if (session->paste_requested != NULL) {
        session->paste_requested(session, session->user_data);
    }
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

    g_free(session->working_directory);
    g_free(session->title);
    g_free(session->status);
    g_free(session);
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
    GcTerminalSession *session = g_new0(GcTerminalSession, 1);
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *terminal_widget = vte_terminal_new();
    GdkRGBA foreground = {0};
    GdkRGBA background = {0};
    PangoFontDescription *font;
    const char *shell = gc_context_shell();
    const char *initial_directory =
        working_directory != NULL && *working_directory != '\0'
            ? working_directory
            : g_get_home_dir();
    char *argv[] = {(char *) shell, NULL};

    session->root = scroller;
    session->terminal = VTE_TERMINAL(terminal_widget);
    session->working_directory = g_strdup(initial_directory);
    session->title = g_strdup("Terminal");
    session->status = g_strdup("Starting shell");
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

    gdk_rgba_parse(&foreground, "#e6edf3");
    gdk_rgba_parse(&background, "#0b1017");
    vte_terminal_set_colors(session->terminal, &foreground, &background, NULL, 0);
    vte_terminal_set_scrollback_lines(session->terminal, 10000);
    vte_terminal_set_allow_hyperlink(session->terminal, TRUE);
    vte_terminal_set_bold_is_bright(session->terminal, TRUE);
    vte_terminal_set_cursor_blink_mode(session->terminal, VTE_CURSOR_BLINK_SYSTEM);
    vte_terminal_search_set_wrap_around(session->terminal, TRUE);

    font = pango_font_description_from_string("Monospace 11");
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
        NULL,
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

gboolean
gc_terminal_session_has_focus(GcTerminalSession *session)
{
    return gtk_widget_has_focus(GTK_WIDGET(session->terminal));
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
