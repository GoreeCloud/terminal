#pragma once

#include <glib.h>

G_BEGIN_DECLS

gboolean gc_command_notification_due(
    guint threshold_seconds,
    gint64 started_at_usec,
    gint64 finished_at_usec,
    guint64 *elapsed_seconds
);

char *gc_command_notification_format_duration(guint64 elapsed_seconds);

G_END_DECLS
