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
        g_assert_cmpstr(first->layout, ==, "0");
        g_assert_cmpuint(first->active_pane, ==, 0);
        g_assert_cmpuint(first->pane_working_directories->len, ==, 1);
        g_assert_cmpstr(second->profile_id, ==, "work");
        g_assert_cmpstr(second->working_directory, ==, "/");
        g_assert_cmpstr(second->layout, ==, "0");
        g_assert_cmpuint(second->active_pane, ==, 0);
    }

    gc_session_store_clear(&saved);
    gc_session_store_clear(&loaded);
    g_remove(path);
    g_rmdir(directory);
}

static void
test_layout_round_trip(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-layout-state-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GPtrArray *panes = g_ptr_array_new();
    GcSessionStore saved;
    GcSessionStore loaded;

    g_assert_no_error(error);
    path = g_build_filename(directory, "workspace.ini", NULL);
    g_ptr_array_add(panes, (gpointer) "/tmp");
    g_ptr_array_add(panes, (gpointer) "/");
    g_ptr_array_add(panes, (gpointer) "/var");

    gc_session_store_init(&saved);
    gc_session_store_add_tab_layout(
        &saved,
        "default",
        "H(0,V(1,2))",
        panes,
        2
    );
    g_assert_cmpuint(saved.tabs->len, ==, 1);
    g_assert_true(gc_session_store_save(path, &saved, &error));
    g_assert_no_error(error);

    gc_session_store_init(&loaded);
    g_assert_true(gc_session_store_load(path, &loaded, &error));
    g_assert_no_error(error);
    g_assert_cmpuint(loaded.version, ==, GC_SESSION_STORE_VERSION);
    g_assert_cmpuint(loaded.tabs->len, ==, 1);

    {
        GcSessionStoreTab *tab = g_ptr_array_index(loaded.tabs, 0);

        g_assert_cmpstr(tab->layout, ==, "H(0,V(1,2))");
        g_assert_cmpuint(tab->active_pane, ==, 2);
        g_assert_cmpuint(tab->pane_working_directories->len, ==, 3);
        g_assert_cmpstr(
            g_ptr_array_index(tab->pane_working_directories, 0),
            ==,
            "/tmp"
        );
        g_assert_cmpstr(
            g_ptr_array_index(tab->pane_working_directories, 2),
            ==,
            "/var"
        );
    }

    gc_session_store_clear(&saved);
    gc_session_store_clear(&loaded);
    g_ptr_array_unref(panes);
    g_remove(path);
    g_rmdir(directory);
}

