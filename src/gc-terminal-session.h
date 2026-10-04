#pragma once

#include <gtk/gtk.h>

#include "gc-link-utils.h"
#include "gc-profile.h"

G_BEGIN_DECLS

typedef struct _GcTerminalSession GcTerminalSession;

typedef void (*GcTerminalSessionChangedFunc)(
    GcTerminalSession *session,
    gpointer user_data
);

typedef enum {
    GC_TERMINAL_PASTE_CLIPBOARD,
    GC_TERMINAL_PASTE_PRIMARY,
} GcTerminalPasteSource;

typedef void (*GcTerminalSessionPasteRequestedFunc)(
    GcTerminalSession *session,
    GcTerminalPasteSource source,
    gpointer user_data
);

typedef void (*GcTerminalSessionOpenRequestedFunc)(
    GcTerminalSession *session,
    GcLinkTargetKind kind,
    const char *target,
    gpointer user_data
);

GcTerminalSession *gc_terminal_session_new(
    const char *working_directory,
    GcTerminalSessionChangedFunc changed,
    GcTerminalSessionPasteRequestedFunc paste_requested,
    GcTerminalSessionOpenRequestedFunc open_requested,
    gpointer user_data
);

GcTerminalSession *gc_terminal_session_new_with_profile(
    const GcProfile *profile,
    const char *working_directory,
    GcTerminalSessionChangedFunc changed,
    GcTerminalSessionPasteRequestedFunc paste_requested,
    GcTerminalSessionOpenRequestedFunc open_requested,
    gpointer user_data
);

GtkWidget *gc_terminal_session_get_widget(GcTerminalSession *session);
const char *gc_terminal_session_get_status(GcTerminalSession *session);
char *gc_terminal_session_dup_working_directory(GcTerminalSession *session);
char *gc_terminal_session_dup_display_title(GcTerminalSession *session);
const char *gc_terminal_session_get_profile_id(GcTerminalSession *session);
gboolean gc_terminal_session_has_focus(GcTerminalSession *session);
gboolean gc_terminal_session_has_shell_integration(GcTerminalSession *session);
gboolean gc_terminal_session_is_command_running(GcTerminalSession *session);
gboolean gc_terminal_session_get_last_exit_status(
    GcTerminalSession *session,
    guint64 *exit_status
);

void gc_terminal_session_copy(GcTerminalSession *session);
void gc_terminal_session_paste(GcTerminalSession *session);
void gc_terminal_session_request_paste(
    GcTerminalSession *session,
    GcTerminalPasteSource source
);
void gc_terminal_session_paste_text(
    GcTerminalSession *session,
    const char *text
);
void gc_terminal_session_focus(GcTerminalSession *session);

gboolean gc_terminal_session_set_search(
    GcTerminalSession *session,
    const char *pattern,
    gboolean regex_enabled,
    gboolean case_sensitive,
    GError **error
);
void gc_terminal_session_clear_search(GcTerminalSession *session);
gboolean gc_terminal_session_search_next(GcTerminalSession *session);
gboolean gc_terminal_session_search_previous(GcTerminalSession *session);

G_END_DECLS
