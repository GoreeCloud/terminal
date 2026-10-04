#include "gc-workspace.h"

#include "gc-workspace-layout.h"

#include <pango/pango.h>

typedef struct {
    GtkWidget *root;
    GPtrArray *sessions;
    GcTerminalSession *active;
    GcProfile *profile;
    GtkLabel *tab_label;
} GcWorkspacePage;

struct _GcWorkspace {
    GtkNotebook *notebook;
    GcWorkspaceChangedFunc changed;
    GcWorkspacePasteRequestedFunc paste_requested;
    GcWorkspaceOpenRequestedFunc open_requested;
    gpointer user_data;
};

static void
notify_changed(GcWorkspace *workspace)
{
    if (workspace->changed != NULL) {
        workspace->changed(workspace, workspace->user_data);
    }
}

static void
page_free(gpointer data)
{
    GcWorkspacePage *page = data;

    g_ptr_array_free(page->sessions, TRUE);
    gc_profile_free(page->profile);
    g_free(page);
}

static GcWorkspacePage *
page_for_widget(GtkWidget *widget)
{
    if (widget == NULL) {
        return NULL;
    }

    return g_object_get_data(G_OBJECT(widget), "goreecloud-workspace-page");
}

static GcWorkspacePage *
current_page(GcWorkspace *workspace)
{
    gint page_num = gtk_notebook_get_current_page(workspace->notebook);

    if (page_num < 0) {
        return NULL;
    }

    return page_for_widget(
        gtk_notebook_get_nth_page(workspace->notebook, page_num)
    );
}

static GcWorkspacePage *
page_at_index(GcWorkspace *workspace, guint index)
{
    if (index >= (guint) gtk_notebook_get_n_pages(workspace->notebook)) {
        return NULL;
    }

    return page_for_widget(
        gtk_notebook_get_nth_page(workspace->notebook, (gint) index)
    );
}

static GcWorkspacePage *
find_page_for_session(GcWorkspace *workspace, GcTerminalSession *session)
{
    gint count = gtk_notebook_get_n_pages(workspace->notebook);

    for (gint page_num = 0; page_num < count; page_num++) {
        GcWorkspacePage *page = page_for_widget(
            gtk_notebook_get_nth_page(workspace->notebook, page_num)
        );

        if (page == NULL) {
            continue;
        }

        for (guint i = 0; i < page->sessions->len; i++) {
            if (g_ptr_array_index(page->sessions, i) == session) {
                return page;
            }
        }
    }

    return NULL;
}

static void
update_tab_label(GcWorkspacePage *page)
{
    g_autofree char *title = NULL;

    if (page == NULL || page->tab_label == NULL || page->active == NULL) {
        return;
    }

    title = gc_terminal_session_dup_display_title(page->active);
    gtk_label_set_text(page->tab_label, title);
    gtk_widget_set_tooltip_text(GTK_WIDGET(page->tab_label), title);
}

static void
configure_paned(GtkPaned *paned)
{
    gtk_widget_set_hexpand(GTK_WIDGET(paned), TRUE);
    gtk_widget_set_vexpand(GTK_WIDGET(paned), TRUE);
    gtk_paned_set_wide_handle(paned, TRUE);
    gtk_paned_set_resize_start_child(paned, TRUE);
    gtk_paned_set_resize_end_child(paned, TRUE);
    gtk_paned_set_shrink_start_child(paned, FALSE);
    gtk_paned_set_shrink_end_child(paned, FALSE);
}

static gint
session_index_for_widget(GcWorkspacePage *page, GtkWidget *widget)
{
    for (guint i = 0; i < page->sessions->len; i++) {
        GcTerminalSession *session = g_ptr_array_index(page->sessions, i);

        if (gc_terminal_session_get_widget(session) == widget) {
            return (gint) i;
        }
    }

    return -1;
}

