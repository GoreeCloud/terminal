#include "gc-onboarding-state.h"

#include <errno.h>
#include <glib/gstdio.h>

void
gc_onboarding_state_init(GcOnboardingState *state)
{
    state->completed = FALSE;
    state->step = 0;
}

gboolean
gc_onboarding_state_load(
    const char *path,
    GcOnboardingState *state,
    GError **error
)
{
    GKeyFile *key_file;
    gint step;

    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(state != NULL, FALSE);

    gc_onboarding_state_init(state);

    if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
        return TRUE;
    }

    key_file = g_key_file_new();
    if (!g_key_file_load_from_file(key_file, path, G_KEY_FILE_NONE, error)) {
        g_key_file_unref(key_file);
        return FALSE;
    }

    if (g_key_file_has_key(key_file, "onboarding", "completed", NULL)) {
        state->completed = g_key_file_get_boolean(
            key_file,
            "onboarding",
            "completed",
            NULL
        );
    }

    if (g_key_file_has_key(key_file, "onboarding", "step", NULL)) {
        step = g_key_file_get_integer(key_file, "onboarding", "step", NULL);
        if (step >= 0) {
            state->step = (guint) step;
        }
    }

    g_key_file_unref(key_file);
    return TRUE;
}

gboolean
gc_onboarding_state_save(
    const char *path,
    const GcOnboardingState *state,
    GError **error
)
{
    GKeyFile *key_file;
    g_autofree char *directory = NULL;
    g_autofree char *data = NULL;
    gsize length = 0;

    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(state != NULL, FALSE);

    directory = g_path_get_dirname(path);
    if (g_mkdir_with_parents(directory, 0700) != 0) {
        g_set_error(
            error,
            G_FILE_ERROR,
            g_file_error_from_errno(errno),
            "Unable to create onboarding state directory: %s",
            g_strerror(errno)
        );
        return FALSE;
    }

    key_file = g_key_file_new();
    g_key_file_set_boolean(key_file, "onboarding", "completed", state->completed);
    g_key_file_set_integer(key_file, "onboarding", "step", (gint) state->step);

    data = g_key_file_to_data(key_file, &length, NULL);
    g_key_file_unref(key_file);

    return g_file_set_contents(path, data, (gssize) length, error);
}

char *
gc_onboarding_state_default_path(void)
{
    return g_build_filename(
        g_get_user_config_dir(),
        "goreecloud-terminal",
        "state.ini",
        NULL
    );
}
