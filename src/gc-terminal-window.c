#include "gc-terminal-window.h"

#include "gc-context.h"

#include <pango/pango.h>
#include <vte/vte.h>

typedef struct {
    GtkWindow *window;
    VteTerminal *terminal;
    GtkLabel *cwd_label;
    GtkLabel *session_label;
} TerminalWindowState;

static const char *development_css =
    ".gc-window { background: #0b1017; }"
    ".gc-header { background: rgba(15, 23, 34, 0.94); border-bottom: 1px solid rgba(122, 162, 247, 0.18); }"
    ".gc-title { font-weight: 700; letter-spacing: 0.02em; }"
    ".gc-subtitle { color: rgba(226, 232, 240, 0.66); font-size: 0.90em; }"
    ".context-bar { padding: 8px 12px; border-bottom: 1px solid rgba(122, 162, 247, 0.14); background: rgba(17, 26, 38, 0.82); }"
    ".context-chip { padding: 4px 9px; border-radius: 999px; background: rgba(122, 162, 247, 0.10); border: 1px solid rgba(122, 162, 247, 0.18); }"
    ".context-chip-elevated { background: rgba(244, 114, 182, 0.12); border-color: rgba(244, 114, 182, 0.28); }"
    ".context-path { color: rgba(226, 232, 240, 0.78); }"
    ".terminal-frame { padding: 8px; }";

static void
install_development_style(void)
{
    GdkDisplay *display = gdk_display_get_default();
    GtkCssProvider *provider;

    if (display == NULL) {
        return;
    }

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, development_css);
    gtk_style_context_add_provider_for_display(
        display,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

static void
update_working_directory(VteTerminal *terminal, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    const char *uri = vte_terminal_get_current_directory_uri(terminal);
    g_autofree char *path = NULL;

    if (uri != NULL) {
        path = g_filename_from_uri(uri, NULL, NULL);
    }

    gtk_label_set_text(state->cwd_label, path != NULL ? path : g_get_home_dir());
}

static void
update_window_title(VteTerminal *terminal, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    const char *title = vte_terminal_get_window_title(terminal);

    if (title == NULL || *title == '\0') {
        gtk_window_set_title(state->window, "GoreeCloud Terminal");
        return;
    }

    g_autofree char *composed = g_strdup_printf("%s — GoreeCloud Terminal", title);
    gtk_window_set_title(state->window, composed);
}

static void
on_child_exited(VteTerminal *terminal, gint status, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) terminal;
    (void) status;

    gtk_label_set_text(state->session_label, "Shell exited");
}

static void
on_spawn_finished(VteTerminal *terminal, GPid pid, GError *error, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) terminal;
    (void) pid;

    if (error != NULL) {
        g_autofree char *message = g_strdup_printf("Shell failed: %s", error->message);
        gtk_label_set_text(state->session_label, message);
        return;
    }

    gtk_label_set_text(state->session_label, "Session active");
}

static void
copy_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    if (vte_terminal_get_has_selection(state->terminal)) {
        vte_terminal_copy_clipboard_format(state->terminal, VTE_FORMAT_TEXT);
    }
}

static void
paste_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    vte_terminal_paste_clipboard(state->terminal);
}

static GtkWidget *
build_header(GtkApplication *application, GtkWindow *window, TerminalWindowState *state)
{
    GtkWidget *header = gtk_header_bar_new();
    GtkWidget *title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *title = gtk_label_new("GoreeCloud Terminal");
    GtkWidget *subtitle = gtk_label_new("Development · local session foundation");
    GtkWidget *copy_button = gtk_button_new_from_icon_name("edit-copy-symbolic");
    GtkWidget *paste_button = gtk_button_new_from_icon_name("edit-paste-symbolic");
    const char *copy_accels[] = {"<Control><Shift>c", NULL};
    const char *paste_accels[] = {"<Control><Shift>v", NULL};
    const GActionEntry actions[] = {
        {"copy", copy_action, NULL, NULL, NULL},
        {"paste", paste_action, NULL, NULL, NULL},
    };

    gtk_widget_add_css_class(header, "gc-header");
    gtk_widget_add_css_class(title, "gc-title");
    gtk_widget_add_css_class(subtitle, "gc-subtitle");

    gtk_box_append(GTK_BOX(title_box), title);
    gtk_box_append(GTK_BOX(title_box), subtitle);
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), title_box);

    gtk_widget_set_tooltip_text(copy_button, "Copy selection");
    gtk_widget_set_tooltip_text(paste_button, "Paste clipboard");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(copy_button), "win.copy");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(paste_button), "win.paste");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), paste_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), copy_button);

    g_action_map_add_action_entries(G_ACTION_MAP(window), actions, G_N_ELEMENTS(actions), state);
    gtk_application_set_accels_for_action(application, "win.copy", copy_accels);
    gtk_application_set_accels_for_action(application, "win.paste", paste_accels);

    return header;
}

