#include "gc-shell-state.h"

void
gc_shell_state_init(GcShellState *state)
{
    g_return_if_fail(state != NULL);

    state->phase = GC_SHELL_PHASE_UNKNOWN;
    state->has_exit_status = FALSE;
    state->last_exit_status = 0;
}

void
gc_shell_state_mark_prompt(GcShellState *state)
{
    g_return_if_fail(state != NULL);

    state->phase = GC_SHELL_PHASE_PROMPT;
}

void
gc_shell_state_mark_preexec(GcShellState *state)
{
    g_return_if_fail(state != NULL);

    state->phase = GC_SHELL_PHASE_RUNNING;
}

void
gc_shell_state_mark_postexec(
    GcShellState *state,
    gboolean has_exit_status,
    guint64 exit_status
)
{
    g_return_if_fail(state != NULL);

    state->phase = GC_SHELL_PHASE_COMPLETE;
    state->has_exit_status = has_exit_status;
    state->last_exit_status = has_exit_status ? exit_status : 0;
}

gboolean
gc_shell_state_is_integrated(const GcShellState *state)
{
    g_return_val_if_fail(state != NULL, FALSE);

    return state->phase != GC_SHELL_PHASE_UNKNOWN;
}

gboolean
gc_shell_state_is_running(const GcShellState *state)
{
    g_return_val_if_fail(state != NULL, FALSE);

    return state->phase == GC_SHELL_PHASE_RUNNING;
}

gboolean
gc_shell_state_get_last_exit_status(
    const GcShellState *state,
    guint64 *exit_status
)
{
    g_return_val_if_fail(state != NULL, FALSE);

    if (!state->has_exit_status) {
        return FALSE;
    }

    if (exit_status != NULL) {
        *exit_status = state->last_exit_status;
    }

    return TRUE;
}

char *
gc_shell_state_dup_status(const GcShellState *state)
{
    g_return_val_if_fail(state != NULL, g_strdup("Session active"));

    switch (state->phase) {
    case GC_SHELL_PHASE_PROMPT:
        if (state->has_exit_status) {
            return g_strdup_printf(
                "Shell ready · last exit %" G_GUINT64_FORMAT,
                state->last_exit_status
            );
        }
        return g_strdup("Shell ready");
    case GC_SHELL_PHASE_RUNNING:
        return g_strdup("Command running");
    case GC_SHELL_PHASE_COMPLETE:
        if (state->has_exit_status) {
            return g_strdup_printf(
                "Command exited %" G_GUINT64_FORMAT,
                state->last_exit_status
            );
        }
        return g_strdup("Command completed");
    case GC_SHELL_PHASE_UNKNOWN:
    default:
        return g_strdup("Session active");
    }
}
