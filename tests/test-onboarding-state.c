#include "gc-onboarding-state.h"

#include <glib.h>
#include <glib/gstdio.h>

static void
test_missing_file_defaults(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-onboarding-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcOnboardingState state;

    g_assert_no_error(error);
    g_assert_nonnull(directory);

    path = g_build_filename(directory, "state.ini", NULL);
    g_assert_true(gc_onboarding_state_load(path, &state, &error));
    g_assert_no_error(error);
    g_assert_false(state.completed);
    g_assert_cmpuint(state.step, ==, 0);

    g_rmdir(directory);
}

static void
test_round_trip(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-onboarding-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcOnboardingState written = {
        .completed = TRUE,
        .step = 3,
    };
    GcOnboardingState loaded;

    g_assert_no_error(error);
    g_assert_nonnull(directory);

    path = g_build_filename(directory, "state.ini", NULL);
    g_assert_true(gc_onboarding_state_save(path, &written, &error));
    g_assert_no_error(error);
    g_assert_true(gc_onboarding_state_load(path, &loaded, &error));
    g_assert_no_error(error);
    g_assert_true(loaded.completed);
    g_assert_cmpuint(loaded.step, ==, 3);

    g_remove(path);
    g_rmdir(directory);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/onboarding/missing-file-defaults", test_missing_file_defaults);
    g_test_add_func("/onboarding/round-trip", test_round_trip);
    return g_test_run();
}
