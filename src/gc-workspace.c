#include "gc-workspace.h"

#include <pango/pango.h>

struct _GcWorkspace {
    GtkNotebook *notebook;
    GcWorkspaceChangedFunc changed;
    gpointer user_data;
};

static void
notify_changed(GcWorkspace *workspace)
{
    if (workspace->changed != NULL) {
        workspace->changed(workspace, workspace->user_data);
    }
}

static GcTerminalSession *
session_for_page(GtkWidget *page)
{
    if (page == NULL) {
        return NULL;
    }

    return g_object_get_data(G_OBJECT(page), "goreecloud-terminal-session");
}

static void
update_tab_label(GcTerminalSession *session)
{
    GtkWidget *page = gc_terminal_session_get_widget(session);
    GtkLabel *label = g_object_get_data(G_OBJECT(page), "goreecloud-tab-label");
    g_autofree char *title = gc_terminal_session_dup_display_title(session);

    if (label == NULL) {
        return;
    }

    gtk_label_set_text(label, title);
    gtk_widget_set_tooltip_text(GTK_WIDGET(label), title);
}

static void
on_session_changed(GcTerminalSession *session, gpointer user_data)
{
    GcWorkspace *workspace = user_data;
    GtkWidget *current;

    update_tab_label(session);

    current = gtk_notebook_get_nth_page(
        workspace->notebook,
        gtk_notebook_get_current_page(workspace->notebook)
    );

    if (current == gc_terminal_session_get_widget(session)) {
        notify_changed(workspace);
    }
}

static void
on_switch_page(
    GtkNotebook *notebook,
    GtkWidget *page,
    guint page_num,
    gpointer user_data
)
{
    GcWorkspace *workspace = user_data;
    GcTerminalSession *session = session_for_page(page);
    (void) notebook;
    (void) page_num;

    notify_changed(workspace);
    if (session != NULL) {
        gc_terminal_session_focus(session);
    }
}

GcWorkspace *
gc_workspace_new(GcWorkspaceChangedFunc changed, gpointer user_data)
{
    GcWorkspace *workspace = g_new0(GcWorkspace, 1);
    GtkWidget *notebook = gtk_notebook_new();

    workspace->notebook = GTK_NOTEBOOK(notebook);
    workspace->changed = changed;
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
    gint page_num = gtk_notebook_get_current_page(workspace->notebook);
    GtkWidget *page;

    if (page_num < 0) {
        return NULL;
    }

    page = gtk_notebook_get_nth_page(workspace->notebook, page_num);
    return session_for_page(page);
}

guint
gc_workspace_get_count(GcWorkspace *workspace)
{
    return (guint) gtk_notebook_get_n_pages(workspace->notebook);
}

void
gc_workspace_add_tab(GcWorkspace *workspace, const char *working_directory)
{
    GcTerminalSession *session = gc_terminal_session_new(
        working_directory,
        on_session_changed,
        workspace
    );
    GtkWidget *page = gc_terminal_session_get_widget(session);
    GtkWidget *label = gtk_label_new("Terminal");
    gint page_num;

    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 28);
    g_object_set_data(G_OBJECT(page), "goreecloud-tab-label", label);

    page_num = gtk_notebook_append_page(workspace->notebook, page, label);
    gtk_notebook_set_tab_reorderable(workspace->notebook, page, TRUE);
    update_tab_label(session);
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
