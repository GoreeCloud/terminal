#include "gc-workspace-layout.h"

#include <glib.h>

static void
test_parse_and_serialize(void)
{
    GError *error = NULL;
    GcWorkspaceLayoutNode *layout;
    g_autofree char *serialized = NULL;

    layout = gc_workspace_layout_parse(
        "H(0,V(1,2))",
        3,
        &error
    );
    g_assert_no_error(error);
    g_assert_nonnull(layout);

    serialized = gc_workspace_layout_serialize(layout);
    g_assert_cmpstr(serialized, ==, "H(0,V(1,2))");

    gc_workspace_layout_free(layout);
}

static void
test_rejects_missing_pane(void)
{
    GError *error = NULL;
    GcWorkspaceLayoutNode *layout;

    layout = gc_workspace_layout_parse("H(0,1)", 3, &error);
    g_assert_null(layout);
    g_assert_error(
        error,
        g_quark_from_static_string(
            "goreecloud-terminal-workspace-layout-error"
        ),
        1
    );
    g_clear_error(&error);
}

static void
test_rejects_duplicate_pane(void)
{
    GError *error = NULL;
    GcWorkspaceLayoutNode *layout;

    layout = gc_workspace_layout_parse("H(0,0)", 2, &error);
    g_assert_null(layout);
    g_assert_error(
        error,
        g_quark_from_static_string(
            "goreecloud-terminal-workspace-layout-error"
        ),
        1
    );
    g_clear_error(&error);
}

static void
test_rejects_out_of_range_pane(void)
{
    GError *error = NULL;
    GcWorkspaceLayoutNode *layout;

    layout = gc_workspace_layout_parse("H(0,2)", 2, &error);
    g_assert_null(layout);
    g_assert_error(
        error,
        g_quark_from_static_string(
            "goreecloud-terminal-workspace-layout-error"
        ),
        1
    );
    g_clear_error(&error);
}

static void
test_rejects_excessive_depth(void)
{
    GError *error = NULL;
    GcWorkspaceLayoutNode *layout;
    const char *text =
        "H(0,H(1,H(2,H(3,H(4,H(5,H(6,H(7,H(8,9)))))))))";

    layout = gc_workspace_layout_parse(text, 10, &error);
    g_assert_null(layout);
    g_assert_error(
        error,
        g_quark_from_static_string(
            "goreecloud-terminal-workspace-layout-error"
        ),
        1
    );
    g_clear_error(&error);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(
        "/workspace-layout/parse-serialize",
        test_parse_and_serialize
    );
    g_test_add_func(
        "/workspace-layout/missing-pane",
        test_rejects_missing_pane
    );
    g_test_add_func(
        "/workspace-layout/duplicate-pane",
        test_rejects_duplicate_pane
    );
    g_test_add_func(
        "/workspace-layout/out-of-range",
        test_rejects_out_of_range_pane
    );
    g_test_add_func(
        "/workspace-layout/depth",
        test_rejects_excessive_depth
    );
    return g_test_run();
}
