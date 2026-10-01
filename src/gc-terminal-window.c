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
    ".gc-window { background: #030914; color: #dceaff; }"
    ".gc-header { background: #071526; border-bottom: 1px solid #0d4777; padding: 4px 6px; }"
    ".brand-box { margin: 0 8px 0 2px; }"
    ".brand-icon { color: #2ea8ff; margin-right: 2px; }"
    ".gc-title { color: #f4f8ff; font-weight: 800; letter-spacing: 0.01em; }"
    ".gc-product { color: #8ca9d4; font-weight: 500; }"
    ".header-center { margin: 0 8px; }"
    ".connection-chip { color: #b9d2f3; padding: 5px 10px; border-radius: 999px; background: #0b1c30; border: 1px solid #174b78; }"
    ".connection-dot { color: #32e99b; font-weight: 900; }"
    ".gc-subtitle { color: #7f9dc5; font-size: 0.88em; }"
    ".header-action { color: #b9d2f3; background: transparent; border: 1px solid transparent; border-radius: 8px; padding: 5px; }"
    ".header-action:hover { background: #0c223a; border-color: #174b78; }"
    ".search-bar { padding: 8px 12px; background: #071525; border-bottom: 1px solid #0d3c63; }"
    ".search-bar entry { background: #0a1b2f; color: #e5efff; border-color: #1a5589; border-radius: 8px; }"
    ".search-error { color: #ff9eaf; }"
    ".sidebar { min-width: 210px; padding: 14px 10px; background: #06111f; border-right: 1px solid #0d3c63; }"
    ".sidebar-heading { color: #6f8eb7; font-size: 0.78em; font-weight: 800; letter-spacing: 0.06em; margin: 2px 6px 6px 6px; }"
    ".sidebar-card { padding: 9px 10px; border-radius: 8px; background: #0a2037; border: 1px solid #174b78; }"
    ".sidebar-dot { color: #32e99b; font-weight: 900; }"
    ".sidebar-title { color: #edf5ff; font-weight: 700; }"
    ".sidebar-detail { color: #7f9dc5; font-size: 0.86em; }"
    ".sidebar-action { color: #abc2df; background: transparent; border: 1px solid transparent; border-radius: 7px; padding: 7px 9px; }"
    ".sidebar-action:hover { color: #f2f7ff; background: #0a2037; border-color: #174b78; }"
    ".sidebar-footer { color: #6f8eb7; font-size: 0.84em; margin: 8px 6px 2px 6px; }"
    ".gc-workspace { background: #050d18; }"
    ".gc-workspace > header { background: #06111f; border-bottom: 1px solid #0d3c63; padding: 0 8px; }"
    ".gc-workspace > header tabs tab { color: #8faaca; background: transparent; border-right: 1px solid #102f4e; padding: 8px 12px; min-width: 120px; }"
    ".gc-workspace > header tabs tab:checked { color: #f2f7ff; background: #0a2340; box-shadow: inset 0 -2px #169cff; }"
    ".gc-workspace > header tabs tab:hover { background: #0a1c31; }"
    ".gc-tab-icon { color: #2ea8ff; }"
    ".gc-tab-close { min-width: 22px; min-height: 22px; padding: 0; color: #7999bd; background: transparent; border-color: transparent; }"
    ".gc-tab-close:hover { color: #f2f7ff; background: #123656; }"
    ".gc-tab-add { min-width: 30px; min-height: 28px; padding: 2px 7px; margin: 4px 6px; color: #89a8cf; background: transparent; border: 1px solid transparent; border-radius: 7px; }"
    ".gc-tab-add:hover { color: #f2f7ff; background: #0c223a; border-color: #174b78; }"
    ".terminal-frame { padding: 10px 12px; background: #050d18; }"
    ".status-bar { padding: 7px 12px; background: #071526; border-top: 1px solid #0d4777; }"
    ".status-chip { padding: 3px 8px; border-radius: 999px; background: #0a2036; border: 1px solid #184d79; color: #b9d2f3; }"
    ".status-chip-elevated { background: #321b33; border-color: #8b426f; color: #ffd4ea; }"
    ".status-identity { color: #9eb7d6; }"
    ".status-path { color: #81c8ff; }"
    ".status-development { color: #f2c86f; padding: 3px 8px; border-radius: 999px; background: #302713; border: 1px solid #695622; }"
    ".onboarding-card { background: #071526; color: #e6f0ff; border: 1px solid #174b78; }"
    ".onboarding-kicker { color: #37aefc; font-size: 0.82em; font-weight: 800; letter-spacing: 0.06em; }"
    ".onboarding-title { color: #f4f8ff; font-size: 1.55em; font-weight: 800; }"
    ".onboarding-body { color: #b9cce6; }"
    ".paste-warning { color: #f6c75f; font-weight: 800; }";

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
        "%u tab%s · %u pane%s",
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
request_safe_paste(
    TerminalWindowState *state,
    GcTerminalSession *session,
    GcTerminalPasteSource source
)
{
    PasteReadRequest *request;
    GdkClipboard *clipboard;

    if (session == NULL) {
        return;
    }

    switch (source) {
    case GC_TERMINAL_PASTE_CLIPBOARD:
        clipboard = gtk_widget_get_clipboard(GTK_WIDGET(state->window));
        break;
    case GC_TERMINAL_PASTE_PRIMARY:
        clipboard = gtk_widget_get_primary_clipboard(GTK_WIDGET(state->window));
        break;
    default:
        g_warning("Refusing unknown terminal paste source: %d", source);
        return;
    }

    request = g_new0(PasteReadRequest, 1);
    request->parent = g_object_ref(state->window);
    request->session_widget = g_object_ref(
        gc_terminal_session_get_widget(session)
    );

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
    GcTerminalPasteSource source,
    gpointer user_data
)
{
    TerminalWindowState *state = user_data;
    (void) workspace;

    request_safe_paste(state, session, source);
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
    GtkWidget *brand_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 7);
    GtkWidget *brand_icon = gtk_image_new_from_icon_name("utilities-terminal-symbolic");
    GtkWidget *brand_name = gtk_label_new("GoreeCloud");
    GtkWidget *product_name = gtk_label_new("Terminal");
    GtkWidget *center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *connection_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *connection_dot = gtk_label_new("●");
    GtkWidget *connection_text = gtk_label_new("Local shell");
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
        {.name = "new-tab", .activate = new_tab_action},
        {.name = "close-tab", .activate = close_tab_action},
        {.name = "split-horizontal", .activate = split_horizontal_action},
        {.name = "split-vertical", .activate = split_vertical_action},
        {.name = "close-pane", .activate = close_pane_action},
        {.name = "next-pane", .activate = next_pane_action},
        {.name = "previous-pane", .activate = previous_pane_action},
        {.name = "next-tab", .activate = next_tab_action},
        {.name = "previous-tab", .activate = previous_tab_action},
        {.name = "search", .activate = search_action},
        {.name = "search-next", .activate = search_next_action},
        {.name = "search-previous", .activate = search_previous_action},
        {.name = "search-close", .activate = search_close_action},
        {.name = "copy", .activate = copy_action},
        {.name = "paste", .activate = paste_action},
        {.name = "help", .activate = help_action},
    };

    state->subtitle_label = GTK_LABEL(gtk_label_new("1 tab · 1 pane"));

    gtk_widget_add_css_class(header, "gc-header");
    gtk_widget_add_css_class(brand_box, "brand-box");
    gtk_widget_add_css_class(brand_icon, "brand-icon");
    gtk_widget_add_css_class(brand_name, "gc-title");
    gtk_widget_add_css_class(product_name, "gc-product");
    gtk_widget_add_css_class(center_box, "header-center");
    gtk_widget_add_css_class(connection_box, "connection-chip");
    gtk_widget_add_css_class(connection_dot, "connection-dot");
    gtk_widget_add_css_class(GTK_WIDGET(state->subtitle_label), "gc-subtitle");

    gtk_box_append(GTK_BOX(brand_box), brand_icon);
    gtk_box_append(GTK_BOX(brand_box), brand_name);
    gtk_box_append(GTK_BOX(brand_box), product_name);

    gtk_box_append(GTK_BOX(connection_box), connection_dot);
    gtk_box_append(GTK_BOX(connection_box), connection_text);
    gtk_box_append(GTK_BOX(center_box), connection_box);
    gtk_box_append(GTK_BOX(center_box), GTK_WIDGET(state->subtitle_label));

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), brand_box);
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), center_box);

    gtk_widget_set_tooltip_text(split_horizontal_button, "Split left/right");
    gtk_widget_set_tooltip_text(split_vertical_button, "Split top/bottom");
    gtk_widget_set_tooltip_text(search_button, "Search active pane");
    gtk_widget_set_tooltip_text(copy_button, "Copy selection");
    gtk_widget_set_tooltip_text(paste_button, "Paste clipboard");
    gtk_widget_set_tooltip_text(help_button, "Replay onboarding");

    GtkWidget *header_actions[] = {
        split_horizontal_button,
        split_vertical_button,
        search_button,
        copy_button,
        paste_button,
        help_button,
    };
    for (guint i = 0; i < G_N_ELEMENTS(header_actions); i++) {
        gtk_widget_add_css_class(header_actions[i], "header-action");
    }

    gtk_actionable_set_action_name(GTK_ACTIONABLE(split_horizontal_button), "win.split-horizontal");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(split_vertical_button), "win.split-vertical");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(search_button), "win.search");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(copy_button), "win.copy");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(paste_button), "win.paste");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(help_button), "win.help");

    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), help_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), paste_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), copy_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), search_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), split_vertical_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), split_horizontal_button);

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
    GtkWidget *bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *scope = gtk_label_new(gc_context_privilege_label());
    g_autofree char *identity = gc_context_identity();
    GtkWidget *identity_label = gtk_label_new(identity);
    GtkWidget *development = gtk_label_new("Development");

    state->cwd_label = GTK_LABEL(gtk_label_new(g_get_home_dir()));
    state->session_label = GTK_LABEL(gtk_label_new("Starting shell"));

    gtk_widget_add_css_class(bar, "status-bar");
    gtk_widget_add_css_class(scope, "status-chip");
    gtk_widget_add_css_class(identity_label, "status-identity");
    gtk_widget_add_css_class(GTK_WIDGET(state->session_label), "status-chip");
    gtk_widget_add_css_class(GTK_WIDGET(state->cwd_label), "status-path");
    gtk_widget_add_css_class(development, "status-development");

    if (g_strcmp0(gc_context_privilege_label(), "Elevated local") == 0) {
        gtk_widget_add_css_class(scope, "status-chip-elevated");
    }

    gtk_label_set_ellipsize(state->cwd_label, PANGO_ELLIPSIZE_MIDDLE);
    gtk_label_set_xalign(state->cwd_label, 0.0f);
    gtk_widget_set_hexpand(GTK_WIDGET(state->cwd_label), TRUE);

    gtk_box_append(GTK_BOX(bar), scope);
    gtk_box_append(GTK_BOX(bar), identity_label);
    gtk_box_append(GTK_BOX(bar), GTK_WIDGET(state->cwd_label));
    gtk_box_append(GTK_BOX(bar), GTK_WIDGET(state->session_label));
    gtk_box_append(GTK_BOX(bar), development);

    return bar;
}

