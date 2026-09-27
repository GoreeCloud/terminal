#include "gc-terminal-window.h"

#include "gc-context.h"
#include "gc-onboarding.h"
#include "gc-workspace.h"

#include <pango/pango.h>

typedef struct {
    GtkWindow *window;
    GcWorkspace *workspace;
    GtkLabel *subtitle_label;
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
    ".terminal-frame { padding: 8px; }"
    ".onboarding-card { background: rgba(15, 23, 34, 0.98); }"
    ".onboarding-kicker { color: rgba(122, 162, 247, 0.82); font-size: 0.82em; font-weight: 700; letter-spacing: 0.06em; }"
    ".onboarding-title { font-size: 1.55em; font-weight: 700; }"
    ".onboarding-body { color: rgba(226, 232, 240, 0.82); line-height: 1.35; }";

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
state_free(gpointer data)
{
    TerminalWindowState *state = data;

    gc_workspace_free(state->workspace);
    g_free(state);
}

static void
update_context(TerminalWindowState *state)
{
    GcTerminalSession *session = gc_workspace_get_current_session(state->workspace);
    guint count = gc_workspace_get_count(state->workspace);

    if (session == NULL) {
        gtk_label_set_text(state->cwd_label, g_get_home_dir());
        gtk_label_set_text(state->session_label, "No active session");
        gtk_window_set_title(state->window, "GoreeCloud Terminal");
        return;
    }

    g_autofree char *cwd = gc_terminal_session_dup_working_directory(session);
    g_autofree char *title = gc_terminal_session_dup_display_title(session);
    g_autofree char *window_title = g_strdup_printf(
        "%s — GoreeCloud Terminal",
        title
    );
    g_autofree char *subtitle = g_strdup_printf(
        "Development · %u local tab%s",
        count,
        count == 1 ? "" : "s"
    );

    gtk_label_set_text(state->cwd_label, cwd);
    gtk_label_set_text(state->session_label, gc_terminal_session_get_status(session));
    gtk_label_set_text(state->subtitle_label, subtitle);
    gtk_window_set_title(state->window, window_title);
}

static void
on_workspace_changed(GcWorkspace *workspace, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) workspace;

    update_context(state);
}

static void
new_tab_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    g_autofree char *cwd = gc_workspace_dup_current_working_directory(state->workspace);
    (void) action;
    (void) parameter;

    gc_workspace_add_tab(state->workspace, cwd);
}

static void
close_tab_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    if (!gc_workspace_close_current(state->workspace)) {
        gtk_window_close(state->window);
    }
}

static void
next_tab_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    gc_workspace_select_relative(state->workspace, 1);
}

static void
previous_tab_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    gc_workspace_select_relative(state->workspace, -1);
}

static void
copy_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    GcTerminalSession *session = gc_workspace_get_current_session(state->workspace);
    (void) action;
    (void) parameter;

    if (session != NULL) {
        gc_terminal_session_copy(session);
    }
}

static void
paste_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    GcTerminalSession *session = gc_workspace_get_current_session(state->workspace);
    (void) action;
    (void) parameter;

    if (session != NULL) {
        gc_terminal_session_paste(session);
    }
}

static void
help_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    gc_onboarding_show_replay(state->window);
}

