#include "gc-link-utils.h"

#include <glib.h>
#include <glib/gstdio.h>

static void
test_allowed_schemes(void)
{
    g_assert_true(gc_link_uri_scheme_allowed("https"));
    g_assert_true(gc_link_uri_scheme_allowed("HTTP"));
    g_assert_true(gc_link_uri_scheme_allowed("mailto"));
    g_assert_true(gc_link_uri_scheme_allowed("file"));
    g_assert_false(gc_link_uri_scheme_allowed("javascript"));
    g_assert_false(gc_link_uri_scheme_allowed("ssh"));
    g_assert_false(gc_link_uri_scheme_allowed(NULL));
}

static void
test_uri_validation(void)
{
    GError *error = NULL;
    g_autofree char *uri = gc_link_target_to_uri(
        GC_LINK_TARGET_URI,
        "https://example.com/docs",
        NULL,
        &error
    );

    g_assert_no_error(error);
    g_assert_cmpstr(uri, ==, "https://example.com/docs");

    g_clear_pointer(&uri, g_free);
    uri = gc_link_target_to_uri(
        GC_LINK_TARGET_URI,
        "javascript:alert(1)",
        NULL,
        &error
    );
    g_assert_null(uri);
    g_assert_error(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED);
    g_clear_error(&error);
}

static void
test_path_resolution(void)
{
    GError *error = NULL;
    g_autofree char *directory = g_dir_make_tmp(
        "goreecloud-terminal-links-XXXXXX",
        &error
    );
    g_autofree char *path = NULL;
    g_autofree char *uri = NULL;
    g_autofree char *expected = NULL;

    g_assert_no_error(error);
    g_assert_nonnull(directory);

    path = g_build_filename(directory, "report.txt", NULL);
    g_assert_true(g_file_set_contents(path, "ok", -1, &error));
    g_assert_no_error(error);

    uri = gc_link_target_to_uri(
        GC_LINK_TARGET_PATH,
        "./report.txt",
        directory,
        &error
    );
    g_assert_no_error(error);
    g_assert_nonnull(uri);

    expected = g_filename_to_uri(path, NULL, &error);
    g_assert_no_error(error);
    g_assert_cmpstr(uri, ==, expected);

    g_clear_pointer(&uri, g_free);
    uri = gc_link_target_to_uri(
        GC_LINK_TARGET_PATH,
        "./missing.txt",
        directory,
        &error
    );
    g_assert_null(uri);
    g_assert_error(error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND);
    g_clear_error(&error);

    g_remove(path);
    g_rmdir(directory);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/links/allowed-schemes", test_allowed_schemes);
    g_test_add_func("/links/uri-validation", test_uri_validation);
    g_test_add_func("/links/path-resolution", test_path_resolution);
    return g_test_run();
}
