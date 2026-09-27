#pragma once

#include <gio/gio.h>

G_BEGIN_DECLS

typedef enum {
    GC_LINK_TARGET_URI,
    GC_LINK_TARGET_PATH,
} GcLinkTargetKind;

gboolean gc_link_uri_scheme_allowed(const char *scheme);

char *gc_link_target_to_uri(
    GcLinkTargetKind kind,
    const char *target,
    const char *working_directory,
    GError **error
);

G_END_DECLS
