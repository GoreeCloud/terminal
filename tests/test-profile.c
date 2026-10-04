#include "gc-profile.h"

#include <glib/gstdio.h>

static void
test_default_profile(void)
{
    GcProfileStore *store = gc_profile_store_new();
    const GcProfile *profile = gc_profile_store_get_default(store);

    g_assert_nonnull(profile);
    g_assert_cmpstr(gc_profile_get_id(profile), ==, "default");
    g_assert_cmpstr(gc_profile_get_name(profile), ==, "Default");
    g_assert_cmpuint(gc_profile_store_get_count(store), ==, 1);
    g_assert_cmpint(
        gc_profile_get_cursor_shape(profile),
        ==,
        GC_PROFILE_CURSOR_SHAPE_BLOCK
    );
    g_assert_cmpint(
        gc_profile_get_cursor_blink(profile),
        ==,
        GC_PROFILE_CURSOR_BLINK_SYSTEM
    );
    g_assert_true(gc_profile_get_bold_is_bright(profile));
    g_assert_cmpint(gc_profile_get_scrollback_lines(profile), ==, 10000);

    gc_profile_store_free(store);
}

static void
test_load_profiles(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-profiles-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcProfileStore *store;
    const GcProfile *profile;
    g_autofree char *cwd = NULL;
    const char *data =
        "[profile work]\n"
        "name=Work shell\n"
        "shell=/bin/sh\n"
        "working-directory=~\n"
        "startup-command=printf 'profile ready'\n"
        "font=Monospace 12\n"
        "foreground=#ffffff\n"
        "background=#101820\n"
        "cursor-shape=ibeam\n"
        "cursor-blink=off\n"
        "bold-is-bright=false\n"
        "scrollback-lines=50000\n"
        "environment=TERM_PROGRAM=GoreeCloud Terminal;GC_TEST=value=with=equals;\n";

    g_assert_no_error(error);
    path = g_build_filename(directory, "profiles.ini", NULL);
    g_assert_true(g_file_set_contents(path, data, -1, &error));
    g_assert_no_error(error);

    store = gc_profile_store_new();
    g_assert_true(gc_profile_store_load(store, path, &error));
    g_assert_no_error(error);
    g_assert_cmpuint(gc_profile_store_get_count(store), ==, 2);

    profile = gc_profile_store_lookup(store, "work");
    g_assert_nonnull(profile);
    g_assert_cmpstr(gc_profile_get_name(profile), ==, "Work shell");
    g_assert_cmpstr(gc_profile_get_shell(profile), ==, "/bin/sh");
    g_assert_cmpstr(
        gc_profile_get_startup_command(profile),
        ==,
        "printf 'profile ready'"
    );
    g_assert_cmpstr(gc_profile_get_font(profile), ==, "Monospace 12");
    g_assert_cmpint(
        gc_profile_get_cursor_shape(profile),
        ==,
        GC_PROFILE_CURSOR_SHAPE_IBEAM
    );
    g_assert_cmpint(
        gc_profile_get_cursor_blink(profile),
        ==,
        GC_PROFILE_CURSOR_BLINK_OFF
    );
    g_assert_false(gc_profile_get_bold_is_bright(profile));
    g_assert_cmpint(gc_profile_get_scrollback_lines(profile), ==, 50000);
    {
        g_auto(GStrv) environment = gc_profile_dup_environment(profile);

        g_assert_nonnull(environment);
        g_assert_cmpstr(
            environment[0],
            ==,
            "TERM_PROGRAM=GoreeCloud Terminal"
        );
        g_assert_cmpstr(environment[1], ==, "GC_TEST=value=with=equals");
        g_assert_null(environment[2]);
    }

    g_setenv("GC_PROFILE_INHERITED_TEST", "parent", TRUE);
    {
        g_auto(GStrv) environment =
            gc_profile_dup_spawn_environment(profile);

        g_assert_nonnull(environment);
        g_assert_cmpstr(
            g_environ_getenv(environment, "GC_PROFILE_INHERITED_TEST"),
            ==,
            "parent"
        );
        g_assert_cmpstr(
            g_environ_getenv(environment, "TERM_PROGRAM"),
            ==,
            "GoreeCloud Terminal"
        );
        g_assert_cmpstr(
            g_environ_getenv(environment, "GC_TEST"),
            ==,
            "value=with=equals"
        );
    }
    g_unsetenv("GC_PROFILE_INHERITED_TEST");

    cwd = gc_profile_dup_effective_working_directory(profile, "/");
    g_assert_cmpstr(cwd, ==, g_get_home_dir());

    gc_profile_store_free(store);
    g_remove(path);
    g_rmdir(directory);
}