static GcWorkspaceLayoutNode *
layout_node_for_widget(GcWorkspacePage *page, GtkWidget *widget)
{
    if (GTK_IS_PANED(widget)) {
        GtkPaned *paned = GTK_PANED(widget);
        GcWorkspaceLayoutNode *start = layout_node_for_widget(
            page,
            gtk_paned_get_start_child(paned)
        );
        GcWorkspaceLayoutNode *end = layout_node_for_widget(
            page,
            gtk_paned_get_end_child(paned)
        );
        GcWorkspaceLayoutOrientation orientation =
            gtk_orientable_get_orientation(GTK_ORIENTABLE(paned)) ==
                GTK_ORIENTATION_HORIZONTAL
                ? GC_WORKSPACE_LAYOUT_HORIZONTAL
                : GC_WORKSPACE_LAYOUT_VERTICAL;

        return gc_workspace_layout_split(orientation, start, end);
    }

    {
        gint index = session_index_for_widget(page, widget);

        return index >= 0
            ? gc_workspace_layout_leaf((guint) index)
            : NULL;
    }
}

static guint
layout_depth(GtkWidget *widget)
{
    if (!GTK_IS_PANED(widget)) {
        return 0;
    }

    return 1 + MAX(
        layout_depth(gtk_paned_get_start_child(GTK_PANED(widget))),
        layout_depth(gtk_paned_get_end_child(GTK_PANED(widget)))
    );
}

static GtkWidget *
widget_for_layout_node(
    GcWorkspacePage *page,
    const GcWorkspaceLayoutNode *node
)
{
    if (node->kind == GC_WORKSPACE_LAYOUT_LEAF) {
        GcTerminalSession *session;

        if (node->value.pane_index >= page->sessions->len) {
            return NULL;
        }

        session = g_ptr_array_index(
            page->sessions,
            node->value.pane_index
        );
        return gc_terminal_session_get_widget(session);
    }

    {
        GtkWidget *paned = gtk_paned_new(
            node->value.split.orientation ==
                GC_WORKSPACE_LAYOUT_HORIZONTAL
                ? GTK_ORIENTATION_HORIZONTAL
                : GTK_ORIENTATION_VERTICAL
        );
        GtkWidget *start = widget_for_layout_node(
            page,
            node->value.split.start
        );
        GtkWidget *end = widget_for_layout_node(
            page,
            node->value.split.end
        );

        if (start == NULL || end == NULL) {
            g_object_unref(paned);
            return NULL;
        }

        configure_paned(GTK_PANED(paned));
        gtk_paned_set_start_child(GTK_PANED(paned), start);
        gtk_paned_set_end_child(GTK_PANED(paned), end);
        return paned;
    }
}

static gboolean
replace_layout_child(
    GcWorkspacePage *page,
    GtkWidget *old_child,
    GtkWidget *new_child
)
{
    GtkWidget *parent = gtk_widget_get_parent(old_child);

    if (parent == page->root) {
        gtk_box_remove(GTK_BOX(page->root), old_child);
        gtk_box_append(GTK_BOX(page->root), new_child);
        return TRUE;
    }

    if (GTK_IS_PANED(parent)) {
        GtkPaned *paned = GTK_PANED(parent);

        if (gtk_paned_get_start_child(paned) == old_child) {
            gtk_paned_set_start_child(paned, NULL);
            gtk_paned_set_start_child(paned, new_child);
            return TRUE;
        }

        if (gtk_paned_get_end_child(paned) == old_child) {
            gtk_paned_set_end_child(paned, NULL);
            gtk_paned_set_end_child(paned, new_child);
            return TRUE;
        }
    }

    return FALSE;
}

static void
on_session_changed(GcTerminalSession *session, gpointer user_data)
{
    GcWorkspace *workspace = user_data;
    GcWorkspacePage *page = find_page_for_session(workspace, session);

    if (page == NULL) {
        return;
    }

    if (gc_terminal_session_has_focus(session)) {
        page->active = session;
    }

    if (page->active == session) {
        update_tab_label(page);

        if (page == current_page(workspace)) {
            notify_changed(workspace);
        }
    }
}

static void
on_session_paste_requested(
    GcTerminalSession *session,
    GcTerminalPasteSource source,
    gpointer user_data
)
{
    GcWorkspace *workspace = user_data;

    if (workspace->paste_requested != NULL) {
        workspace->paste_requested(
            workspace,
            session,
            source,
            workspace->user_data
        );
    }
}

static void
on_session_open_requested(
    GcTerminalSession *session,
    GcLinkTargetKind kind,
    const char *target,
    gpointer user_data
)
{
    GcWorkspace *workspace = user_data;

    if (workspace->open_requested != NULL) {
        workspace->open_requested(
            workspace,
            session,
            kind,
            target,
            workspace->user_data
        );
    }
}

