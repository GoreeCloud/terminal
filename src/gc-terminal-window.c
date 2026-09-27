#include "gc-terminal-window.h"

#include "gc-context.h"
#include "gc-onboarding.h"
#include "gc-link-utils.h"
#include "gc-paste-safety.h"
#include "gc-workspace.h"

#include <pango/pango.h>

typedef struct {
    GtkWindow *window;
    GcWorkspace *workspace;
    GtkLabel *subtitle_label;
    GtkLabel *cwd_label;
    GtkLabel *session_label;
    GtkWidget *search_bar;
    GtkSearchEntry *search_entry;
    GtkCheckButton *search_regex;
    GtkCheckButton *search_case;
    GtkLabel *search_status;
} TerminalWindowState;

typedef struct {
    GtkWindow *parent;
    GtkWidget *session_widget;
} PasteReadRequest;

typedef struct {
    GtkWindow *window;
    GtkWidget *session_widget;
    char *text;
} PasteReview;

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
    ".search-bar { padding: 8px 12px; background: rgba(15, 23, 34, 0.96); border-bottom: 1px solid rgba(122, 162, 247, 0.16); }"
    ".search-error { color: #fda4af; }"
    ".onboarding-card { background: rgba(15, 23, 34, 0.98); color: #e6edf3; }"
    ".onboarding-kicker { color: rgba(122, 162, 247, 0.82); font-size: 0.82em; font-weight: 700; letter-spacing: 0.06em; }"
    ".onboarding-title { color: #e6edf3; font-size: 1.55em; font-weight: 700; }"
    ".onboarding-body { color: rgba(226, 232, 240, 0.82); }"
    ".paste-warning { color: #fbbf24; font-weight: 700; }";

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
paste_read_request_free(PasteReadRequest *request)
{
    g_clear_object(&request->parent);
    g_clear_object(&request->session_widget);
    g_free(request);
}

static void
paste_review_free(gpointer data)
{
    PasteReview *review = data;

    g_clear_object(&review->session_widget);
    g_free(review->text);
    g_free(review);
}

static void
update_context(TerminalWindowState *state)
{
    GcTerminalSession *session = gc_workspace_get_current_session(state->workspace);
    guint tabs = gc_workspace_get_count(state->workspace);
    guint panes = gc_workspace_get_current_pane_count(state->workspace);

    if (session == NULL) {
        gtk_label_set_text(state->cwd_label, g_get_home_dir());
        gtk_label_set_text(state->session_label, "No active session");
        gtk_window_set_title(state->window, "GoreeCloud Terminal");
        return;
    }

    g_autofree char *cwd = gc_terminal_session_dup_working_directory(session);
    g_autofree char *title = gc_terminal_session_dup_display_title(session);
    g_autofree char *window_title = g_strdup_printf("%s — GoreeCloud Terminal", title);
    g_autofree char *subtitle = g_strdup_printf(
        "Development · %u tab%s · %u pane%s",
        tabs,
        tabs == 1 ? "" : "s",
        panes,
        panes == 1 ? "" : "s"
    );

    gtk_label_set_text(state->cwd_label, cwd);
    gtk_label_set_text(state->session_label, gc_terminal_session_get_status(session));
    gtk_label_set_text(state->subtitle_label, subtitle);
    gtk_window_set_title(state->window, window_title);
}

static gboolean
apply_search(TerminalWindowState *state, gboolean move_to_match)
{
    GcTerminalSession *session = gc_workspace_get_current_session(state->workspace);
    const char *pattern = gtk_editable_get_text(GTK_EDITABLE(state->search_entry));
    gboolean regex_enabled = gtk_check_button_get_active(state->search_regex);
    gboolean case_sensitive = gtk_check_button_get_active(state->search_case);
    GError *error = NULL;
    gboolean found = TRUE;

    gtk_widget_remove_css_class(GTK_WIDGET(state->search_status), "search-error");
    gtk_label_set_text(state->search_status, "");

    if (session == NULL) {
        return FALSE;
    }

    if (!gtk_widget_get_visible(state->search_bar) || pattern == NULL || *pattern == '\0') {
        gc_terminal_session_clear_search(session);
        return TRUE;
    }

    if (!gc_terminal_session_set_search(
            session,
            pattern,
            regex_enabled,
            case_sensitive,
            &error
        )) {
        gtk_widget_add_css_class(GTK_WIDGET(state->search_status), "search-error");
        gtk_label_set_text(
            state->search_status,
            error != NULL ? error->message : "Invalid search expression"
        );
        g_clear_error(&error);
        return FALSE;
    }

    if (move_to_match) {
        found = gc_terminal_session_search_next(session);
        gtk_label_set_text(state->search_status, found ? "" : "No match");
    }

    return found;
}

