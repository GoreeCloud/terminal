#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GcTerminalSession GcTerminalSession;

typedef void (*GcTerminalSessionChangedFunc)(
    GcTerminalSession *session,
    gpointer user_data
);

typedef void (*GcTerminalSessionPasteRequestedFunc)(
    GcTerminalSession *session,
    gpointer user_data
);

GcTerminalSession *gc_terminal_session_new(
    const char *working_directory,
    GcTerminalSessionChangedFunc changed,
    GcTerminalSessionPasteRequestedFunc paste_requested,
    gpointer user_data
);

GtkWidget *gc_terminal_session_get_widget(GcTerminalSession *session);
const char *gc_terminal_session_get_status(GcTerminalSession *session);
char *gc_terminal_session_dup_working_directory(GcTerminalSession *session);
char *gc_terminal_session_dup_display_title(GcTerminalSession *session);
gboolean gc_terminal_session_has_focus(GcTerminalSession *session);

void gc_terminal_session_copy(GcTerminalSession *session);
void gc_terminal_session_paste(GcTerminalSession *session);
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
