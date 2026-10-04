#pragma once

#include <glib.h>

G_BEGIN_DECLS

#define GC_COMMAND_BOUNDARIES_MAX 512

typedef struct {
    GArray *rows;
} GcCommandBoundaries;

void gc_command_boundaries_init(GcCommandBoundaries *boundaries);
void gc_command_boundaries_clear(GcCommandBoundaries *boundaries);
void gc_command_boundaries_mark_start(
    GcCommandBoundaries *boundaries,
    gint64 row
);
void gc_command_boundaries_prune_before(
    GcCommandBoundaries *boundaries,
    gint64 minimum_row
);
guint gc_command_boundaries_get_count(
    const GcCommandBoundaries *boundaries
);
gboolean gc_command_boundaries_previous(
    const GcCommandBoundaries *boundaries,
    gint64 from_row,
    gint64 *target_row
);
gboolean gc_command_boundaries_next(
    const GcCommandBoundaries *boundaries,
    gint64 from_row,
    gint64 *target_row
);

G_END_DECLS
