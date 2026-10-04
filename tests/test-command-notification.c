#include "gc-command-notification.h"

#include <glib.h>

static void
test_due_threshold(void)
{
    guint64 elapsed = 0;
    gint64 started = 10 * G_USEC_PER_SEC;

    g_assert_false(
        gc_command_notification_due(
            30,
            started,
            started + 29 * G_USEC_PER_SEC,
            &elapsed
        )
    );
    g_assert_cmpuint(elapsed, ==, 0);

    g_assert_true(
        gc_command_notification_due(
            30,
            started,
            started + 30 * G_USEC_PER_SEC,
            &elapsed
        )
    );
    g_assert_cmpuint(elapsed, ==, 30);

    g_assert_true(
        gc_command_notification_due(
            30,
            started,
            started + 91 * G_USEC_PER_SEC,
            &elapsed
        )
    );
    g_assert_cmpuint(elapsed, ==, 91);
}

static void
test_disabled_and_invalid(void)
{
    guint64 elapsed = 999;

    g_assert_false(
        gc_command_notification_due(
            0,
            G_USEC_PER_SEC,
            20 * G_USEC_PER_SEC,
            &elapsed
        )
    );
    g_assert_false(
        gc_command_notification_due(
            10,
            0,
            20 * G_USEC_PER_SEC,
            &elapsed
        )
    );
    g_assert_false(
        gc_command_notification_due(
            10,
            20 * G_USEC_PER_SEC,
            10 * G_USEC_PER_SEC,
            &elapsed
        )
    );
    g_assert_cmpuint(elapsed, ==, 999);
}

static void
test_format_duration(void)
{
    g_autofree char *seconds =
        gc_command_notification_format_duration(9);
    g_autofree char *minutes =
        gc_command_notification_format_duration(125);
    g_autofree char *hours =
        gc_command_notification_format_duration(7384);

    g_assert_cmpstr(seconds, ==, "9s");
    g_assert_cmpstr(minutes, ==, "2m 5s");
    g_assert_cmpstr(hours, ==, "2h 3m 4s");
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func(
        "/command-notification/due-threshold",
        test_due_threshold
    );
    g_test_add_func(
        "/command-notification/disabled-invalid",
        test_disabled_and_invalid
    );
    g_test_add_func(
        "/command-notification/format-duration",
        test_format_duration
    );
    return g_test_run();
}