static void
test_invalid_profile_does_not_replace_store(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-profiles-invalid-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcProfileStore *store;
    const char *valid_data =
        "[profile work]\n"
        "name=Work\n"
        "shell=/bin/sh\n";
    const char *invalid_data =
        "[profile bad]\n"
        "shell=relative-shell\n";

    g_assert_no_error(error);
    path = g_build_filename(directory, "profiles.ini", NULL);

    store = gc_profile_store_new();
    g_assert_true(g_file_set_contents(path, valid_data, -1, &error));
    g_assert_no_error(error);
    g_assert_true(gc_profile_store_load(store, path, &error));
    g_assert_no_error(error);
    g_assert_nonnull(gc_profile_store_lookup(store, "work"));

    g_assert_true(g_file_set_contents(path, invalid_data, -1, &error));
    g_assert_no_error(error);
    g_assert_false(gc_profile_store_load(store, path, &error));
    g_assert_error(error, g_quark_from_static_string("goreecloud-terminal-profile-error"), 3);
    g_clear_error(&error);

    g_assert_nonnull(gc_profile_store_lookup(store, "work"));
    g_assert_null(gc_profile_store_lookup(store, "bad"));

    gc_profile_store_free(store);
    g_remove(path);
    g_rmdir(directory);
}

static void
test_invalid_environment_preserves_store(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-profiles-env-invalid-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcProfileStore *store;
    const char *valid_data =
        "[profile work]\n"
        "environment=GOOD=value;\n";
    const char *invalid_data =
        "[profile bad]\n"
        "environment=1BAD=value;\n";

    g_assert_no_error(error);
    path = g_build_filename(directory, "profiles.ini", NULL);

    store = gc_profile_store_new();
    g_assert_true(g_file_set_contents(path, valid_data, -1, &error));
    g_assert_no_error(error);
    g_assert_true(gc_profile_store_load(store, path, &error));
    g_assert_no_error(error);

    g_assert_true(g_file_set_contents(path, invalid_data, -1, &error));
    g_assert_no_error(error);
    g_assert_false(gc_profile_store_load(store, path, &error));
    g_assert_error(
        error,
        g_quark_from_static_string("goreecloud-terminal-profile-error"),
        5
    );
    g_clear_error(&error);

    g_assert_nonnull(gc_profile_store_lookup(store, "work"));
    g_assert_null(gc_profile_store_lookup(store, "bad"));

    gc_profile_store_free(store);
    g_remove(path);
    g_rmdir(directory);
}

static void
test_oversized_startup_command_preserves_store(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-profiles-startup-invalid-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    g_autofree char *oversized = NULL;
    g_autofree char *invalid_data = NULL;
    GcProfileStore *store;
    const char *valid_data =
        "[profile work]\n"
        "startup-command=printf ready\n";

    g_assert_no_error(error);
    path = g_build_filename(directory, "profiles.ini", NULL);
    oversized = g_strnfill(4097, 'x');
    invalid_data = g_strdup_printf(
        "[profile bad]\nstartup-command=%s\n",
        oversized
    );

    store = gc_profile_store_new();
    g_assert_true(g_file_set_contents(path, valid_data, -1, &error));
    g_assert_no_error(error);
    g_assert_true(gc_profile_store_load(store, path, &error));
    g_assert_no_error(error);
    g_assert_nonnull(gc_profile_store_lookup(store, "work"));

    g_assert_true(g_file_set_contents(path, invalid_data, -1, &error));
    g_assert_no_error(error);
    g_assert_false(gc_profile_store_load(store, path, &error));
    g_assert_error(
        error,
        g_quark_from_static_string("goreecloud-terminal-profile-error"),
        8
    );
    g_clear_error(&error);

    g_assert_nonnull(gc_profile_store_lookup(store, "work"));
    g_assert_null(gc_profile_store_lookup(store, "bad"));

    gc_profile_store_free(store);
    g_remove(path);
    g_rmdir(directory);
}

