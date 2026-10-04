#pragma once

#include <glib.h>

G_BEGIN_DECLS

#define GC_WORKSPACE_LAYOUT_MAX_PANES 16
#define GC_WORKSPACE_LAYOUT_MAX_DEPTH 8

typedef enum {
    GC_WORKSPACE_LAYOUT_HORIZONTAL,
    GC_WORKSPACE_LAYOUT_VERTICAL,
} GcWorkspaceLayoutOrientation;

typedef enum {
    GC_WORKSPACE_LAYOUT_LEAF,
    GC_WORKSPACE_LAYOUT_SPLIT,
} GcWorkspaceLayoutKind;

typedef struct _GcWorkspaceLayoutNode GcWorkspaceLayoutNode;

struct _GcWorkspaceLayoutNode {
    GcWorkspaceLayoutKind kind;
    union {
        guint pane_index;
        struct {
            GcWorkspaceLayoutOrientation orientation;
            GcWorkspaceLayoutNode *start;
            GcWorkspaceLayoutNode *end;
        } split;
    } value;
};

GcWorkspaceLayoutNode *gc_workspace_layout_leaf(guint pane_index);
GcWorkspaceLayoutNode *gc_workspace_layout_split(
    GcWorkspaceLayoutOrientation orientation,
    GcWorkspaceLayoutNode *start,
    GcWorkspaceLayoutNode *end
);
void gc_workspace_layout_free(GcWorkspaceLayoutNode *node);
GcWorkspaceLayoutNode *gc_workspace_layout_parse(
    const char *text,
    guint pane_count,
    GError **error
);
char *gc_workspace_layout_serialize(const GcWorkspaceLayoutNode *node);

G_END_DECLS
