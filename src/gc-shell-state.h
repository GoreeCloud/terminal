#pragma once

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
    GC_SHELL_PHASE_UNKNOWN,
    GC_SHELL_PHASE_PROMPT,
    GC_SHELL_PHASE_RUNNING,
    GC_SHELL_PHASE_COMPLETE,
} GcShellPhase;

typedef struct {
    GcShellPhase phase;
    gboolean has_exit_status;
    guint64 last_exit_status;
} GcShellState;

void gc_shell_state_init(GcShellState *state);
void gc_shell_state_mark_prompt(GcShellState *state);
void gc_shell_state_mark_preexec(GcShellState *state);
void gc_shell_state_mark_postexec(
    GcShellState *state,
    gboolean has_exit_status,
    guint64 exit_status
);
gboolean gc_shell_state_is_integrated(const GcShellState *state);
gboolean gc_shell_state_is_running(const GcShellState *state);
gboolean gc_shell_state_get_last_exit_status(
    const GcShellState *state,
    guint64 *exit_status
);
char *gc_shell_state_dup_status(const GcShellState *state);

G_END_DECLS