static void
test_invalid_cursor_appearance_preserves_store(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-profiles-cursor-invalid-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcProfileStore *store;
    const char *valid_data =
        "[profile work]\n"
        "cursor-shape=underline\n"
        "cursor-blink=on\n"
        "bold-is-bright=true\n";
    const char *invalid_shape =
        "[profile bad]\n"
        "cursor-shape=triangle\n";
    const char *invalid_blink =
        "[profile bad]\n"
        "cursor-blink=fast\n";
    const char *invalid_bold =
        "[profile bad]\n"
        "bold-is-bright=maybe\n";

    g_assert_no_error(error);
    path = g_build_filename(directory, "profiles.ini", NULL);

    store = gc_profile_store_new();
    g_assert_true(g_file_set_contents(path, valid_data, -1, &error));
    g_assert_no_error(error);
    g_assert_true(gc_profile_store_load(store, path, &error));
    g_assert_no_error(error);

    g_assert_true(g_file_set_contents(path, invalid_shape, -1, &error));
    g_assert_no_error(error);
    g_assert_false(gc_profile_store_load(store, path, &error));
    g_assert_error(
        error,
        g_quark_from_static_string("goreecloud-terminal-profile-error"),
        9
    );
    g_clear_error(&error);
    g_assert_nonnull(gc_profile_store_lookup(store, "work"));

    g_assert_true(g_file_set_contents(path, invalid_blink, -1, &error));
    g_assert_no_error(error);
    g_assert_false(gc_profile_store_load(store, path, &error));
    g_assert_error(
        error,
        g_quark_from_static_string("goreecloud-terminal-profile-error"),
        10
    );
    g_clear_error(&error);
    g_assert_nonnull(gc_profile_store_lookup(store, "work"));

    g_assert_true(g_file_set_contents(path, invalid_bold, -1, &error));
    g_assert_no_error(error);
    g_assert_false(gc_profile_store_load(store, path, &error));
    g_assert_error(
        error,
        g_quark_from_static_string("goreecloud-terminal-profile-error"),
        11
    );
    g_clear_error(&error);
    g_assert_nonnull(gc_profile_store_lookup(store, "work"));

    gc_profile_store_free(store);
    g_remove(path);
    g_rmdir(directory);
}

static void
test_invalid_scrollback_preserves_store(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-profiles-scrollback-invalid-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    GcProfileStore *store;
    const char *valid_data =
        "[profile work]\n"
        "scrollback-lines=25000\n";
    const char *invalid_data =
        "[profile bad]\n"
        "scrollback-lines=1000001\n";

    g_assert_no_error(error);
    path = g_build_filename(directory, "profiles.ini", NULL);

    store = gc_profile_store_new();
    g_assert_true(g_file_set_contents(path, valid_data, -1, &error));
    g_assert_no_error(error);
    g_assert_true(gc_profile_store_load(store, path, &error));
    g_assert_no_error(error);

    g_assert_true(g_file_set_contents(path, invalid_data, -1, &error));
    g_assert_no_error(error);
    g_assert_false(gc_profile_store_load(store, path, &error));
    g_assert_error(
        error,
        g_quark_from_static_string("goreecloud-terminal-profile-error"),
        7
    );
    g_clear_error(&error);

    g_assert_nonnull(gc_profile_store_lookup(store, "work"));
    g_assert_null(gc_profile_store_lookup(store, "bad"));

    gc_profile_store_free(store);
    g_remove(path);
    g_rmdir(directory);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/profiles/default", test_default_profile);
    g_test_add_func("/profiles/load", test_load_profiles);
    g_test_add_func(
        "/profiles/invalid-preserves-store",
        test_invalid_profile_does_not_replace_store
    );
    g_test_add_func(
        "/profiles/invalid-environment-preserves-store",
        test_invalid_environment_preserves_store
    );
    g_test_add_func(
        "/profiles/invalid-startup-command-preserves-store",
        test_oversized_startup_command_preserves_store
    );
    g_test_add_func(
        "/profiles/invalid-cursor-appearance-preserves-store",
        test_invalid_cursor_appearance_preserves_store
    );
    g_test_add_func(
        "/profiles/invalid-scrollback-preserves-store",
        test_invalid_scrollback_preserves_store
    );
    return g_test_run();
}
