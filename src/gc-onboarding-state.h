#pragma once

#include <glib.h>

G_BEGIN_DECLS

typedef struct {
    gboolean completed;
    guint step;
} GcOnboardingState;

void gc_onboarding_state_init(GcOnboardingState *state);
gboolean gc_onboarding_state_load(
    const char *path,
    GcOnboardingState *state,
    GError **error
);
gboolean gc_onboarding_state_save(
    const char *path,
    const GcOnboardingState *state,
    GError **error
);
char *gc_onboarding_state_default_path(void);

G_END_DECLS
