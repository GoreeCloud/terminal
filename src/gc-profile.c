#include "gc-profile.h"

#include <gio/gio.h>
#include <string.h>

struct _GcProfile {
    char *id;
    char *name;
    char *shell;
    char *working_directory;
    char *font;
    char *foreground;
    char *background;
    char **environment;
};

struct _GcProfileStore {
    GPtrArray *profiles;
};

static GQuark
profile_error_quark(void)
{
    return g_quark_from_static_string("goreecloud-terminal-profile-error");
}

static GcProfile *
profile_new_default(void)
{
    GcProfile *profile = g_new0(GcProfile, 1);

    profile->id = g_strdup("default");
    profile->name = g_strdup("Default");
    profile->font = g_strdup("Monospace 11");
    profile->foreground = g_strdup("#dceaff");
    profile->background = g_strdup("#050d18");

    return profile;
}

static gboolean
profile_id_is_valid(const char *id)
{
    if (id == NULL || *id == '\0' || strlen(id) > 64) {
        return FALSE;
    }

    for (const unsigned char *p = (const unsigned char *) id; *p != '\0'; p++) {
        if (!(g_ascii_isalnum(*p) || *p == '-' || *p == '_' || *p == '.')) {
            return FALSE;
        }
    }

    return TRUE;
}

static gboolean
environment_entry_is_valid(const char *entry)
{
    const char *equals;
    gsize name_length;

    if (entry == NULL) {
        return FALSE;
    }

    equals = strchr(entry, '=');
    if (equals == NULL || equals == entry) {
        return FALSE;
    }

    name_length = (gsize) (equals - entry);
    if (!(g_ascii_isalpha(entry[0]) || entry[0] == '_')) {
        return FALSE;
    }

    for (gsize i = 1; i < name_length; i++) {
        if (!(g_ascii_isalnum(entry[i]) || entry[i] == '_')) {
            return FALSE;
        }
    }

    return TRUE;
}

static gboolean
color_is_valid(const char *value)
{
    gsize length;

    if (value == NULL) {
        return TRUE;
    }

    length = strlen(value);
    if (length != 7 && length != 9) {
        return FALSE;
    }

    if (value[0] != '#') {
        return FALSE;
    }

    for (gsize i = 1; i < length; i++) {
        if (!g_ascii_isxdigit(value[i])) {
            return FALSE;
        }
    }

    return TRUE;
}

static char *
optional_key_string(GKeyFile *key_file, const char *group, const char *key)
{
    GError *error = NULL;
    char *value = g_key_file_get_string(key_file, group, key, &error);

    if (error != NULL) {
        g_clear_error(&error);
        return NULL;
    }

    if (value != NULL) {
        g_strstrip(value);
        if (*value == '\0') {
            g_clear_pointer(&value, g_free);
        }
    }

    return value;
}