static void
on_workspace_changed(GcWorkspace *workspace, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) workspace;

    update_context(state);
    apply_search(state, FALSE);
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
split_horizontal_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    gc_workspace_split_current(state->workspace, GTK_ORIENTATION_HORIZONTAL);
}

static void
split_vertical_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    gc_workspace_split_current(state->workspace, GTK_ORIENTATION_VERTICAL);
}

static void
close_pane_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    if (!gc_workspace_close_current_pane(state->workspace)) {
        close_tab_action(NULL, NULL, state);
    }
}

static void
next_pane_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    gc_workspace_focus_relative_pane(state->workspace, 1);
}

static void
previous_pane_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    gc_workspace_focus_relative_pane(state->workspace, -1);
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
on_paste_review_confirm(GtkButton *button, gpointer user_data)
{
    PasteReview *review = user_data;
    GcTerminalSession *session = g_object_get_data(
        G_OBJECT(review->session_widget),
        "goreecloud-terminal-session"
    );
    (void) button;

    if (session != NULL) {
        gc_terminal_session_paste_text(session, review->text);
    }

    gtk_window_destroy(review->window);
}

static void
show_paste_review(
    GtkWindow *parent,
    GtkWidget *session_widget,
    const char *text
)
{
    PasteReview *review = g_new0(PasteReview, 1);
    GtkWidget *window = gtk_window_new();
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *warning = gtk_label_new(
        "This paste contains line breaks and may execute multiple commands."
    );
    GtkWidget *summary;
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *text_view = gtk_text_view_new();
    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *cancel_button = gtk_button_new_with_label("Cancel");
    GtkWidget *paste_button = gtk_button_new_with_label("Paste anyway");
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    guint lines = gc_paste_text_line_count(text);
    glong characters = g_utf8_strlen(text, -1);
    g_autofree char *summary_text = g_strdup_printf(
        "%u line%s · %ld character%s",
        lines,
        lines == 1 ? "" : "s",
        characters,
        characters == 1 ? "" : "s"
    );

    review->window = GTK_WINDOW(window);
    review->session_widget = g_object_ref(session_widget);
    review->text = g_strdup(text);

    g_object_set_data_full(
        G_OBJECT(window),
        "goreecloud-paste-review",
        review,
        paste_review_free
    );

    gtk_window_set_title(review->window, "Review paste — GoreeCloud Terminal");
    gtk_window_set_default_size(review->window, 620, 420);
    gtk_window_set_modal(review->window, TRUE);
    gtk_window_set_transient_for(review->window, parent);
    gtk_window_set_destroy_with_parent(review->window, TRUE);

    gtk_widget_set_margin_top(root, 20);
    gtk_widget_set_margin_bottom(root, 20);
    gtk_widget_set_margin_start(root, 20);
    gtk_widget_set_margin_end(root, 20);

    gtk_widget_add_css_class(warning, "paste-warning");
    gtk_label_set_xalign(GTK_LABEL(warning), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(warning), TRUE);

    summary = gtk_label_new(summary_text);
    gtk_label_set_xalign(GTK_LABEL(summary), 0.0f);

    gtk_text_buffer_set_text(buffer, text, -1);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(text_view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_NONE);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), text_view);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_widget_set_hexpand(scroller, TRUE);

    gtk_widget_set_halign(buttons, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(buttons), cancel_button);
    gtk_box_append(GTK_BOX(buttons), paste_button);

    gtk_box_append(GTK_BOX(root), warning);
    gtk_box_append(GTK_BOX(root), summary);
    gtk_box_append(GTK_BOX(root), scroller);
    gtk_box_append(GTK_BOX(root), buttons);
    gtk_window_set_child(review->window, root);

    g_signal_connect_swapped(
        cancel_button,
        "clicked",
        G_CALLBACK(gtk_window_destroy),
        review->window
    );
    g_signal_connect(
        paste_button,
        "clicked",
        G_CALLBACK(on_paste_review_confirm),
        review
    );

    gtk_window_present(review->window);
}

