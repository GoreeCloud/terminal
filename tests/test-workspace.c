#include "gc-workspace.h"

#include <gtk/gtk.h>

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
    g_test_add_func("/workspace/split-and-close", test_split_and_close);
    return g_test_run();
}