static gboolean
profile_store_has_id(GPtrArray *profiles, const char *id)
{
    for (guint i = 0; i < profiles->len; i++) {
        GcProfile *profile = g_ptr_array_index(profiles, i);

        if (g_strcmp0(profile->id, id) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean
load_profile_group(
    GKeyFile *key_file,
    const char *group,
    GPtrArray *profiles,
    GError **error
)
{
    const char *prefix = "profile ";
    const char *id = group + strlen(prefix);
    GcProfile *profile = NULL;

    if (!profile_id_is_valid(id) || g_strcmp0(id, "default") == 0) {
        g_set_error(
            error,
            profile_error_quark(),
            1,
            "Invalid or reserved profile id in group [%s]",
            group
        );
        return FALSE;
    }

    if (profile_store_has_id(profiles, id)) {
        g_set_error(
            error,
            profile_error_quark(),
            2,
            "Duplicate profile id: %s",
            id
        );
        return FALSE;
    }

    profile = g_new0(GcProfile, 1);
    profile->id = g_strdup(id);
    profile->name = optional_key_string(key_file, group, "name");
    profile->shell = optional_key_string(key_file, group, "shell");
    profile->working_directory = optional_key_string(
        key_file,
        group,
        "working-directory"
    );
    profile->font = optional_key_string(key_file, group, "font");
    profile->foreground = optional_key_string(key_file, group, "foreground");
    profile->background = optional_key_string(key_file, group, "background");
    profile->environment = g_key_file_get_string_list(
        key_file,
        group,
        "environment",
        NULL,
        NULL
    );

    if (profile->name == NULL) {
        profile->name = g_strdup(id);
    }

    if (profile->shell != NULL && !g_path_is_absolute(profile->shell)) {
        g_set_error(
            error,
            profile_error_quark(),
            3,
            "Profile %s shell must be an absolute path",
            id
        );
        gc_profile_free(profile);
        return FALSE;
    }

    if (profile->working_directory != NULL &&
        !g_path_is_absolute(profile->working_directory) &&
        g_strcmp0(profile->working_directory, "~") != 0 &&
        !g_str_has_prefix(profile->working_directory, "~/")) {
        g_set_error(
            error,
            profile_error_quark(),
            4,
            "Profile %s working-directory must be absolute or begin with ~/",
            id
        );
        gc_profile_free(profile);
        return FALSE;
    }

    for (guint i = 0;
         profile->environment != NULL && profile->environment[i] != NULL;
         i++) {
        if (!environment_entry_is_valid(profile->environment[i])) {
            g_set_error(
                error,
                profile_error_quark(),
                5,
                "Profile %s has invalid environment entry: %s",
                id,
                profile->environment[i]
            );
            gc_profile_free(profile);
            return FALSE;
        }
    }

    if (!color_is_valid(profile->foreground) ||
        !color_is_valid(profile->background)) {
        g_set_error(
            error,
            profile_error_quark(),
            6,
            "Profile %s colors must use #RRGGBB or #RRGGBBAA",
            id
        );
        gc_profile_free(profile);
        return FALSE;
    }

    g_ptr_array_add(profiles, profile);
    return TRUE;
}

GcProfileStore *
gc_profile_store_new(void)
{
    GcProfileStore *store = g_new0(GcProfileStore, 1);

    store->profiles = g_ptr_array_new_with_free_func(
        (GDestroyNotify) gc_profile_free
    );
    g_ptr_array_add(store->profiles, profile_new_default());
    return store;
}

void
gc_profile_store_free(GcProfileStore *store)
{
    if (store == NULL) {
        return;
    }

    g_ptr_array_free(store->profiles, TRUE);
    g_free(store);
}

gboolean
gc_profile_store_load(
    GcProfileStore *store,
    const char *path,
    GError **error
)
{
    GPtrArray *loaded = g_ptr_array_new_with_free_func(
        (GDestroyNotify) gc_profile_free
    );
    GKeyFile *key_file = g_key_file_new();
    gsize group_count = 0;
    char **groups = NULL;
    GError *local_error = NULL;

    g_ptr_array_add(loaded, profile_new_default());

    if (path == NULL || !g_file_test(path, G_FILE_TEST_EXISTS)) {
        g_ptr_array_free(store->profiles, TRUE);
        store->profiles = loaded;
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
            "Unable to load terminal profiles: "
        );
        g_ptr_array_free(loaded, TRUE);
        g_key_file_unref(key_file);
        return FALSE;
    }

    groups = g_key_file_get_groups(key_file, &group_count);
    for (gsize i = 0; i < group_count; i++) {
        if (!g_str_has_prefix(groups[i], "profile ")) {
            continue;
        }

        if (!load_profile_group(key_file, groups[i], loaded, error)) {
            g_strfreev(groups);
            g_ptr_array_free(loaded, TRUE);
            g_key_file_unref(key_file);
            return FALSE;
        }
    }

    g_strfreev(groups);
    g_key_file_unref(key_file);
    g_ptr_array_free(store->profiles, TRUE);
    store->profiles = loaded;
    return TRUE;
}

guint
gc_profile_store_get_count(const GcProfileStore *store)
{
    return store != NULL ? store->profiles->len : 0;
}

const GcProfile *
gc_profile_store_get(const GcProfileStore *store, guint index)
{
    if (store == NULL || index >= store->profiles->len) {
        return NULL;
    }

    return g_ptr_array_index(store->profiles, index);
}

const GcProfile *
gc_profile_store_lookup(const GcProfileStore *store, const char *id)
{
    if (store == NULL || id == NULL) {
        return NULL;
    }

    for (guint i = 0; i < store->profiles->len; i++) {
        GcProfile *profile = g_ptr_array_index(store->profiles, i);

        if (g_strcmp0(profile->id, id) == 0) {
            return profile;
        }
    }

    return NULL;
}

const GcProfile *
gc_profile_store_get_default(const GcProfileStore *store)
{
    return gc_profile_store_lookup(store, "default");
}

GcProfile *
gc_profile_copy(const GcProfile *profile)
{
    GcProfile *copy;

    if (profile == NULL) {
        return NULL;
    }

    copy = g_new0(GcProfile, 1);
    copy->id = g_strdup(profile->id);
    copy->name = g_strdup(profile->name);
    copy->shell = g_strdup(profile->shell);
    copy->working_directory = g_strdup(profile->working_directory);
    copy->font = g_strdup(profile->font);
    copy->foreground = g_strdup(profile->foreground);
    copy->background = g_strdup(profile->background);
    copy->environment = g_strdupv(profile->environment);
    return copy;
}

void
gc_profile_free(GcProfile *profile)
{
    if (profile == NULL) {
        return;
    }

    g_free(profile->id);
    g_free(profile->name);
    g_free(profile->shell);
    g_free(profile->working_directory);
    g_free(profile->font);
    g_free(profile->foreground);
    g_free(profile->background);
    g_strfreev(profile->environment);
    g_free(profile);
}

const char *
gc_profile_get_id(const GcProfile *profile)
{
    return profile != NULL ? profile->id : "default";
}

const char *
gc_profile_get_name(const GcProfile *profile)
{
    return profile != NULL ? profile->name : "Default";
}

const char *
gc_profile_get_shell(const GcProfile *profile)
{
    return profile != NULL ? profile->shell : NULL;
}

const char *
gc_profile_get_working_directory(const GcProfile *profile)
{
    return profile != NULL ? profile->working_directory : NULL;
}

const char *
gc_profile_get_font(const GcProfile *profile)
{
    return profile != NULL && profile->font != NULL
        ? profile->font
        : "Monospace 11";
}

const char *
gc_profile_get_foreground(const GcProfile *profile)
{
    return profile != NULL && profile->foreground != NULL
        ? profile->foreground
        : "#dceaff";
}

const char *
gc_profile_get_background(const GcProfile *profile)
{
    return profile != NULL && profile->background != NULL
        ? profile->background
        : "#050d18";
}

char **
gc_profile_dup_environment(const GcProfile *profile)
{
    return profile != NULL ? g_strdupv(profile->environment) : NULL;
}

char **
gc_profile_dup_spawn_environment(const GcProfile *profile)
{
    char **environment = g_get_environ();
    g_auto(GStrv) overrides = gc_profile_dup_environment(profile);

    for (guint i = 0;
         overrides != NULL && overrides[i] != NULL;
         i++) {
        const char *equals = strchr(overrides[i], '=');
        g_autofree char *name = NULL;

        if (equals == NULL) {
            continue;
        }

        name = g_strndup(
            overrides[i],
            (gsize) (equals - overrides[i])
        );
        environment = g_environ_setenv(
            environment,
            name,
            equals + 1,
            TRUE
        );
    }

    return environment;
}

char *
gc_profile_dup_effective_working_directory(
    const GcProfile *profile,
    const char *fallback
)
{
    const char *configured = gc_profile_get_working_directory(profile);
    const char *safe_fallback =
        fallback != NULL && *fallback != '\0'
            ? fallback
            : g_get_home_dir();
    g_autofree char *resolved = NULL;

    if (configured == NULL || *configured == '\0') {
        return g_strdup(safe_fallback);
    }

    if (g_strcmp0(configured, "~") == 0) {
        resolved = g_strdup(g_get_home_dir());
    } else if (g_str_has_prefix(configured, "~/")) {
        resolved = g_build_filename(
            g_get_home_dir(),
            configured + 2,
            NULL
        );
    } else {
        resolved = g_strdup(configured);
    }

    if (!g_file_test(resolved, G_FILE_TEST_IS_DIR)) {
        return g_strdup(safe_fallback);
    }

    return g_steal_pointer(&resolved);
}

char *
gc_profile_default_path(void)
{
    return g_build_filename(
        g_get_user_config_dir(),
        "goreecloud-terminal",
        "profiles.ini",
        NULL
    );
}
