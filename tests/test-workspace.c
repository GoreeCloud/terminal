#include "gc-workspace.h"

#include <gtk/gtk.h>

typedef struct {
    guint requests;
    GcTerminalPasteSource source;
} PasteProbe;

static void
on_paste_requested(
    GcWorkspace *workspace,
    GcTerminalSession *session,
    GcTerminalPasteSource source,
    gpointer user_data
)
{
    PasteProbe *probe = user_data;
    (void) workspace;
    (void) session;

    probe->requests++;
    probe->source = source;
}

static void
test_paste_source_routing(void)
{
    PasteProbe probe = {0};
    GcWorkspace *workspace = gc_workspace_new(
        NULL,
        on_paste_requested,
        NULL,
        &probe
    );
    GtkWidget *widget = gc_workspace_get_widget(workspace);
    GcTerminalSession *session;

    g_object_ref_sink(widget);
    gc_workspace_add_tab(workspace, g_get_home_dir());
    session = gc_workspace_get_current_session(workspace);

    g_assert_nonnull(session);

    gc_terminal_session_request_paste(
        session,
        GC_TERMINAL_PASTE_CLIPBOARD
    );
    g_assert_cmpuint(probe.requests, ==, 1);
    g_assert_cmpint(probe.source, ==, GC_TERMINAL_PASTE_CLIPBOARD);

    gc_terminal_session_request_paste(
        session,
        GC_TERMINAL_PASTE_PRIMARY
    );
    g_assert_cmpuint(probe.requests, ==, 2);
    g_assert_cmpint(probe.source, ==, GC_TERMINAL_PASTE_PRIMARY);

    g_object_unref(widget);
    gc_workspace_free(workspace);
}

static void
test_tab_chrome(void)
{
    GcWorkspace *workspace = gc_workspace_new(NULL, NULL, NULL, NULL);
    GtkWidget *widget = gc_workspace_get_widget(workspace);
    GtkNotebook *notebook = GTK_NOTEBOOK(widget);
    GtkWidget *add_button;
    GtkWidget *page;
    GtkWidget *tab;
    guint tab_children = 0;

    g_object_ref_sink(widget);

    add_button = gtk_notebook_get_action_widget(notebook, GTK_PACK_END);
    g_assert_nonnull(add_button);
    g_assert_true(GTK_IS_BUTTON(add_button));
    g_assert_true(gtk_widget_has_css_class(add_button, "gc-tab-add"));

    gc_workspace_add_tab(workspace, g_get_home_dir());
    gc_workspace_add_tab(workspace, g_get_home_dir());
    g_assert_cmpuint(gc_workspace_get_count(workspace), ==, 2);

    page = gtk_notebook_get_nth_page(notebook, 0);
    g_assert_nonnull(page);
    tab = gtk_notebook_get_tab_label(notebook, page);
    g_assert_nonnull(tab);
    g_assert_true(GTK_IS_BOX(tab));
    g_assert_true(gtk_widget_has_css_class(tab, "gc-tab-label"));

    for (GtkWidget *child = gtk_widget_get_first_child(tab);
         child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        tab_children++;
    }
    g_assert_cmpuint(tab_children, ==, 3);

    g_object_unref(widget);
    gc_workspace_free(workspace);
}

static void
test_split_and_close(void)
{
    GcWorkspace *workspace = gc_workspace_new(NULL, NULL, NULL, NULL);
    GtkWidget *widget = gc_workspace_get_widget(workspace);

    g_object_ref_sink(widget);

    gc_workspace_add_tab(workspace, g_get_home_dir());
    g_assert_cmpuint(gc_workspace_get_count(workspace), ==, 1);
    g_assert_cmpuint(gc_workspace_get_current_pane_count(workspace), ==, 1);

    g_assert_true(
        gc_workspace_split_current(workspace, GTK_ORIENTATION_HORIZONTAL)
    );
    g_assert_cmpuint(gc_workspace_get_current_pane_count(workspace), ==, 2);

    g_assert_true(
        gc_workspace_split_current(workspace, GTK_ORIENTATION_VERTICAL)
    );
    g_assert_cmpuint(gc_workspace_get_current_pane_count(workspace), ==, 3);

    gc_workspace_focus_relative_pane(workspace, -1);
    gc_workspace_focus_relative_pane(workspace, 1);

    g_assert_true(gc_workspace_close_current_pane(workspace));
    g_assert_cmpuint(gc_workspace_get_current_pane_count(workspace), ==, 2);
    g_assert_true(gc_workspace_close_current_pane(workspace));
    g_assert_cmpuint(gc_workspace_get_current_pane_count(workspace), ==, 1);
    g_assert_false(gc_workspace_close_current_pane(workspace));

    g_object_unref(widget);
    gc_workspace_free(workspace);
}

int
main(int argc, char **argv)
{
    gtk_init();
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/workspace/paste-source-routing", test_paste_source_routing);
    g_test_add_func("/workspace/tab-chrome", test_tab_chrome);
    g_test_add_func("/workspace/split-and-close", test_split_and_close);
    return g_test_run();
}