static void
on_clipboard_text_ready(
    GObject *source_object,
    GAsyncResult *result,
    gpointer user_data
)
{
    PasteReadRequest *request = user_data;
    GdkClipboard *clipboard = GDK_CLIPBOARD(source_object);
    TerminalWindowState *state = g_object_get_data(
        G_OBJECT(request->parent),
        "goreecloud-terminal-window-state"
    );
    GcTerminalSession *requested_session = g_object_get_data(
        G_OBJECT(request->session_widget),
        "goreecloud-terminal-session"
    );
    GError *error = NULL;
    g_autofree char *text = gdk_clipboard_read_text_finish(
        clipboard,
        result,
        &error
    );

    if (error != NULL) {
        g_warning("Unable to read clipboard text: %s", error->message);
        g_clear_error(&error);
        paste_read_request_free(request);
        return;
    }

    if (state == NULL || requested_session == NULL ||
        gc_workspace_get_current_session(state->workspace) != requested_session ||
        text == NULL || *text == '\0') {
        paste_read_request_free(request);
        return;
    }

    if (gc_paste_text_requires_review(text)) {
        show_paste_review(request->parent, request->session_widget, text);
    } else {
        gc_terminal_session_paste_text(requested_session, text);
    }

    paste_read_request_free(request);
}

static void
request_safe_clipboard_paste(
    TerminalWindowState *state,
    GcTerminalSession *session
)
{
    PasteReadRequest *request;
    GdkClipboard *clipboard;

    if (session == NULL) {
        return;
    }

    request = g_new0(PasteReadRequest, 1);
    request->parent = g_object_ref(state->window);
    request->session_widget = g_object_ref(
        gc_terminal_session_get_widget(session)
    );

    clipboard = gtk_widget_get_clipboard(GTK_WIDGET(state->window));
    gdk_clipboard_read_text_async(
        clipboard,
        NULL,
        on_clipboard_text_ready,
        request
    );
}

static void
on_workspace_paste_requested(
    GcWorkspace *workspace,
    GcTerminalSession *session,
    gpointer user_data
)
{
    TerminalWindowState *state = user_data;
    (void) workspace;

    request_safe_clipboard_paste(state, session);
}

static void
on_workspace_open_requested(
    GcWorkspace *workspace,
    GcTerminalSession *session,
    GcLinkTargetKind kind,
    const char *target,
    gpointer user_data
)
{
    g_autofree char *working_directory =
        gc_terminal_session_dup_working_directory(session);
    g_autofree char *uri = NULL;
    GError *error = NULL;
    (void) workspace;
    (void) user_data;

    uri = gc_link_target_to_uri(
        kind,
        target,
        working_directory,
        &error
    );

    if (uri == NULL) {
        g_warning(
            "Refusing terminal link target '%s': %s",
            target,
            error != NULL ? error->message : "invalid target"
        );
        g_clear_error(&error);
        return;
    }

    if (!g_app_info_launch_default_for_uri(uri, NULL, &error)) {
        g_warning(
            "Unable to open terminal link target: %s",
            error != NULL ? error->message : "desktop handler failed"
        );
        g_clear_error(&error);
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
search_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) action;
    (void) parameter;

    gtk_widget_set_visible(state->search_bar, TRUE);
    gtk_widget_grab_focus(GTK_WIDGET(state->search_entry));
    apply_search(state, FALSE);
}

static void
search_close_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    GcTerminalSession *session = gc_workspace_get_current_session(state->workspace);
    (void) action;
    (void) parameter;

    if (session != NULL) {
        gc_terminal_session_clear_search(session);
        gc_terminal_session_focus(session);
    }

    gtk_widget_set_visible(state->search_bar, FALSE);
    gtk_label_set_text(state->search_status, "");
}

static void
search_next_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    GcTerminalSession *session = gc_workspace_get_current_session(state->workspace);
    (void) action;
    (void) parameter;

    if (session != NULL && apply_search(state, FALSE)) {
        gboolean found = gc_terminal_session_search_next(session);
        gtk_label_set_text(state->search_status, found ? "" : "No match");
    }
}

static void
search_previous_action(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    GcTerminalSession *session = gc_workspace_get_current_session(state->workspace);
    (void) action;
    (void) parameter;

    if (session != NULL && apply_search(state, FALSE)) {
        gboolean found = gc_terminal_session_search_previous(session);
        gtk_label_set_text(state->search_status, found ? "" : "No match");
    }
}

static void
on_search_changed(GtkSearchEntry *entry, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) entry;

    apply_search(state, TRUE);
}

static void
on_search_option_toggled(GtkCheckButton *button, gpointer user_data)
{
    TerminalWindowState *state = user_data;
    (void) button;

    apply_search(state, TRUE);
}

