#include "gc-session-store.h"

#include "gc-workspace-layout.h"

#include <errno.h>
#include <unistd.h>

#include <glib/gstdio.h>

#define GC_SESSION_STORE_MAX_TABS 64
#define GC_SESSION_STORE_ERROR_UNSUPPORTED_VERSION 1

static GQuark
session_store_error_quark(void)
{
    return g_quark_from_static_string(
        "goreecloud-terminal-session-store-error"
    );
}

static char *
normalized_working_directory(const char *working_directory)
{
    return g_strdup(
        working_directory != NULL &&
        *working_directory != '\0' &&
        g_path_is_absolute(working_directory)
            ? working_directory
            : g_get_home_dir()
    );
}

static void
tab_free(gpointer data)
{
    GcSessionStoreTab *tab = data;

    g_free(tab->profile_id);
    g_free(tab->working_directory);
    g_free(tab->layout);
    g_clear_pointer(&tab->pane_working_directories, g_ptr_array_unref);
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
gc_session_store_add_tab_layout(
    GcSessionStore *state,
    const char *profile_id,
    const char *layout,
    GPtrArray *pane_working_directories,
    guint active_pane
)
{
    GcSessionStoreTab *tab;
    GcWorkspaceLayoutNode *parsed;
    GError *error = NULL;
    guint pane_count;

    if (state == NULL || state->tabs == NULL ||
        state->tabs->len >= GC_SESSION_STORE_MAX_TABS ||
        pane_working_directories == NULL) {
        return;
    }

    pane_count = pane_working_directories->len;
    if (pane_count == 0 ||
        pane_count > GC_WORKSPACE_LAYOUT_MAX_PANES ||
        active_pane >= pane_count) {
        return;
    }

    parsed = gc_workspace_layout_parse(layout, pane_count, &error);
    if (parsed == NULL) {
        g_clear_error(&error);
        return;
    }
    gc_workspace_layout_free(parsed);

    tab = g_new0(GcSessionStoreTab, 1);
    tab->profile_id = g_strdup(
        profile_id != NULL && *profile_id != '\0'
            ? profile_id
            : "default"
    );
    tab->layout = g_strdup(layout);
    tab->active_pane = active_pane;
    tab->pane_working_directories =
        g_ptr_array_new_with_free_func(g_free);

    for (guint i = 0; i < pane_count; i++) {
        const char *cwd = g_ptr_array_index(
            pane_working_directories,
            i
        );

        g_ptr_array_add(
            tab->pane_working_directories,
            normalized_working_directory(cwd)
        );
    }

    tab->working_directory = g_strdup(
        g_ptr_array_index(
            tab->pane_working_directories,
            tab->active_pane
        )
    );
    g_ptr_array_add(state->tabs, tab);
}

void
gc_session_store_add_tab(
    GcSessionStore *state,
    const char *profile_id,
    const char *working_directory
)
{
    GPtrArray *panes = g_ptr_array_new();

    g_ptr_array_add(panes, (gpointer) working_directory);
    gc_session_store_add_tab_layout(
        state,
        profile_id,
        "0",
        panes,
        0
    );
    g_ptr_array_unref(panes);
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

static char *
session_store_backup_path(const char *path)
{
    return g_strdup_printf("%s.bak", path);
}

static gboolean
load_version_one_tab(
    GKeyFile *key_file,
    const char *group,
    GcSessionStore *loaded
)
{
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

    gc_session_store_add_tab(
        loaded,
        profile_id,
        working_directory
    );
    return TRUE;
}

static gboolean
load_version_two_tab(
    GKeyFile *key_file,
    gint tab_index,
    const char *group,
    GcSessionStore *loaded,
    GError **error
)
{
    g_autofree char *profile_id = NULL;
    g_autofree char *layout = NULL;
    GPtrArray *pane_working_directories = NULL;
    GcWorkspaceLayoutNode *parsed = NULL;
    GError *local_error = NULL;
    gint pane_count;
    gint active_pane;

    profile_id = g_key_file_get_string(
        key_file,
        group,
        "profile",
        NULL
    );
    layout = g_key_file_get_string(
        key_file,
        group,
        "layout",
        &local_error
    );
    if (local_error != NULL) {
        g_propagate_prefixed_error(
            error,
            local_error,
            "Session tab layout is missing or invalid: "
        );
        return FALSE;
    }

    pane_count = g_key_file_get_integer(
        key_file,
        group,
        "pane-count",
        &local_error
    );
    if (local_error != NULL || pane_count <= 0 ||
        pane_count > GC_WORKSPACE_LAYOUT_MAX_PANES) {
        g_clear_error(&local_error);
        g_set_error(
            error,
            G_KEY_FILE_ERROR,
            G_KEY_FILE_ERROR_INVALID_VALUE,
            "Session tab pane-count is invalid"
        );
        return FALSE;
    }

    active_pane = g_key_file_get_integer(
        key_file,
        group,
        "active-pane",
        &local_error
    );
    if (local_error != NULL || active_pane < 0 ||
        active_pane >= pane_count) {
        g_clear_error(&local_error);
        g_set_error(
            error,
            G_KEY_FILE_ERROR,
            G_KEY_FILE_ERROR_INVALID_VALUE,
            "Session tab active-pane is invalid"
        );
        return FALSE;
    }

    parsed = gc_workspace_layout_parse(
        layout,
        (guint) pane_count,
        &local_error
    );
    if (parsed == NULL) {
        g_propagate_prefixed_error(
            error,
            local_error,
            "Session tab layout is invalid: "
        );
        return FALSE;
    }
    gc_workspace_layout_free(parsed);

    pane_working_directories =
        g_ptr_array_new_with_free_func(g_free);

    for (gint pane = 0; pane < pane_count; pane++) {
        g_autofree char *pane_group = g_strdup_printf(
            "tab %d pane %d",
            tab_index,
            pane
        );
        g_autofree char *cwd = g_key_file_get_string(
            key_file,
            pane_group,
            "working-directory",
            NULL
        );

        g_ptr_array_add(
            pane_working_directories,
            normalized_working_directory(cwd)
        );
    }

    gc_session_store_add_tab_layout(
        loaded,
        profile_id,
        layout,
        pane_working_directories,
        (guint) active_pane
    );
    g_ptr_array_unref(pane_working_directories);
    return TRUE;
}

static gboolean
session_store_load_file(
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

    if (version != 1 && version != GC_SESSION_STORE_VERSION) {
        g_set_error(
            error,
            session_store_error_quark(),
            GC_SESSION_STORE_ERROR_UNSUPPORTED_VERSION,
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
        gboolean loaded_tab;

        if (version == 1) {
            loaded_tab = load_version_one_tab(
                key_file,
                group,
                &loaded
            );
        } else {
            loaded_tab = load_version_two_tab(
                key_file,
                i,
                group,
                &loaded,
                &local_error
            );
        }

        if (!loaded_tab) {
            g_propagate_error(error, local_error);
            gc_session_store_clear(&loaded);
            g_key_file_unref(key_file);
            return FALSE;
        }
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
gc_session_store_load(
    const char *path,
    GcSessionStore *state,
    GError **error
)
{
    g_autofree char *backup_path = NULL;
    GError *primary_error = NULL;
    GError *backup_error = NULL;

    if (path == NULL) {
        gc_session_store_clear(state);
        gc_session_store_init(state);
        return TRUE;
    }

    backup_path = session_store_backup_path(path);

    if (g_file_test(path, G_FILE_TEST_EXISTS) &&
        session_store_load_file(path, state, &primary_error)) {
        return TRUE;
    }

    if (primary_error != NULL &&
        g_error_matches(
            primary_error,
            session_store_error_quark(),
            GC_SESSION_STORE_ERROR_UNSUPPORTED_VERSION
        )) {
        g_propagate_error(error, primary_error);
        return FALSE;
    }

    if (g_file_test(backup_path, G_FILE_TEST_EXISTS) &&
        session_store_load_file(backup_path, state, &backup_error)) {
        g_clear_error(&primary_error);
        return TRUE;
    }

    if (primary_error != NULL) {
        g_propagate_error(error, primary_error);
        g_clear_error(&backup_error);
        return FALSE;
    }

    if (backup_error != NULL) {
        g_propagate_error(error, backup_error);
        return FALSE;
    }

    gc_session_store_clear(state);
    gc_session_store_init(state);
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
    g_autofree char *backup_path = NULL;
    gboolean had_primary = FALSE;
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

    if (state->tabs->len > GC_SESSION_STORE_MAX_TABS) {
        g_set_error(
            error,
            G_FILE_ERROR,
            G_FILE_ERROR_INVAL,
            "Session state contains too many tabs"
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
        GcWorkspaceLayoutNode *parsed;
        GError *layout_error = NULL;
        guint pane_count;

        if (tab->pane_working_directories == NULL) {
            g_set_error(
                error,
                G_FILE_ERROR,
                G_FILE_ERROR_INVAL,
                "Session tab has no pane state"
            );
            g_key_file_unref(key_file);
            return FALSE;
        }

        pane_count = tab->pane_working_directories->len;
        if (pane_count == 0 ||
            pane_count > GC_WORKSPACE_LAYOUT_MAX_PANES ||
            tab->active_pane >= pane_count) {
            g_set_error(
                error,
                G_FILE_ERROR,
                G_FILE_ERROR_INVAL,
                "Session tab pane state is invalid"
            );
            g_key_file_unref(key_file);
            return FALSE;
        }

        parsed = gc_workspace_layout_parse(
            tab->layout,
            pane_count,
            &layout_error
        );
        if (parsed == NULL) {
            g_propagate_prefixed_error(
                error,
                layout_error,
                "Session tab layout is invalid: "
            );
            g_key_file_unref(key_file);
            return FALSE;
        }
        gc_workspace_layout_free(parsed);

        g_key_file_set_string(
            key_file,
            group,
            "profile",
            tab->profile_id != NULL ? tab->profile_id : "default"
        );
        g_key_file_set_string(
            key_file,
            group,
            "layout",
            tab->layout
        );
        g_key_file_set_integer(
            key_file,
            group,
            "pane-count",
            (gint) pane_count
        );
        g_key_file_set_integer(
            key_file,
            group,
            "active-pane",
            (gint) tab->active_pane
        );
        g_key_file_set_string(
            key_file,
            group,
            "working-directory",
            tab->working_directory != NULL
                ? tab->working_directory
                : g_get_home_dir()
        );

        for (guint pane = 0; pane < pane_count; pane++) {
            g_autofree char *pane_group = g_strdup_printf(
                "tab %u pane %u",
                i,
                pane
            );
            const char *cwd = g_ptr_array_index(
                tab->pane_working_directories,
                pane
            );

            g_key_file_set_string(
                key_file,
                pane_group,
                "working-directory",
                cwd != NULL ? cwd : g_get_home_dir()
            );
        }
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
    backup_path = session_store_backup_path(path);
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

    had_primary = g_file_test(path, G_FILE_TEST_EXISTS);
    if (had_primary) {
        if (g_unlink(backup_path) != 0 && errno != ENOENT) {
            saved_errno = errno;
            g_unlink(temporary_path);
            g_set_error(
                error,
                G_FILE_ERROR,
                g_file_error_from_errno(saved_errno),
                "Unable to clear previous session backup: %s",
                g_strerror(saved_errno)
            );
            return FALSE;
        }

        if (g_rename(path, backup_path) != 0) {
            saved_errno = errno;
            g_unlink(temporary_path);
            g_set_error(
                error,
                G_FILE_ERROR,
                g_file_error_from_errno(saved_errno),
                "Unable to preserve previous session state: %s",
                g_strerror(saved_errno)
            );
            return FALSE;
        }
    }

    if (g_rename(temporary_path, path) != 0) {
        saved_errno = errno;
        g_unlink(temporary_path);

        if (had_primary) {
            if (g_rename(backup_path, path) != 0) {
                g_warning(
                    "Unable to restore previous session state after replacement failure: %s",
                    g_strerror(errno)
                );
            }
        }

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