static void
on_tab_close_clicked(GtkButton *button, gpointer user_data)
{
    GcWorkspace *workspace = user_data;
    GtkWidget *page_root = g_object_get_data(
        G_OBJECT(button),
        "goreecloud-tab-page-root"
    );
    gint page_num;

    if (page_root == NULL) {
        return;
    }

    page_num = gtk_notebook_page_num(workspace->notebook, page_root);
    if (page_num < 0) {
        return;
    }

    gtk_notebook_set_current_page(workspace->notebook, page_num);

    if (!gtk_widget_activate_action(GTK_WIDGET(button), "win.close-tab", NULL) &&
        gtk_notebook_get_n_pages(workspace->notebook) > 1) {
        gtk_notebook_remove_page(workspace->notebook, page_num);
        notify_changed(workspace);
    }
}

static void
on_switch_page(
    GtkNotebook *notebook,
    GtkWidget *page_widget,
    guint page_num,
    gpointer user_data
)
{
    GcWorkspace *workspace = user_data;
    GcWorkspacePage *page = page_for_widget(page_widget);
    (void) notebook;
    (void) page_num;

    if (page != NULL && page->active != NULL) {
        gc_terminal_session_focus(page->active);
    }

    notify_changed(workspace);
}

GcWorkspace *
gc_workspace_new(
    GcWorkspaceChangedFunc changed,
    GcWorkspacePasteRequestedFunc paste_requested,
    GcWorkspaceOpenRequestedFunc open_requested,
    gpointer user_data
)
{
    GcWorkspace *workspace = g_new0(GcWorkspace, 1);
    GtkWidget *notebook = gtk_notebook_new();
    GtkWidget *add_button = gtk_button_new_from_icon_name("list-add-symbolic");

    workspace->notebook = GTK_NOTEBOOK(notebook);
    workspace->changed = changed;
    workspace->paste_requested = paste_requested;
    workspace->open_requested = open_requested;
    workspace->user_data = user_data;

    gtk_notebook_set_scrollable(workspace->notebook, TRUE);
    gtk_notebook_set_show_border(workspace->notebook, FALSE);
    gtk_notebook_set_tab_pos(workspace->notebook, GTK_POS_TOP);
    gtk_widget_set_hexpand(notebook, TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);
    gtk_widget_add_css_class(notebook, "gc-workspace");

    gtk_widget_add_css_class(add_button, "gc-tab-add");
    gtk_widget_set_tooltip_text(add_button, "New tab");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(add_button), "win.new-tab");
    gtk_notebook_set_action_widget(
        workspace->notebook,
        add_button,
        GTK_PACK_END
    );

    g_signal_connect(
        workspace->notebook,
        "switch-page",
        G_CALLBACK(on_switch_page),
        workspace
    );

    return workspace;
}

void
gc_workspace_free(GcWorkspace *workspace)
{
    g_free(workspace);
}

GtkWidget *
gc_workspace_get_widget(GcWorkspace *workspace)
{
    return GTK_WIDGET(workspace->notebook);
}

GcTerminalSession *
gc_workspace_get_current_session(GcWorkspace *workspace)
{
    GcWorkspacePage *page = current_page(workspace);

    return page != NULL ? page->active : NULL;
}

guint
gc_workspace_get_count(GcWorkspace *workspace)
{
    return (guint) gtk_notebook_get_n_pages(workspace->notebook);
}

guint
gc_workspace_get_current_pane_count(GcWorkspace *workspace)
{
    GcWorkspacePage *page = current_page(workspace);

    return page != NULL ? page->sessions->len : 0;
}

guint
gc_workspace_get_current_index(GcWorkspace *workspace)
{
    gint index = gtk_notebook_get_current_page(workspace->notebook);

    return index >= 0 ? (guint) index : 0;
}

const char *
gc_workspace_get_current_profile_id(GcWorkspace *workspace)
{
    GcWorkspacePage *page = current_page(workspace);

    if (page == NULL || page->active == NULL) {
        return "default";
    }

    return gc_terminal_session_get_profile_id(page->active);
}

const char *
gc_workspace_get_tab_profile_id(GcWorkspace *workspace, guint index)
{
    GcWorkspacePage *page = page_at_index(workspace, index);

    if (page == NULL || page->active == NULL) {
        return "default";
    }

    return gc_terminal_session_get_profile_id(page->active);
}

