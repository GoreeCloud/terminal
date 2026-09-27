#pragma once

#include <gtk/gtk.h>

#include "gc-terminal-session.h"

G_BEGIN_DECLS

typedef struct _GcWorkspace GcWorkspace;

typedef void (*GcWorkspaceChangedFunc)(
    GcWorkspace *workspace,
    gpointer user_data
);

typedef void (*GcWorkspacePasteRequestedFunc)(
    GcWorkspace *workspace,
    GcTerminalSession *session,
    GcTerminalPasteSource source,
    gpointer user_data
);

typedef void (*GcWorkspaceOpenRequestedFunc)(
    GcWorkspace *workspace,
    GcTerminalSession *session,
    GcLinkTargetKind kind,
    const char *target,
    gpointer user_data
);

GcWorkspace *gc_workspace_new(
    GcWorkspaceChangedFunc changed,
    GcWorkspacePasteRequestedFunc paste_requested,
    GcWorkspaceOpenRequestedFunc open_requested,
    gpointer user_data
);
void gc_workspace_free(GcWorkspace *workspace);

GtkWidget *gc_workspace_get_widget(GcWorkspace *workspace);
GcTerminalSession *gc_workspace_get_current_session(GcWorkspace *workspace);
guint gc_workspace_get_count(GcWorkspace *workspace);
guint gc_workspace_get_current_pane_count(GcWorkspace *workspace);

void gc_workspace_add_tab(GcWorkspace *workspace, const char *working_directory);
gboolean gc_workspace_close_current(GcWorkspace *workspace);
void gc_workspace_select_relative(GcWorkspace *workspace, gint delta);
char *gc_workspace_dup_current_working_directory(GcWorkspace *workspace);

gboolean gc_workspace_split_current(
    GcWorkspace *workspace,
    GtkOrientation orientation
);
gboolean gc_workspace_close_current_pane(GcWorkspace *workspace);
void gc_workspace_focus_relative_pane(GcWorkspace *workspace, gint delta);

G_END_DECLS
