#include "gc-command-boundaries.h"

static void
ensure_rows(GcCommandBoundaries *boundaries)
{
    if (boundaries->rows == NULL) {
        boundaries->rows = g_array_new(FALSE, FALSE, sizeof(gint64));
    }
}

void
gc_command_boundaries_init(GcCommandBoundaries *boundaries)
{
    g_return_if_fail(boundaries != NULL);

    boundaries->rows = g_array_new(FALSE, FALSE, sizeof(gint64));
}

void
gc_command_boundaries_clear(GcCommandBoundaries *boundaries)
{
    if (boundaries == NULL) {
        return;
    }

    g_clear_pointer(&boundaries->rows, g_array_unref);
}

void
gc_command_boundaries_mark_start(
    GcCommandBoundaries *boundaries,
    gint64 row
)
{
    gint64 last_row;

    g_return_if_fail(boundaries != NULL);

    if (row < 0) {
        return;
    }

    ensure_rows(boundaries);

    if (boundaries->rows->len > 0) {
        last_row = g_array_index(
            boundaries->rows,
            gint64,
            boundaries->rows->len - 1
        );

        if (last_row == row) {
            return;
        }
    }

    g_array_append_val(boundaries->rows, row);

    if (boundaries->rows->len > GC_COMMAND_BOUNDARIES_MAX) {
        g_array_remove_range(
            boundaries->rows,
            0,
            boundaries->rows->len - GC_COMMAND_BOUNDARIES_MAX
        );
    }
}

void
gc_command_boundaries_prune_before(
    GcCommandBoundaries *boundaries,
    gint64 minimum_row
)
{
    guint remove_count = 0;

    g_return_if_fail(boundaries != NULL);

    if (boundaries->rows == NULL) {
        return;
    }

    while (remove_count < boundaries->rows->len &&
           g_array_index(boundaries->rows, gint64, remove_count) <
               minimum_row) {
        remove_count++;
    }

    if (remove_count > 0) {
        g_array_remove_range(boundaries->rows, 0, remove_count);
    }
}

guint
gc_command_boundaries_get_count(const GcCommandBoundaries *boundaries)
{
    return boundaries != NULL && boundaries->rows != NULL
        ? boundaries->rows->len
        : 0;
}

gboolean
gc_command_boundaries_previous(
    const GcCommandBoundaries *boundaries,
    gint64 from_row,
    gint64 *target_row
)
{
    if (boundaries == NULL || boundaries->rows == NULL) {
        return FALSE;
    }

    for (guint i = boundaries->rows->len; i > 0; i--) {
        gint64 row = g_array_index(boundaries->rows, gint64, i - 1);

        if (row < from_row) {
            if (target_row != NULL) {
                *target_row = row;
            }
            return TRUE;
        }
    }

    return FALSE;
}

gboolean
gc_command_boundaries_next(
    const GcCommandBoundaries *boundaries,
    gint64 from_row,
    gint64 *target_row
)
{
    if (boundaries == NULL || boundaries->rows == NULL) {
        return FALSE;
    }

    for (guint i = 0; i < boundaries->rows->len; i++) {
        gint64 row = g_array_index(boundaries->rows, gint64, i);

        if (row > from_row) {
            if (target_row != NULL) {
                *target_row = row;
            }
            return TRUE;
        }
    }

    return FALSE;
}
