#include "gc-session-store.h"

#include <glib/gstdio.h>

static void
test_round_trip(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-local-state-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcSessionStore saved;
    GcSessionStore loaded;

    g_assert_no_error(error);
    path = g_build_filename(directory, "workspace.ini", NULL);

    gc_session_store_init(&saved);
    gc_session_store_add_tab(&saved, "default", "/tmp");
    gc_session_store_add_tab(&saved, "work", "/");
    saved.current_tab = 1;

    g_assert_true(gc_session_store_save(path, &saved, &error));
    g_assert_no_error(error);

    gc_session_store_init(&loaded);
    g_assert_true(gc_session_store_load(path, &loaded, &error));
    g_assert_no_error(error);
    g_assert_cmpuint(loaded.version, ==, GC_SESSION_STORE_VERSION);
    g_assert_cmpuint(loaded.tabs->len, ==, 2);
    g_assert_cmpuint(loaded.current_tab, ==, 1);

    {
        GcSessionStoreTab *first = g_ptr_array_index(loaded.tabs, 0);
        GcSessionStoreTab *second = g_ptr_array_index(loaded.tabs, 1);

        g_assert_cmpstr(first->profile_id, ==, "default");
        g_assert_cmpstr(first->working_directory, ==, "/tmp");
        g_assert_cmpstr(second->profile_id, ==, "work");
        g_assert_cmpstr(second->working_directory, ==, "/");
    }

    gc_session_store_clear(&saved);
    gc_session_store_clear(&loaded);
    g_remove(path);
    g_rmdir(directory);
}

static void
test_missing_file_is_empty_state(void)
{
    GError *error = NULL;
    GcSessionStore state;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-local-missing-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;

    g_assert_no_error(error);
    path = g_build_filename(directory, "missing.ini", NULL);

    gc_session_store_init(&state);
    gc_session_store_add_tab(&state, "stale", "/tmp");

    g_assert_true(gc_session_store_load(path, &state, &error));
    g_assert_no_error(error);
    g_assert_cmpuint(state.tabs->len, ==, 0);
    g_assert_cmpuint(state.current_tab, ==, 0);

    gc_session_store_clear(&state);
    g_rmdir(directory);
}

static void
test_rejects_unsupported_version(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-local-version-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcSessionStore state;
    const char *data =
        "[workspace]\n"
        "version=99\n"
        "tab-count=0\n"
        "current-tab=0\n";

    g_assert_no_error(error);
    path = g_build_filename(directory, "workspace.ini", NULL);
    g_assert_true(g_file_set_contents(path, data, -1, &error));
    g_assert_no_error(error);

    gc_session_store_init(&state);
    g_assert_false(gc_session_store_load(path, &state, &error));
    g_assert_error(
        error,
        G_KEY_FILE_ERROR,
        G_KEY_FILE_ERROR_INVALID_VALUE
    );
    g_clear_error(&error);

    gc_session_store_clear(&state);
    g_remove(path);
    g_rmdir(directory);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/local-state/round-trip", test_round_trip);
    g_test_add_func("/local-state/missing", test_missing_file_is_empty_state);
    g_test_add_func(
        "/local-state/unsupported-version",
        test_rejects_unsupported_version
    );
    return g_test_run();
}
