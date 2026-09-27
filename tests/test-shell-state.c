#include "gc-shell-state.h"

#include <glib.h>

static void
test_initial_state(void)
{
    GcShellState state;
    g_autofree char *status = NULL;

    gc_shell_state_init(&state);

    g_assert_false(gc_shell_state_is_integrated(&state));
    g_assert_false(gc_shell_state_is_running(&state));
    g_assert_false(gc_shell_state_get_last_exit_status(&state, NULL));

    status = gc_shell_state_dup_status(&state);
    g_assert_cmpstr(status, ==, "Session active");
}

static void
test_command_lifecycle(void)
{
    GcShellState state;
    guint64 exit_status = 0;
    g_autofree char *prompt_status = NULL;
    g_autofree char *running_status = NULL;
    g_autofree char *complete_status = NULL;
    g_autofree char *next_prompt_status = NULL;

    gc_shell_state_init(&state);

    gc_shell_state_mark_prompt(&state);
    g_assert_true(gc_shell_state_is_integrated(&state));
    prompt_status = gc_shell_state_dup_status(&state);
    g_assert_cmpstr(prompt_status, ==, "Shell ready");

    gc_shell_state_mark_preexec(&state);
    g_assert_true(gc_shell_state_is_running(&state));
    running_status = gc_shell_state_dup_status(&state);
    g_assert_cmpstr(running_status, ==, "Command running");

    gc_shell_state_mark_postexec(&state, TRUE, 7);
    g_assert_false(gc_shell_state_is_running(&state));
    g_assert_true(gc_shell_state_get_last_exit_status(&state, &exit_status));
    g_assert_cmpuint(exit_status, ==, 7);
    complete_status = gc_shell_state_dup_status(&state);
    g_assert_cmpstr(complete_status, ==, "Command exited 7");

    gc_shell_state_mark_prompt(&state);
    next_prompt_status = gc_shell_state_dup_status(&state);
    g_assert_cmpstr(next_prompt_status, ==, "Shell ready · last exit 7");
}

static void
test_completion_without_status(void)
{
    GcShellState state;
    g_autofree char *status = NULL;

    gc_shell_state_init(&state);
    gc_shell_state_mark_preexec(&state);
    gc_shell_state_mark_postexec(&state, FALSE, 99);

    g_assert_false(gc_shell_state_get_last_exit_status(&state, NULL));
    status = gc_shell_state_dup_status(&state);
    g_assert_cmpstr(status, ==, "Command completed");
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/shell-state/initial", test_initial_state);
    g_test_add_func("/shell-state/lifecycle", test_command_lifecycle);
    g_test_add_func(
        "/shell-state/completion-without-status",
        test_completion_without_status
    );
    return g_test_run();
}
