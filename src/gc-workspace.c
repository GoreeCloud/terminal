#include "gc-workspace.h"

#include <pango/pango.h>

typedef struct {
    GtkWidget *root;
    GPtrArray *sessions;
    GcTerminalSession *active;
    GtkLabel *tab_label;
} GcWorkspacePage;

struct _GcWorkspace {
    GtkNotebook *notebook;
    GcWorkspaceChangedFunc changed;
    GcWorkspacePasteRequestedFunc paste_requested;
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
on_session_paste_requested(GcTerminalSession *session, gpointer user_data)
{
    GcWorkspace *workspace = user_data;

    if (workspace->paste_requested != NULL) {
        workspace->paste_requested(workspace, session, workspace->user_data);
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
    gpointer user_data
)
{
    GcWorkspace *workspace = g_new0(GcWorkspace, 1);
    GtkWidget *notebook = gtk_notebook_new();

    workspace->notebook = GTK_NOTEBOOK(notebook);
    workspace->changed = changed;
    workspace->paste_requested = paste_requested;
    workspace->user_data = user_data;

    gtk_notebook_set_scrollable(workspace->notebook, TRUE);
    gtk_notebook_set_show_border(workspace->notebook, FALSE);
    gtk_notebook_set_tab_pos(workspace->notebook, GTK_POS_TOP);
    gtk_widget_set_hexpand(notebook, TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);

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

void
gc_workspace_add_tab(GcWorkspace *workspace, const char *working_directory)
{
    GcWorkspacePage *page = g_new0(GcWorkspacePage, 1);
    GcTerminalSession *session = gc_terminal_session_new(
        working_directory,
        on_session_changed,
        on_session_paste_requested,
        workspace
    );
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *label = gtk_label_new("Terminal");
    gint page_num;

    page->root = root;
    page->sessions = g_ptr_array_new();
    page->active = session;
    page->tab_label = GTK_LABEL(label);
    g_ptr_array_add(page->sessions, session);

    g_object_set_data_full(
        G_OBJECT(root),
        "goreecloud-workspace-page",
        page,
        page_free
    );

    gtk_widget_set_hexpand(root, TRUE);
    gtk_widget_set_vexpand(root, TRUE);
    gtk_box_append(GTK_BOX(root), gc_terminal_session_get_widget(session));

    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 28);

    page_num = gtk_notebook_append_page(workspace->notebook, root, label);
    gtk_notebook_set_tab_reorderable(workspace->notebook, root, TRUE);
    update_tab_label(page);
    gtk_notebook_set_current_page(workspace->notebook, page_num);
    gc_terminal_session_focus(session);
    notify_changed(workspace);
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

    if (page == NULL || page->active == NULL) {
        return FALSE;
    }

    active = page->active;
    active_widget = gc_terminal_session_get_widget(active);
    working_directory = gc_terminal_session_dup_working_directory(active);
    created = gc_terminal_session_new(
        working_directory,
        on_session_changed,
        on_session_paste_requested,
        workspace
    );
    created_widget = gc_terminal_session_get_widget(created);
    paned = gtk_paned_new(orientation);

    gtk_widget_set_hexpand(paned, TRUE);
    gtk_widget_set_vexpand(paned, TRUE);
    gtk_paned_set_wide_handle(GTK_PANED(paned), TRUE);
    gtk_paned_set_resize_start_child(GTK_PANED(paned), TRUE);
    gtk_paned_set_resize_end_child(GTK_PANED(paned), TRUE);
    gtk_paned_set_shrink_start_child(GTK_PANED(paned), FALSE);
    gtk_paned_set_shrink_end_child(GTK_PANED(paned), FALSE);

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