char *
gc_workspace_dup_tab_working_directory(GcWorkspace *workspace, guint index)
{
    GcWorkspacePage *page = page_at_index(workspace, index);

    if (page == NULL || page->active == NULL) {
        return g_strdup(g_get_home_dir());
    }

    return gc_terminal_session_dup_working_directory(page->active);
}

guint
gc_workspace_get_tab_pane_count(
    GcWorkspace *workspace,
    guint index
)
{
    GcWorkspacePage *page = page_at_index(workspace, index);

    return page != NULL ? page->sessions->len : 0;
}

guint
gc_workspace_get_tab_active_pane_index(
    GcWorkspace *workspace,
    guint index
)
{
    GcWorkspacePage *page = page_at_index(workspace, index);

    if (page == NULL || page->active == NULL) {
        return 0;
    }

    for (guint i = 0; i < page->sessions->len; i++) {
        if (g_ptr_array_index(page->sessions, i) == page->active) {
            return i;
        }
    }

    return 0;
}

char *
gc_workspace_dup_tab_pane_working_directory(
    GcWorkspace *workspace,
    guint index,
    guint pane_index
)
{
    GcWorkspacePage *page = page_at_index(workspace, index);

    if (page == NULL || pane_index >= page->sessions->len) {
        return g_strdup(g_get_home_dir());
    }

    return gc_terminal_session_dup_working_directory(
        g_ptr_array_index(page->sessions, pane_index)
    );
}

char *
gc_workspace_dup_tab_layout(
    GcWorkspace *workspace,
    guint index
)
{
    GcWorkspacePage *page = page_at_index(workspace, index);
    GtkWidget *child;
    GcWorkspaceLayoutNode *layout;
    char *serialized;

    if (page == NULL) {
        return g_strdup("0");
    }

    child = gtk_widget_get_first_child(page->root);
    if (child == NULL) {
        return g_strdup("0");
    }

    layout = layout_node_for_widget(page, child);
    if (layout == NULL) {
        return g_strdup("0");
    }

    serialized = gc_workspace_layout_serialize(layout);
    gc_workspace_layout_free(layout);
    return serialized;
}

gboolean
gc_workspace_add_tab_with_profile_layout(
    GcWorkspace *workspace,
    const GcProfile *profile,
    const char *layout,
    GPtrArray *pane_working_directories,
    guint active_pane
)
{
    GError *error = NULL;
    GcWorkspaceLayoutNode *parsed;
    GcWorkspacePage *page;
    GtkWidget *root;
    GtkWidget *layout_widget;
    GtkWidget *tab;
    GtkWidget *icon;
    GtkWidget *label;
    GtkWidget *close;
    gint page_num;

    if (pane_working_directories == NULL ||
        pane_working_directories->len == 0 ||
        pane_working_directories->len > GC_WORKSPACE_LAYOUT_MAX_PANES ||
        active_pane >= pane_working_directories->len) {
        return FALSE;
    }

    parsed = gc_workspace_layout_parse(
        layout,
        pane_working_directories->len,
        &error
    );
    if (parsed == NULL) {
        g_warning(
            "Refusing invalid restored workspace layout: %s",
            error != NULL ? error->message : "invalid layout"
        );
        g_clear_error(&error);
        return FALSE;
    }

    page = g_new0(GcWorkspacePage, 1);
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    tab = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    icon = gtk_image_new_from_icon_name("utilities-terminal-symbolic");
    label = gtk_label_new("Terminal");
    close = gtk_button_new_from_icon_name("window-close-symbolic");

    page->root = root;
    page->sessions = g_ptr_array_new();
    page->profile = gc_profile_copy(profile);
    page->tab_label = GTK_LABEL(label);

    for (guint i = 0; i < pane_working_directories->len; i++) {
        const char *cwd = g_ptr_array_index(
            pane_working_directories,
            i
        );
        GcTerminalSession *session = gc_terminal_session_new_with_profile(
            profile,
            cwd,
            on_session_changed,
            on_session_paste_requested,
            on_session_open_requested,
            workspace
        );

        g_ptr_array_add(page->sessions, session);
    }

    page->active = g_ptr_array_index(page->sessions, active_pane);
    layout_widget = widget_for_layout_node(page, parsed);
    gc_workspace_layout_free(parsed);

    if (layout_widget == NULL) {
        g_ptr_array_free(page->sessions, TRUE);
        gc_profile_free(page->profile);
        g_free(page);
        return FALSE;
    }

    g_object_set_data_full(
        G_OBJECT(root),
        "goreecloud-workspace-page",
        page,
        page_free
    );

    gtk_widget_set_hexpand(root, TRUE);
    gtk_widget_set_vexpand(root, TRUE);
    gtk_box_append(GTK_BOX(root), layout_widget);

    gtk_widget_add_css_class(tab, "gc-tab-label");
    gtk_widget_add_css_class(icon, "gc-tab-icon");
    gtk_widget_add_css_class(close, "gc-tab-close");
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 28);
    gtk_widget_set_tooltip_text(close, "Close tab");
    g_object_set_data(
        G_OBJECT(close),
        "goreecloud-tab-page-root",
        root
    );
    g_signal_connect(
        close,
        "clicked",
        G_CALLBACK(on_tab_close_clicked),
        workspace
    );

    gtk_box_append(GTK_BOX(tab), icon);
    gtk_box_append(GTK_BOX(tab), label);
    gtk_box_append(GTK_BOX(tab), close);

    page_num = gtk_notebook_append_page(workspace->notebook, root, tab);
    gtk_notebook_set_tab_reorderable(workspace->notebook, root, TRUE);
    update_tab_label(page);
    gtk_notebook_set_current_page(workspace->notebook, page_num);
    gc_terminal_session_focus(page->active);
    notify_changed(workspace);
    return TRUE;
}