static gboolean
on_search_key_pressed(
    GtkEventControllerKey *controller,
    guint keyval,
    guint keycode,
    GdkModifierType modifiers,
    gpointer user_data
)
{
    (void) controller;
    (void) keycode;
    (void) modifiers;

    if (keyval != GDK_KEY_Escape) {
        return FALSE;
    }

    search_close_action(NULL, NULL, user_data);
    return TRUE;
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
    GtkWidget *split_horizontal_button = gtk_button_new_with_label("↔");
    GtkWidget *split_vertical_button = gtk_button_new_with_label("↕");
    GtkWidget *search_button = gtk_button_new_from_icon_name("edit-find-symbolic");
    GtkWidget *copy_button = gtk_button_new_from_icon_name("edit-copy-symbolic");
    GtkWidget *paste_button = gtk_button_new_from_icon_name("edit-paste-symbolic");
    GtkWidget *help_button = gtk_button_new_from_icon_name("help-about-symbolic");
    const char *new_accels[] = {"<Control><Shift>t", NULL};
    const char *close_accels[] = {"<Control><Shift>w", NULL};
    const char *split_horizontal_accels[] = {"<Control><Shift>e", NULL};
    const char *split_vertical_accels[] = {"<Control><Shift>o", NULL};
    const char *close_pane_accels[] = {"<Control><Shift>x", NULL};
    const char *next_pane_accels[] = {"<Alt>Right", NULL};
    const char *previous_pane_accels[] = {"<Alt>Left", NULL};
    const char *next_accels[] = {"<Control>Page_Down", NULL};
    const char *previous_accels[] = {"<Control>Page_Up", NULL};
    const char *search_accels[] = {"<Control><Shift>f", NULL};
    const char *search_next_accels[] = {"<Control>g", NULL};
    const char *search_previous_accels[] = {"<Control><Shift>g", NULL};
    const char *copy_accels[] = {"<Control><Shift>c", NULL};
    const char *paste_accels[] = {"<Control><Shift>v", NULL};
    const GActionEntry actions[] = {
        {"new-tab", new_tab_action, NULL, NULL, NULL},
        {"close-tab", close_tab_action, NULL, NULL, NULL},
        {"split-horizontal", split_horizontal_action, NULL, NULL, NULL},
        {"split-vertical", split_vertical_action, NULL, NULL, NULL},
        {"close-pane", close_pane_action, NULL, NULL, NULL},
        {"next-pane", next_pane_action, NULL, NULL, NULL},
        {"previous-pane", previous_pane_action, NULL, NULL, NULL},
        {"next-tab", next_tab_action, NULL, NULL, NULL},
        {"previous-tab", previous_tab_action, NULL, NULL, NULL},
        {"search", search_action, NULL, NULL, NULL},
        {"search-next", search_next_action, NULL, NULL, NULL},
        {"search-previous", search_previous_action, NULL, NULL, NULL},
        {"search-close", search_close_action, NULL, NULL, NULL},
        {"copy", copy_action, NULL, NULL, NULL},
        {"paste", paste_action, NULL, NULL, NULL},
        {"help", help_action, NULL, NULL, NULL},
    };

    state->subtitle_label = GTK_LABEL(gtk_label_new("Development · 1 tab · 1 pane"));

    gtk_widget_add_css_class(header, "gc-header");
    gtk_widget_add_css_class(title, "gc-title");
    gtk_widget_add_css_class(GTK_WIDGET(state->subtitle_label), "gc-subtitle");

    gtk_box_append(GTK_BOX(title_box), title);
    gtk_box_append(GTK_BOX(title_box), GTK_WIDGET(state->subtitle_label));
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), title_box);

    gtk_widget_set_tooltip_text(new_button, "New tab");
    gtk_widget_set_tooltip_text(close_button, "Close active tab");
    gtk_widget_set_tooltip_text(split_horizontal_button, "Split left/right");
    gtk_widget_set_tooltip_text(split_vertical_button, "Split top/bottom");
    gtk_widget_set_tooltip_text(search_button, "Search active pane");
    gtk_widget_set_tooltip_text(copy_button, "Copy selection");
    gtk_widget_set_tooltip_text(paste_button, "Paste clipboard");
    gtk_widget_set_tooltip_text(help_button, "Replay onboarding");

    gtk_actionable_set_action_name(GTK_ACTIONABLE(new_button), "win.new-tab");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(close_button), "win.close-tab");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(split_horizontal_button), "win.split-horizontal");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(split_vertical_button), "win.split-vertical");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(search_button), "win.search");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(copy_button), "win.copy");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(paste_button), "win.paste");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(help_button), "win.help");

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), new_button);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), close_button);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), split_horizontal_button);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), split_vertical_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), help_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), paste_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), copy_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), search_button);

    g_action_map_add_action_entries(
        G_ACTION_MAP(window),
        actions,
        G_N_ELEMENTS(actions),
        state
    );

    gtk_application_set_accels_for_action(application, "win.new-tab", new_accels);
    gtk_application_set_accels_for_action(application, "win.close-tab", close_accels);
    gtk_application_set_accels_for_action(application, "win.split-horizontal", split_horizontal_accels);
    gtk_application_set_accels_for_action(application, "win.split-vertical", split_vertical_accels);
    gtk_application_set_accels_for_action(application, "win.close-pane", close_pane_accels);
    gtk_application_set_accels_for_action(application, "win.next-pane", next_pane_accels);
    gtk_application_set_accels_for_action(application, "win.previous-pane", previous_pane_accels);
    gtk_application_set_accels_for_action(application, "win.next-tab", next_accels);
    gtk_application_set_accels_for_action(application, "win.previous-tab", previous_accels);
    gtk_application_set_accels_for_action(application, "win.search", search_accels);
    gtk_application_set_accels_for_action(application, "win.search-next", search_next_accels);
    gtk_application_set_accels_for_action(application, "win.search-previous", search_previous_accels);
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

