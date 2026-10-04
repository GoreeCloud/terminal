#pragma once

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GcProfile GcProfile;
typedef struct _GcProfileStore GcProfileStore;

GcProfileStore *gc_profile_store_new(void);
void gc_profile_store_free(GcProfileStore *store);

gboolean gc_profile_store_load(
    GcProfileStore *store,
    const char *path,
    GError **error
);
guint gc_profile_store_get_count(const GcProfileStore *store);
const GcProfile *gc_profile_store_get(
    const GcProfileStore *store,
    guint index
);
const GcProfile *gc_profile_store_lookup(
    const GcProfileStore *store,
    const char *id
);
const GcProfile *gc_profile_store_get_default(const GcProfileStore *store);

GcProfile *gc_profile_copy(const GcProfile *profile);
void gc_profile_free(GcProfile *profile);

const char *gc_profile_get_id(const GcProfile *profile);
const char *gc_profile_get_name(const GcProfile *profile);
const char *gc_profile_get_shell(const GcProfile *profile);
const char *gc_profile_get_working_directory(const GcProfile *profile);
const char *gc_profile_get_font(const GcProfile *profile);
const char *gc_profile_get_foreground(const GcProfile *profile);
const char *gc_profile_get_background(const GcProfile *profile);
char **gc_profile_dup_environment(const GcProfile *profile);

char *gc_profile_dup_effective_working_directory(
    const GcProfile *profile,
    const char *fallback
);
char *gc_profile_default_path(void);

G_END_DECLS
