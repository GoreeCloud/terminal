#pragma once

#include <glib.h>

G_BEGIN_DECLS

#define GC_SESSION_STORE_VERSION 1

typedef struct {
    char *profile_id;
    char *working_directory;
} GcSessionStoreTab;

typedef struct {
    guint version;
    guint current_tab;
    GPtrArray *tabs;
} GcSessionStore;

void gc_session_store_init(GcSessionStore *state);
void gc_session_store_clear(GcSessionStore *state);

void gc_session_store_add_tab(
    GcSessionStore *state,
    const char *profile_id,
    const char *working_directory
);

gboolean gc_session_store_load(
    const char *path,
    GcSessionStore *state,
    GError **error
);
gboolean gc_session_store_save(
    const char *path,
    const GcSessionStore *state,
    GError **error
);

char *gc_session_store_default_path(void);

G_END_DECLS
