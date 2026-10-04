#include "gc-session-store.h"

#include <errno.h>
#include <unistd.h>

#include <glib/gstdio.h>

#define GC_SESSION_STORE_MAX_TABS 64

static void
tab_free(gpointer data)
{
    GcSessionStoreTab *tab = data;

    g_free(tab->profile_id);
    g_free(tab->working_directory);
    g_free(tab);
}

void
gc_session_store_init(GcSessionStore *state)
{
    state->version = GC_SESSION_STORE_VERSION;
    state->current_tab = 0;
    state->tabs = g_ptr_array_new_with_free_func(tab_free);
}

void
gc_session_store_clear(GcSessionStore *state)
{
    if (state == NULL) {
        return;
    }

    g_clear_pointer(&state->tabs, g_ptr_array_unref);
    state->version = GC_SESSION_STORE_VERSION;
    state->current_tab = 0;
}

void
gc_session_store_add_tab(
    GcSessionStore *state,
    const char *profile_id,
    const char *working_directory
)
{
    GcSessionStoreTab *tab;

    if (state == NULL || state->tabs == NULL ||
        state->tabs->len >= GC_SESSION_STORE_MAX_TABS) {
        return;
    }

    tab = g_new0(GcSessionStoreTab, 1);
    tab->profile_id = g_strdup(
        profile_id != NULL && *profile_id != '\0'
            ? profile_id
            : "default"
    );
    tab->working_directory = g_strdup(
        working_directory != NULL && *working_directory != '\0'
            ? working_directory
            : g_get_home_dir()
    );
    g_ptr_array_add(state->tabs, tab);
}

static gboolean
ensure_parent_directory(const char *path, GError **error)
{
    g_autofree char *parent = g_path_get_dirname(path);

    if (g_mkdir_with_parents(parent, 0700) != 0) {
        int saved_errno = errno;

        g_set_error(
            error,
            G_FILE_ERROR,
            g_file_error_from_errno(saved_errno),
            "Unable to create session state directory %s: %s",
            parent,
            g_strerror(saved_errno)
        );
        return FALSE;
    }

    return TRUE;
}

gboolean
gc_session_store_load(
    const char *path,
    GcSessionStore *state,
    GError **error
)
{
    GKeyFile *key_file = g_key_file_new();
    GcSessionStore loaded;
    GError *local_error = NULL;
    gint version;
    gint tab_count;
    gint current_tab;

    gc_session_store_init(&loaded);

    if (path == NULL || !g_file_test(path, G_FILE_TEST_EXISTS)) {
        gc_session_store_clear(state);
        *state = loaded;
        g_key_file_unref(key_file);
        return TRUE;
    }

    if (!g_key_file_load_from_file(
            key_file,
            path,
            G_KEY_FILE_NONE,
            &local_error
        )) {
        g_propagate_prefixed_error(
            error,
            local_error,
            "Unable to read session state: "
        );
        gc_session_store_clear(&loaded);
        g_key_file_unref(key_file);
        return FALSE;
    }

    version = g_key_file_get_integer(
        key_file,
        "workspace",
        "version",
        &local_error
    );
    if (local_error != NULL) {
        g_propagate_prefixed_error(
            error,
            local_error,
            "Session state version is missing or invalid: "
        );
        gc_session_store_clear(&loaded);
        g_key_file_unref(key_file);
        return FALSE;
    }

    if (version != GC_SESSION_STORE_VERSION) {
        g_set_error(
            error,
            G_KEY_FILE_ERROR,
            G_KEY_FILE_ERROR_INVALID_VALUE,
            "Unsupported session state version %d",
            version
        );
        gc_session_store_clear(&loaded);
        g_key_file_unref(key_file);
        return FALSE;
    }

    tab_count = g_key_file_get_integer(
        key_file,
        "workspace",
        "tab-count",
        &local_error
    );
    if (local_error != NULL || tab_count < 0 ||
        tab_count > GC_SESSION_STORE_MAX_TABS) {
        g_clear_error(&local_error);
        g_set_error(
            error,
            G_KEY_FILE_ERROR,
            G_KEY_FILE_ERROR_INVALID_VALUE,
            "Session state tab-count is invalid"
        );
        gc_session_store_clear(&loaded);
        g_key_file_unref(key_file);
        return FALSE;
    }

    current_tab = g_key_file_get_integer(
        key_file,
        "workspace",
        "current-tab",
        NULL
    );
    if (current_tab < 0) {
        current_tab = 0;
    }

    for (gint i = 0; i < tab_count; i++) {
        g_autofree char *group = g_strdup_printf("tab %d", i);
        g_autofree char *profile_id = g_key_file_get_string(
            key_file,
            group,
            "profile",
            NULL
        );
        g_autofree char *working_directory = g_key_file_get_string(
            key_file,
            group,
            "working-directory",
            NULL
        );

        if (working_directory == NULL ||
            !g_path_is_absolute(working_directory)) {
            g_clear_pointer(&working_directory, g_free);
            working_directory = g_strdup(g_get_home_dir());
        }

        gc_session_store_add_tab(
            &loaded,
            profile_id,
            working_directory
        );
    }

    if (loaded.tabs->len > 0) {
        loaded.current_tab = MIN(
            (guint) current_tab,
            loaded.tabs->len - 1
        );
    }

    gc_session_store_clear(state);
    *state = loaded;
    g_key_file_unref(key_file);
    return TRUE;
}

