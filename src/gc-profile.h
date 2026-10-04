#pragma once

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GcProfile GcProfile;
typedef struct _GcProfileStore GcProfileStore;

typedef enum {
    GC_PROFILE_CURSOR_SHAPE_BLOCK,
    GC_PROFILE_CURSOR_SHAPE_IBEAM,
    GC_PROFILE_CURSOR_SHAPE_UNDERLINE,
} GcProfileCursorShape;

typedef enum {
    GC_PROFILE_CURSOR_BLINK_SYSTEM,
    GC_PROFILE_CURSOR_BLINK_ON,
    GC_PROFILE_CURSOR_BLINK_OFF,
} GcProfileCursorBlink;

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
const char *gc_profile_get_startup_command(const GcProfile *profile);
const char *gc_profile_get_font(const GcProfile *profile);
const char *gc_profile_get_foreground(const GcProfile *profile);
const char *gc_profile_get_background(const GcProfile *profile);
GcProfileCursorShape gc_profile_get_cursor_shape(const GcProfile *profile);
GcProfileCursorBlink gc_profile_get_cursor_blink(const GcProfile *profile);
gboolean gc_profile_get_bold_is_bright(const GcProfile *profile);
guint gc_profile_get_notify_after_seconds(const GcProfile *profile);
gint64 gc_profile_get_scrollback_lines(const GcProfile *profile);
char **gc_profile_dup_environment(const GcProfile *profile);
char **gc_profile_dup_keybindings(const GcProfile *profile);
char **gc_profile_dup_spawn_environment(const GcProfile *profile);

char *gc_profile_dup_effective_working_directory(
    const GcProfile *profile,
    const char *fallback
);
char *gc_profile_default_path(void);

G_END_DECLS