static GtkWidget *
build_sidebar(void)
{
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *workspace_heading = gtk_label_new("WORKSPACE");
    GtkWidget *local_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    GtkWidget *local_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 7);
    GtkWidget *local_dot = gtk_label_new("●");
    GtkWidget *local_title = gtk_label_new("Local Shell");
    g_autofree char *identity = gc_context_identity();
    GtkWidget *local_detail = gtk_label_new(identity);
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *actions_heading = gtk_label_new("QUICK ACTIONS");
    GtkWidget *new_tab = gtk_button_new_with_label("＋  New tab");
    GtkWidget *split_left_right = gtk_button_new_with_label("↔  Split left / right");
    GtkWidget *split_top_bottom = gtk_button_new_with_label("↕  Split top / bottom");
    GtkWidget *search = gtk_button_new_with_label("⌕  Search scrollback");
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *footer = gtk_label_new("GoreeCloud · Development");
    GtkWidget *actions[] = {
        new_tab,
        split_left_right,
        split_top_bottom,
        search,
    };

    gtk_widget_add_css_class(sidebar, "sidebar");
    gtk_widget_add_css_class(workspace_heading, "sidebar-heading");
    gtk_widget_add_css_class(local_card, "sidebar-card");
    gtk_widget_add_css_class(local_dot, "sidebar-dot");
    gtk_widget_add_css_class(local_title, "sidebar-title");
    gtk_widget_add_css_class(local_detail, "sidebar-detail");
    gtk_widget_add_css_class(actions_heading, "sidebar-heading");
    gtk_widget_add_css_class(footer, "sidebar-footer");

    gtk_label_set_xalign(GTK_LABEL(workspace_heading), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(local_title), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(local_detail), 0.0f);
    gtk_label_set_ellipsize(GTK_LABEL(local_detail), PANGO_ELLIPSIZE_END);
    gtk_label_set_xalign(GTK_LABEL(actions_heading), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(footer), 0.0f);

    gtk_box_append(GTK_BOX(local_row), local_dot);
    gtk_box_append(GTK_BOX(local_row), local_title);
    gtk_box_append(GTK_BOX(local_card), local_row);
    gtk_box_append(GTK_BOX(local_card), local_detail);

    for (guint i = 0; i < G_N_ELEMENTS(actions); i++) {
        gtk_widget_add_css_class(actions[i], "sidebar-action");
        gtk_widget_set_hexpand(actions[i], TRUE);
        gtk_widget_set_halign(actions[i], GTK_ALIGN_FILL);
    }

    gtk_actionable_set_action_name(GTK_ACTIONABLE(new_tab), "win.new-tab");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(split_left_right), "win.split-horizontal");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(split_top_bottom), "win.split-vertical");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(search), "win.search");

    gtk_widget_set_vexpand(spacer, TRUE);

    gtk_box_append(GTK_BOX(sidebar), workspace_heading);
    gtk_box_append(GTK_BOX(sidebar), local_card);
    gtk_box_append(GTK_BOX(sidebar), separator);
    gtk_box_append(GTK_BOX(sidebar), actions_heading);
    gtk_box_append(GTK_BOX(sidebar), new_tab);
    gtk_box_append(GTK_BOX(sidebar), split_left_right);
    gtk_box_append(GTK_BOX(sidebar), split_top_bottom);
    gtk_box_append(GTK_BOX(sidebar), search);
    gtk_box_append(GTK_BOX(sidebar), spacer);
    gtk_box_append(GTK_BOX(sidebar), footer);

    return sidebar;
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
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *main_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

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
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 800);
    gtk_window_set_title(GTK_WINDOW(window), "GoreeCloud Terminal");
    gtk_window_set_titlebar(
        GTK_WINDOW(window),
        build_header(application, GTK_WINDOW(window), state)
    );

    gtk_widget_set_hexpand(main_area, TRUE);
    gtk_widget_set_vexpand(main_area, TRUE);
    gtk_box_append(GTK_BOX(main_area), build_search_bar(state));
    gtk_box_append(GTK_BOX(main_area), gc_workspace_get_widget(state->workspace));

    gtk_box_append(GTK_BOX(content), build_sidebar());
    gtk_box_append(GTK_BOX(content), main_area);
    gtk_widget_set_hexpand(content, TRUE);
    gtk_widget_set_vexpand(content, TRUE);

    gtk_box_append(GTK_BOX(root), content);
    gtk_box_append(GTK_BOX(root), build_context_bar(state));
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
