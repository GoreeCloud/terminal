#include "gc-command-notification.h"

gboolean
gc_command_notification_due(
    guint threshold_seconds,
    gint64 started_at_usec,
    gint64 finished_at_usec,
    guint64 *elapsed_seconds
)
{
    guint64 elapsed;

    if (threshold_seconds == 0 ||
        started_at_usec <= 0 ||
        finished_at_usec < started_at_usec) {
        return FALSE;
    }

    elapsed = (guint64) (
        (finished_at_usec - started_at_usec) / G_USEC_PER_SEC
    );

    if (elapsed < threshold_seconds) {
        return FALSE;
    }

    if (elapsed_seconds != NULL) {
        *elapsed_seconds = elapsed;
    }

    return TRUE;
}

char *
gc_command_notification_format_duration(guint64 elapsed_seconds)
{
    guint64 hours = elapsed_seconds / 3600;
    guint64 minutes = (elapsed_seconds % 3600) / 60;
    guint64 seconds = elapsed_seconds % 60;

    if (hours > 0) {
        return g_strdup_printf(
            "%" G_GUINT64_FORMAT "h %" G_GUINT64_FORMAT "m %" G_GUINT64_FORMAT "s",
            hours,
            minutes,
            seconds
        );
    }

    if (minutes > 0) {
        return g_strdup_printf(
            "%" G_GUINT64_FORMAT "m %" G_GUINT64_FORMAT "s",
            minutes,
            seconds
        );
    }

    return g_strdup_printf(
        "%" G_GUINT64_FORMAT "s",
        seconds
    );
}