void
gc_workspace_add_tab_with_profile(
    GcWorkspace *workspace,
    const GcProfile *profile,
    const char *working_directory
)
{
    GPtrArray *panes = g_ptr_array_new();

    g_ptr_array_add(panes, (gpointer) working_directory);
    gc_workspace_add_tab_with_profile_layout(
        workspace,
        profile,
        "0",
        panes,
        0
    );
    g_ptr_array_unref(panes);
}

void
gc_workspace_add_tab(GcWorkspace *workspace, const char *working_directory)
{
    gc_workspace_add_tab_with_profile(
        workspace,
        NULL,
        working_directory
    );
}

gboolean
gc_workspace_close_current(GcWorkspace *workspace)
{
    gint page_num = gtk_notebook_get_current_page(workspace->notebook);

    if (page_num < 0 || gtk_notebook_get_n_pages(workspace->notebook) <= 1) {
        return FALSE;
    }

    gtk_notebook_remove_page(workspace->notebook, page_num);
    notify_changed(workspace);
    return TRUE;
}

void
gc_workspace_select_relative(GcWorkspace *workspace, gint delta)
{
    gint count = gtk_notebook_get_n_pages(workspace->notebook);
    gint current = gtk_notebook_get_current_page(workspace->notebook);
    gint next;

    if (count <= 1 || current < 0 || delta == 0) {
        return;
    }

    next = (current + delta) % count;
    if (next < 0) {
        next += count;
    }

    gtk_notebook_set_current_page(workspace->notebook, next);
}

void
gc_workspace_select_index(GcWorkspace *workspace, guint index)
{
    gint count = gtk_notebook_get_n_pages(workspace->notebook);

    if (count <= 0 || index >= (guint) count) {
        return;
    }

    gtk_notebook_set_current_page(workspace->notebook, (gint) index);
}

char *
gc_workspace_dup_current_working_directory(GcWorkspace *workspace)
{
    GcTerminalSession *session = gc_workspace_get_current_session(workspace);

    if (session == NULL) {
        return g_strdup(g_get_home_dir());
    }

    return gc_terminal_session_dup_working_directory(session);
}