static GtkWidget *
build_header(GtkApplication *application, GtkWindow *window, TerminalWindowState *state)
{
    GtkWidget *header = gtk_header_bar_new();
    GtkWidget *title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *title = gtk_label_new("GoreeCloud Terminal");
    GtkWidget *new_button = gtk_button_new_from_icon_name("list-add-symbolic");
    GtkWidget *close_button = gtk_button_new_from_icon_name("window-close-symbolic");
    GtkWidget *copy_button = gtk_button_new_from_icon_name("edit-copy-symbolic");
    GtkWidget *paste_button = gtk_button_new_from_icon_name("edit-paste-symbolic");
    GtkWidget *help_button = gtk_button_new_from_icon_name("help-about-symbolic");
    const char *new_accels[] = {"<Control><Shift>t", NULL};
    const char *close_accels[] = {"<Control><Shift>w", NULL};
    const char *next_accels[] = {"<Control>Page_Down", NULL};
    const char *previous_accels[] = {"<Control>Page_Up", NULL};
    const char *copy_accels[] = {"<Control><Shift>c", NULL};
    const char *paste_accels[] = {"<Control><Shift>v", NULL};
    const GActionEntry actions[] = {
        {"new-tab", new_tab_action, NULL, NULL, NULL},
        {"close-tab", close_tab_action, NULL, NULL, NULL},
        {"next-tab", next_tab_action, NULL, NULL, NULL},
        {"previous-tab", previous_tab_action, NULL, NULL, NULL},
        {"copy", copy_action, NULL, NULL, NULL},
        {"paste", paste_action, NULL, NULL, NULL},
        {"help", help_action, NULL, NULL, NULL},
    };

    state->subtitle_label = GTK_LABEL(gtk_label_new("Development · 1 local tab"));

    gtk_widget_add_css_class(header, "gc-header");
    gtk_widget_add_css_class(title, "gc-title");
    gtk_widget_add_css_class(GTK_WIDGET(state->subtitle_label), "gc-subtitle");

    gtk_box_append(GTK_BOX(title_box), title);
    gtk_box_append(GTK_BOX(title_box), GTK_WIDGET(state->subtitle_label));
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), title_box);

    gtk_widget_set_tooltip_text(new_button, "New tab");
    gtk_widget_set_tooltip_text(close_button, "Close active tab");
    gtk_widget_set_tooltip_text(copy_button, "Copy selection");
    gtk_widget_set_tooltip_text(paste_button, "Paste clipboard");
    gtk_widget_set_tooltip_text(help_button, "Replay onboarding");

    gtk_actionable_set_action_name(GTK_ACTIONABLE(new_button), "win.new-tab");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(close_button), "win.close-tab");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(copy_button), "win.copy");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(paste_button), "win.paste");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(help_button), "win.help");

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), new_button);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), close_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), help_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), paste_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), copy_button);

    g_action_map_add_action_entries(
        G_ACTION_MAP(window),
        actions,
        G_N_ELEMENTS(actions),
        state
    );

    gtk_application_set_accels_for_action(application, "win.new-tab", new_accels);
    gtk_application_set_accels_for_action(application, "win.close-tab", close_accels);
    gtk_application_set_accels_for_action(application, "win.next-tab", next_accels);
    gtk_application_set_accels_for_action(application, "win.previous-tab", previous_accels);
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

GtkWindow *
gc_terminal_window_new(GtkApplication *application)
{
    TerminalWindowState *state = g_new0(TerminalWindowState, 1);
    GtkWidget *window = gtk_application_window_new(application);
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    state->window = GTK_WINDOW(window);
    state->workspace = gc_workspace_new(on_workspace_changed, state);

    g_object_set_data_full(
        G_OBJECT(window),
        "goreecloud-terminal-window-state",
        state,
        state_free
    );

    install_development_style();

    gtk_widget_add_css_class(window, "gc-window");
    gtk_window_set_default_size(GTK_WINDOW(window), 1120, 720);
    gtk_window_set_title(GTK_WINDOW(window), "GoreeCloud Terminal");
    gtk_window_set_titlebar(
        GTK_WINDOW(window),
        build_header(application, GTK_WINDOW(window), state)
    );

    gtk_box_append(GTK_BOX(root), build_context_bar(state));
    gtk_box_append(GTK_BOX(root), gc_workspace_get_widget(state->workspace));
    gtk_window_set_child(GTK_WINDOW(window), root);

    gc_workspace_add_tab(state->workspace, g_get_home_dir());
    update_context(state);

    return GTK_WINDOW(window);
}

void
gc_terminal_window_maybe_show_onboarding(GtkWindow *window)
{
    gc_onboarding_maybe_show(window);
}