static GtkWidget *
build_context_bar(TerminalWindowState *state)
{
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *scope = gtk_label_new(gc_context_privilege_label());
    g_autofree char *identity = gc_context_identity();
    GtkWidget *identity_label = gtk_label_new(identity);

    state->cwd_label = GTK_LABEL(gtk_label_new(g_get_home_dir()));
    state->session_label = GTK_LABEL(gtk_label_new("Starting shell"));

    gtk_widget_add_css_class(bar, "context-bar");
    gtk_widget_add_css_class(scope, "context-chip");
    gtk_widget_add_css_class(identity_label, "context-chip");
    gtk_widget_add_css_class(GTK_WIDGET(state->session_label), "context-chip");
    gtk_widget_add_css_class(GTK_WIDGET(state->cwd_label), "context-path");

    if (g_strcmp0(gc_context_privilege_label(), "Elevated local") == 0) {
        gtk_widget_add_css_class(scope, "context-chip-elevated");
    }

    gtk_label_set_ellipsize(state->cwd_label, PANGO_ELLIPSIZE_MIDDLE);
    gtk_label_set_xalign(state->cwd_label, 0.0f);
    gtk_widget_set_hexpand(GTK_WIDGET(state->cwd_label), TRUE);

    gtk_box_append(GTK_BOX(bar), scope);
    gtk_box_append(GTK_BOX(bar), identity_label);
    gtk_box_append(GTK_BOX(bar), GTK_WIDGET(state->cwd_label));
    gtk_box_append(GTK_BOX(bar), GTK_WIDGET(state->session_label));

    return bar;
}

static void
configure_terminal(TerminalWindowState *state)
{
    GdkRGBA foreground = {0};
    GdkRGBA background = {0};
    PangoFontDescription *font;
    const char *shell = gc_context_shell();
    char *argv[] = {(char *) shell, NULL};

    gdk_rgba_parse(&foreground, "#e6edf3");
    gdk_rgba_parse(&background, "#0b1017");

    vte_terminal_set_colors(state->terminal, &foreground, &background, NULL, 0);
    vte_terminal_set_scrollback_lines(state->terminal, 10000);
    vte_terminal_set_allow_hyperlink(state->terminal, TRUE);
    vte_terminal_set_bold_is_bright(state->terminal, TRUE);
    vte_terminal_set_cursor_blink_mode(state->terminal, VTE_CURSOR_BLINK_SYSTEM);

    font = pango_font_description_from_string("Monospace 11");
    vte_terminal_set_font(state->terminal, font);
    pango_font_description_free(font);

    g_signal_connect(state->terminal, "current-directory-uri-changed", G_CALLBACK(update_working_directory), state);
    g_signal_connect(state->terminal, "window-title-changed", G_CALLBACK(update_window_title), state);
    g_signal_connect(state->terminal, "child-exited", G_CALLBACK(on_child_exited), state);

    vte_terminal_spawn_async(
        state->terminal,
        VTE_PTY_DEFAULT,
        g_get_home_dir(),
        argv,
        NULL,
        G_SPAWN_DEFAULT,
        NULL,
        NULL,
        NULL,
        -1,
        NULL,
        on_spawn_finished,
        state
    );
}

GtkWindow *
gc_terminal_window_new(GtkApplication *application)
{
    TerminalWindowState *state = g_new0(TerminalWindowState, 1);
    GtkWidget *window = gtk_application_window_new(application);
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *terminal_widget = vte_terminal_new();

    state->window = GTK_WINDOW(window);
    state->terminal = VTE_TERMINAL(terminal_widget);

    g_object_set_data_full(G_OBJECT(window), "goreecloud-terminal-window-state", state, g_free);

    install_development_style();

    gtk_widget_add_css_class(window, "gc-window");
    gtk_window_set_default_size(GTK_WINDOW(window), 1040, 680);
    gtk_window_set_title(GTK_WINDOW(window), "GoreeCloud Terminal");
    gtk_window_set_titlebar(GTK_WINDOW(window), build_header(application, GTK_WINDOW(window), state));

    gtk_box_append(GTK_BOX(root), build_context_bar(state));

    gtk_widget_add_css_class(scroller, "terminal-frame");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), terminal_widget);
    gtk_widget_set_hexpand(scroller, TRUE);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_box_append(GTK_BOX(root), scroller);

    gtk_window_set_child(GTK_WINDOW(window), root);

    configure_terminal(state);
    return GTK_WINDOW(window);
}