static void
test_version_one_migrates_to_current_schema(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-v1-state-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcSessionStore loaded;
    const char *data =
        "[workspace]\n"
        "version=1\n"
        "tab-count=1\n"
        "current-tab=0\n"
        "[tab 0]\n"
        "profile=default\n"
        "working-directory=/tmp\n";

    g_assert_no_error(error);
    path = g_build_filename(directory, "workspace.ini", NULL);
    g_assert_true(g_file_set_contents(path, data, -1, &error));
    g_assert_no_error(error);

    gc_session_store_init(&loaded);
    g_assert_true(gc_session_store_load(path, &loaded, &error));
    g_assert_no_error(error);
    g_assert_cmpuint(loaded.version, ==, GC_SESSION_STORE_VERSION);
    g_assert_cmpuint(loaded.tabs->len, ==, 1);

    {
        GcSessionStoreTab *tab = g_ptr_array_index(loaded.tabs, 0);

        g_assert_cmpstr(tab->layout, ==, "0");
        g_assert_cmpuint(tab->active_pane, ==, 0);
        g_assert_cmpuint(tab->pane_working_directories->len, ==, 1);
        g_assert_cmpstr(
            g_ptr_array_index(tab->pane_working_directories, 0),
            ==,
            "/tmp"
        );
    }

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
test_recovers_from_previous_snapshot(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-local-recovery-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    g_autofree char *backup_path = NULL;
    GcSessionStore first;
    GcSessionStore second;
    GcSessionStore loaded;

    g_assert_no_error(error);
    path = g_build_filename(directory, "workspace.ini", NULL);
    backup_path = g_strdup_printf("%s.bak", path);

    gc_session_store_init(&first);
    gc_session_store_add_tab(&first, "default", "/tmp");
    first.current_tab = 0;
    g_assert_true(gc_session_store_save(path, &first, &error));
    g_assert_no_error(error);

    gc_session_store_init(&second);
    gc_session_store_add_tab(&second, "work", "/");
    second.current_tab = 0;
    g_assert_true(gc_session_store_save(path, &second, &error));
    g_assert_no_error(error);
    g_assert_true(g_file_test(backup_path, G_FILE_TEST_EXISTS));

    g_assert_true(
        g_file_set_contents(
            path,
            "[workspace]\nversion=not-an-integer\n",
            -1,
            &error
        )
    );
    g_assert_no_error(error);

    gc_session_store_init(&loaded);
    g_assert_true(gc_session_store_load(path, &loaded, &error));
    g_assert_no_error(error);
    g_assert_cmpuint(loaded.tabs->len, ==, 1);

    {
        GcSessionStoreTab *tab = g_ptr_array_index(loaded.tabs, 0);

        g_assert_cmpstr(tab->profile_id, ==, "default");
        g_assert_cmpstr(tab->working_directory, ==, "/tmp");
    }

    gc_session_store_clear(&first);
    gc_session_store_clear(&second);
    gc_session_store_clear(&loaded);
    g_remove(path);
    g_remove(backup_path);
    g_rmdir(directory);
}

static void
test_recovers_when_primary_is_missing(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-local-missing-primary-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    g_autofree char *backup_path = NULL;
    GcSessionStore first;
    GcSessionStore second;
    GcSessionStore loaded;

    g_assert_no_error(error);
    path = g_build_filename(directory, "workspace.ini", NULL);
    backup_path = g_strdup_printf("%s.bak", path);

    gc_session_store_init(&first);
    gc_session_store_add_tab(&first, "default", "/tmp");
    g_assert_true(gc_session_store_save(path, &first, &error));
    g_assert_no_error(error);

    gc_session_store_init(&second);
    gc_session_store_add_tab(&second, "work", "/");
    g_assert_true(gc_session_store_save(path, &second, &error));
    g_assert_no_error(error);

    g_assert_cmpint(g_remove(path), ==, 0);
    g_assert_true(g_file_test(backup_path, G_FILE_TEST_EXISTS));

    gc_session_store_init(&loaded);
    g_assert_true(gc_session_store_load(path, &loaded, &error));
    g_assert_no_error(error);
    g_assert_cmpuint(loaded.tabs->len, ==, 1);

    {
        GcSessionStoreTab *tab = g_ptr_array_index(loaded.tabs, 0);

        g_assert_cmpstr(tab->profile_id, ==, "default");
        g_assert_cmpstr(tab->working_directory, ==, "/tmp");
    }

    gc_session_store_clear(&first);
    gc_session_store_clear(&second);
    gc_session_store_clear(&loaded);
    g_remove(backup_path);
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
    g_autofree char *backup_path = NULL;
    GcSessionStore state;
    const char *data =
        "[workspace]\n"
        "version=99\n"
        "tab-count=0\n"
        "current-tab=0\n";
    const char *backup_data =
        "[workspace]\n"
        "version=1\n"
        "tab-count=1\n"
        "current-tab=0\n"
        "[tab 0]\n"
        "profile=default\n"
        "working-directory=/tmp\n";

    g_assert_no_error(error);
    path = g_build_filename(directory, "workspace.ini", NULL);
    backup_path = g_strdup_printf("%s.bak", path);
    g_assert_true(g_file_set_contents(path, data, -1, &error));
    g_assert_no_error(error);
    g_assert_true(
        g_file_set_contents(backup_path, backup_data, -1, &error)
    );
    g_assert_no_error(error);

    gc_session_store_init(&state);
    g_assert_false(gc_session_store_load(path, &state, &error));
    g_assert_error(
        error,
        g_quark_from_static_string("goreecloud-terminal-session-store-error"),
        1
    );
    g_clear_error(&error);

    gc_session_store_clear(&state);
    g_remove(path);
    g_remove(backup_path);
    g_rmdir(directory);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/local-state/round-trip", test_round_trip);
    g_test_add_func("/local-state/layout-round-trip", test_layout_round_trip);
    g_test_add_func(
        "/local-state/version-one-migration",
        test_version_one_migrates_to_current_schema
    );
    g_test_add_func("/local-state/missing", test_missing_file_is_empty_state);
    g_test_add_func(
        "/local-state/recover-corrupt-primary",
        test_recovers_from_previous_snapshot
    );
    g_test_add_func(
        "/local-state/recover-missing-primary",
        test_recovers_when_primary_is_missing
    );
    g_test_add_func(
        "/local-state/unsupported-version",
        test_rejects_unsupported_version
    );
    return g_test_run();
}
