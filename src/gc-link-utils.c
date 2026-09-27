#include "gc-link-utils.h"

static gboolean
local_path_exists(const char *path, GError **error)
{
    if (g_file_test(path, G_FILE_TEST_EXISTS)) {
        return TRUE;
    }

    g_set_error(
        error,
        G_IO_ERROR,
        G_IO_ERROR_NOT_FOUND,
        "Local path does not exist: %s",
        path
    );
    return FALSE;
}

gboolean
gc_link_uri_scheme_allowed(const char *scheme)
{
    return scheme != NULL && (
        g_ascii_strcasecmp(scheme, "http") == 0 ||
        g_ascii_strcasecmp(scheme, "https") == 0 ||
        g_ascii_strcasecmp(scheme, "mailto") == 0 ||
        g_ascii_strcasecmp(scheme, "file") == 0
    );
}

static char *
validated_uri(const char *target, GError **error)
{
    GUri *uri;
    const char *scheme;

    uri = g_uri_parse(target, G_URI_FLAGS_PARSE_RELAXED, error);
    if (uri == NULL) {
        return NULL;
    }

    scheme = g_uri_get_scheme(uri);
    if (!gc_link_uri_scheme_allowed(scheme)) {
        g_set_error(
            error,
            G_IO_ERROR,
            G_IO_ERROR_NOT_SUPPORTED,
            "URI scheme is not allowed: %s",
            scheme != NULL ? scheme : "(none)"
        );
        g_uri_unref(uri);
        return NULL;
    }

    if (g_ascii_strcasecmp(scheme, "file") == 0) {
        g_autofree char *hostname = NULL;
        g_autofree char *path = g_filename_from_uri(target, &hostname, error);

        g_uri_unref(uri);

        if (path == NULL) {
            return NULL;
        }

        if (hostname != NULL && *hostname != '\0' &&
            g_ascii_strcasecmp(hostname, "localhost") != 0) {
            g_set_error(
                error,
                G_IO_ERROR,
                G_IO_ERROR_NOT_SUPPORTED,
                "Remote file URI hosts are not allowed"
            );
            return NULL;
        }

        if (!local_path_exists(path, error)) {
            return NULL;
        }

        return g_filename_to_uri(path, NULL, error);
    }

    g_uri_unref(uri);
    return g_strdup(target);
}

static char *
validated_path(
    const char *target,
    const char *working_directory,
    GError **error
)
{
    g_autofree char *expanded = NULL;
    g_autofree char *canonical = NULL;

    if (g_path_is_absolute(target)) {
        expanded = g_strdup(target);
    } else if (g_str_has_prefix(target, "~/")) {
        expanded = g_build_filename(g_get_home_dir(), target + 2, NULL);
    } else if (g_str_has_prefix(target, "./") ||
               g_str_has_prefix(target, "../")) {
        const char *base =
            working_directory != NULL && *working_directory != '\0'
                ? working_directory
                : g_get_home_dir();
        expanded = g_build_filename(base, target, NULL);
    } else {
        g_set_error(
            error,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "Only absolute, ~/..., ./..., and ../... local paths are supported"
        );
        return NULL;
    }

    canonical = g_canonicalize_filename(expanded, NULL);
    if (!local_path_exists(canonical, error)) {
        return NULL;
    }

    return g_filename_to_uri(canonical, NULL, error);
}

char *
gc_link_target_to_uri(
    GcLinkTargetKind kind,
    const char *target,
    const char *working_directory,
    GError **error
)
{
    if (target == NULL || *target == '\0') {
        g_set_error(
            error,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "Link target is empty"
        );
        return NULL;
    }

    for (const unsigned char *cursor = (const unsigned char *) target;
         *cursor != '\0';
         cursor++) {
        if (g_ascii_iscntrl(*cursor)) {
            g_set_error(
                error,
                G_IO_ERROR,
                G_IO_ERROR_INVALID_ARGUMENT,
                "Link target contains control characters"
            );
            return NULL;
        }
    }

    switch (kind) {
    case GC_LINK_TARGET_URI:
        return validated_uri(target, error);
    case GC_LINK_TARGET_PATH:
        return validated_path(target, working_directory, error);
    default:
        g_set_error(
            error,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "Unknown link target kind"
        );
        return NULL;
    }
}