gboolean
gc_session_store_save(
    const char *path,
    const GcSessionStore *state,
    GError **error
)
{
    GKeyFile *key_file;
    g_autofree char *data = NULL;
    gsize data_length = 0;
    g_autofree char *temporary_path = NULL;
    int saved_errno;

    if (path == NULL || state == NULL || state->tabs == NULL) {
        g_set_error(
            error,
            G_FILE_ERROR,
            G_FILE_ERROR_INVAL,
            "Session state path and state are required"
        );
        return FALSE;
    }

    if (!ensure_parent_directory(path, error)) {
        return FALSE;
    }

    key_file = g_key_file_new();
    g_key_file_set_integer(
        key_file,
        "workspace",
        "version",
        GC_SESSION_STORE_VERSION
    );
    g_key_file_set_integer(
        key_file,
        "workspace",
        "tab-count",
        (gint) state->tabs->len
    );
    g_key_file_set_integer(
        key_file,
        "workspace",
        "current-tab",
        (gint) state->current_tab
    );

    for (guint i = 0; i < state->tabs->len; i++) {
        GcSessionStoreTab *tab = g_ptr_array_index(state->tabs, i);
        g_autofree char *group = g_strdup_printf("tab %u", i);

        g_key_file_set_string(
            key_file,
            group,
            "profile",
            tab->profile_id != NULL ? tab->profile_id : "default"
        );
        g_key_file_set_string(
            key_file,
            group,
            "working-directory",
            tab->working_directory != NULL
                ? tab->working_directory
                : g_get_home_dir()
        );
    }

    data = g_key_file_to_data(key_file, &data_length, error);
    g_key_file_unref(key_file);
    if (data == NULL) {
        return FALSE;
    }

    temporary_path = g_strdup_printf(
        "%s.tmp.%u",
        path,
        (guint) getpid()
    );
    if (!g_file_set_contents(
            temporary_path,
            data,
            (gssize) data_length,
            error
        )) {
        return FALSE;
    }

    if (g_chmod(temporary_path, 0600) != 0) {
        saved_errno = errno;
        g_unlink(temporary_path);
        g_set_error(
            error,
            G_FILE_ERROR,
            g_file_error_from_errno(saved_errno),
            "Unable to protect session state file: %s",
            g_strerror(saved_errno)
        );
        return FALSE;
    }

    if (g_rename(temporary_path, path) != 0) {
        saved_errno = errno;
        g_unlink(temporary_path);
        g_set_error(
            error,
            G_FILE_ERROR,
            g_file_error_from_errno(saved_errno),
            "Unable to replace session state: %s",
            g_strerror(saved_errno)
        );
        return FALSE;
    }

    return TRUE;
}

char *
gc_session_store_default_path(void)
{
    return g_build_filename(
        g_get_user_state_dir(),
        "goreecloud-terminal",
        "workspace.ini",
        NULL
    );
}