static GtkWidget *
build_search_bar(TerminalWindowState *state)
{
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *entry = gtk_search_entry_new();
    GtkWidget *regex = gtk_check_button_new_with_label("Regex");
    GtkWidget *case_button = gtk_check_button_new_with_label("Case");
    GtkWidget *previous = gtk_button_new_from_icon_name("go-up-symbolic");
    GtkWidget *next = gtk_button_new_from_icon_name("go-down-symbolic");
    GtkWidget *close = gtk_button_new_from_icon_name("window-close-symbolic");
    GtkEventController *key_controller = gtk_event_controller_key_new();

    state->search_bar = bar;
    state->search_entry = GTK_SEARCH_ENTRY(entry);
    state->search_regex = GTK_CHECK_BUTTON(regex);
    state->search_case = GTK_CHECK_BUTTON(case_button);
    state->search_status = GTK_LABEL(gtk_label_new(""));

    gtk_widget_add_css_class(bar, "search-bar");
    gtk_widget_set_visible(bar, FALSE);
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_widget_set_tooltip_text(entry, "Search scrollback in the active pane");
    gtk_widget_set_tooltip_text(previous, "Previous match");
    gtk_widget_set_tooltip_text(next, "Next match");
    gtk_widget_set_tooltip_text(close, "Close search");

    gtk_actionable_set_action_name(GTK_ACTIONABLE(previous), "win.search-previous");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(next), "win.search-next");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(close), "win.search-close");

    gtk_event_controller_set_propagation_phase(
        key_controller,
        GTK_PHASE_CAPTURE
    );
    gtk_widget_add_controller(bar, key_controller);
    g_signal_connect(
        key_controller,
        "key-pressed",
        G_CALLBACK(on_search_key_pressed),
        state
    );

    gtk_box_append(GTK_BOX(bar), entry);
    gtk_box_append(GTK_BOX(bar), regex);
    gtk_box_append(GTK_BOX(bar), case_button);
    gtk_box_append(GTK_BOX(bar), GTK_WIDGET(state->search_status));
    gtk_box_append(GTK_BOX(bar), previous);
    gtk_box_append(GTK_BOX(bar), next);
    gtk_box_append(GTK_BOX(bar), close);

    g_signal_connect(
        entry,
        "search-changed",
        G_CALLBACK(on_search_changed),
        state
    );
    g_signal_connect(
        regex,
        "toggled",
        G_CALLBACK(on_search_option_toggled),
        state
    );
    g_signal_connect(
        case_button,
        "toggled",
        G_CALLBACK(on_search_option_toggled),
        state
    );

    return bar;
}

GtkWindow *
gc_terminal_window_new(GtkApplication *application)
{
    TerminalWindowState *state = g_new0(TerminalWindowState, 1);
    GtkWidget *window = gtk_application_window_new(application);
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    state->window = GTK_WINDOW(window);
    state->workspace = gc_workspace_new(
        on_workspace_changed,
        on_workspace_paste_requested,
        on_workspace_open_requested,
        state
    );

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
    gtk_box_append(GTK_BOX(root), build_search_bar(state));
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