gboolean
gc_workspace_split_current(
    GcWorkspace *workspace,
    GtkOrientation orientation
)
{
    GcWorkspacePage *page = current_page(workspace);
    GcTerminalSession *active;
    GcTerminalSession *created;
    GtkWidget *active_widget;
    GtkWidget *created_widget;
    GtkWidget *paned;
    g_autofree char *working_directory = NULL;

    if (page == NULL || page->active == NULL ||
        page->sessions->len >= GC_WORKSPACE_LAYOUT_MAX_PANES) {
        return FALSE;
    }

    {
        GtkWidget *layout_root = gtk_widget_get_first_child(page->root);

        if (layout_root != NULL &&
            layout_depth(layout_root) >= GC_WORKSPACE_LAYOUT_MAX_DEPTH) {
            return FALSE;
        }
    }

    active = page->active;
    active_widget = gc_terminal_session_get_widget(active);
    working_directory = gc_terminal_session_dup_working_directory(active);
    created = gc_terminal_session_new_with_profile(
        page->profile,
        working_directory,
        on_session_changed,
        on_session_paste_requested,
        on_session_open_requested,
        workspace
    );
    created_widget = gc_terminal_session_get_widget(created);
    paned = gtk_paned_new(orientation);

    configure_paned(GTK_PANED(paned));

    g_object_ref(active_widget);
    if (!replace_layout_child(page, active_widget, paned)) {
        g_object_unref(active_widget);
        g_object_unref(created_widget);
        g_object_unref(paned);
        return FALSE;
    }

    gtk_paned_set_start_child(GTK_PANED(paned), active_widget);
    gtk_paned_set_end_child(GTK_PANED(paned), created_widget);
    g_object_unref(active_widget);

    g_ptr_array_add(page->sessions, created);
    page->active = created;
    update_tab_label(page);
    gc_terminal_session_focus(created);
    notify_changed(workspace);
    return TRUE;
}

gboolean
gc_workspace_close_current_pane(GcWorkspace *workspace)
{
    GcWorkspacePage *page = current_page(workspace);
    GcTerminalSession *active;
    GtkWidget *active_widget;
    GtkWidget *parent;
    GtkWidget *sibling;
    guint active_index = 0;
    gboolean found = FALSE;

    if (page == NULL || page->active == NULL || page->sessions->len <= 1) {
        return FALSE;
    }

    active = page->active;
    active_widget = gc_terminal_session_get_widget(active);
    parent = gtk_widget_get_parent(active_widget);

    if (!GTK_IS_PANED(parent)) {
        return FALSE;
    }

    sibling =
        gtk_paned_get_start_child(GTK_PANED(parent)) == active_widget
            ? gtk_paned_get_end_child(GTK_PANED(parent))
            : gtk_paned_get_start_child(GTK_PANED(parent));

    if (sibling == NULL) {
        return FALSE;
    }

    for (guint i = 0; i < page->sessions->len; i++) {
        if (g_ptr_array_index(page->sessions, i) == active) {
            active_index = i;
            found = TRUE;
            break;
        }
    }

    if (!found) {
        return FALSE;
    }

    g_ptr_array_remove_index(page->sessions, active_index);

    g_object_ref(sibling);
    if (gtk_paned_get_start_child(GTK_PANED(parent)) == sibling) {
        gtk_paned_set_start_child(GTK_PANED(parent), NULL);
    } else {
        gtk_paned_set_end_child(GTK_PANED(parent), NULL);
    }

    if (!replace_layout_child(page, parent, sibling)) {
        g_object_unref(sibling);
        return FALSE;
    }
    g_object_unref(sibling);

    if (active_index >= page->sessions->len) {
        active_index = page->sessions->len - 1;
    }

    page->active = g_ptr_array_index(page->sessions, active_index);
    update_tab_label(page);
    gc_terminal_session_focus(page->active);
    notify_changed(workspace);
    return TRUE;
}

void
gc_workspace_focus_relative_pane(GcWorkspace *workspace, gint delta)
{
    GcWorkspacePage *page = current_page(workspace);
    guint active_index = 0;
    gboolean found = FALSE;
    gint next;

    if (page == NULL || page->active == NULL ||
        page->sessions->len <= 1 || delta == 0) {
        return;
    }

    for (guint i = 0; i < page->sessions->len; i++) {
        if (g_ptr_array_index(page->sessions, i) == page->active) {
            active_index = i;
            found = TRUE;
            break;
        }
    }

    if (!found) {
        return;
    }

    next = ((gint) active_index + delta) % (gint) page->sessions->len;
    if (next < 0) {
        next += (gint) page->sessions->len;
    }

    page->active = g_ptr_array_index(page->sessions, (guint) next);
    update_tab_label(page);
    gc_terminal_session_focus(page->active);
    notify_changed(workspace);
}
