#include "gc-command-boundaries.h"

#include <glib.h>

static void
test_navigation(void)
{
    GcCommandBoundaries boundaries;
    gint64 target = -1;

    gc_command_boundaries_init(&boundaries);
    gc_command_boundaries_mark_start(&boundaries, 10);
    gc_command_boundaries_mark_start(&boundaries, 25);
    gc_command_boundaries_mark_start(&boundaries, 40);

    g_assert_cmpuint(gc_command_boundaries_get_count(&boundaries), ==, 3);

    g_assert_true(gc_command_boundaries_previous(&boundaries, 41, &target));
    g_assert_cmpint(target, ==, 40);

    g_assert_true(gc_command_boundaries_previous(&boundaries, 40, &target));
    g_assert_cmpint(target, ==, 25);

    g_assert_true(gc_command_boundaries_next(&boundaries, 10, &target));
    g_assert_cmpint(target, ==, 25);

    g_assert_false(gc_command_boundaries_previous(&boundaries, 10, &target));
    g_assert_false(gc_command_boundaries_next(&boundaries, 40, &target));

    gc_command_boundaries_clear(&boundaries);
}

static void
test_prune_and_deduplicate(void)
{
    GcCommandBoundaries boundaries;
    gint64 target = -1;

    gc_command_boundaries_init(&boundaries);
    gc_command_boundaries_mark_start(&boundaries, 5);
    gc_command_boundaries_mark_start(&boundaries, 5);
    gc_command_boundaries_mark_start(&boundaries, 12);
    gc_command_boundaries_mark_start(&boundaries, 20);

    g_assert_cmpuint(gc_command_boundaries_get_count(&boundaries), ==, 3);

    gc_command_boundaries_prune_before(&boundaries, 12);
    g_assert_cmpuint(gc_command_boundaries_get_count(&boundaries), ==, 2);
    g_assert_true(gc_command_boundaries_previous(&boundaries, 21, &target));
    g_assert_cmpint(target, ==, 20);
    g_assert_false(gc_command_boundaries_previous(&boundaries, 12, &target));

    gc_command_boundaries_clear(&boundaries);
}

static void
test_history_is_bounded(void)
{
    GcCommandBoundaries boundaries;
    gint64 target = -1;

    gc_command_boundaries_init(&boundaries);

    for (guint i = 0; i < GC_COMMAND_BOUNDARIES_MAX + 20; i++) {
        gc_command_boundaries_mark_start(&boundaries, (gint64) i);
    }

    g_assert_cmpuint(
        gc_command_boundaries_get_count(&boundaries),
        ==,
        GC_COMMAND_BOUNDARIES_MAX
    );
    g_assert_true(
        gc_command_boundaries_previous(
            &boundaries,
            GC_COMMAND_BOUNDARIES_MAX + 20,
            &target
        )
    );
    g_assert_cmpint(target, ==, GC_COMMAND_BOUNDARIES_MAX + 19);
    g_assert_false(gc_command_boundaries_previous(&boundaries, 20, &target));

    gc_command_boundaries_clear(&boundaries);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/command-boundaries/navigation", test_navigation);
    g_test_add_func(
        "/command-boundaries/prune-deduplicate",
        test_prune_and_deduplicate
    );
    g_test_add_func(
        "/command-boundaries/bounded-history",
        test_history_is_bounded
    );
    return g_test_run();
}
