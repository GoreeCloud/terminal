#include "gc-terminal-session.h"

#include "gc-context.h"

#include <pango/pango.h>
#include <vte/vte.h>

struct _GcTerminalSession {
    GtkWidget *root;
    VteTerminal *terminal;
    char *working_directory;
    char *title;
    char *status;
    GcTerminalSessionChangedFunc changed;
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

    font = pango_font_description_from_string("Monospace 11");
    vte_terminal_set_font(session->terminal, font);
    pango_font_description_free(font);

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
gc_terminal_session_focus(GcTerminalSession *session)
{
    gtk_widget_grab_focus(GTK_WIDGET(session->terminal));
}
