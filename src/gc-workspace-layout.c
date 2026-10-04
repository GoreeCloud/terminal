#include "gc-workspace-layout.h"

#define GC_WORKSPACE_LAYOUT_ERROR 1

typedef struct {
    const char *cursor;
    guint pane_count;
    guint depth;
    gboolean *seen;
} LayoutParser;

static GQuark
workspace_layout_error_quark(void)
{
    return g_quark_from_static_string(
        "goreecloud-terminal-workspace-layout-error"
    );
}

GcWorkspaceLayoutNode *
gc_workspace_layout_leaf(guint pane_index)
{
    GcWorkspaceLayoutNode *node = g_new0(GcWorkspaceLayoutNode, 1);

    node->kind = GC_WORKSPACE_LAYOUT_LEAF;
    node->value.pane_index = pane_index;
    return node;
}

GcWorkspaceLayoutNode *
gc_workspace_layout_split(
    GcWorkspaceLayoutOrientation orientation,
    GcWorkspaceLayoutNode *start,
    GcWorkspaceLayoutNode *end
)
{
    GcWorkspaceLayoutNode *node;

    if (start == NULL || end == NULL) {
        gc_workspace_layout_free(start);
        gc_workspace_layout_free(end);
        return NULL;
    }

    node = g_new0(GcWorkspaceLayoutNode, 1);
    node->kind = GC_WORKSPACE_LAYOUT_SPLIT;
    node->value.split.orientation = orientation;
    node->value.split.start = start;
    node->value.split.end = end;
    return node;
}

void
gc_workspace_layout_free(GcWorkspaceLayoutNode *node)
{
    if (node == NULL) {
        return;
    }

    if (node->kind == GC_WORKSPACE_LAYOUT_SPLIT) {
        gc_workspace_layout_free(node->value.split.start);
        gc_workspace_layout_free(node->value.split.end);
    }

    g_free(node);
}

static void
skip_spaces(LayoutParser *parser)
{
    while (g_ascii_isspace(*parser->cursor)) {
        parser->cursor++;
    }
}

static GcWorkspaceLayoutNode *
parse_node(LayoutParser *parser, GError **error)
{
    GcWorkspaceLayoutNode *start = NULL;
    GcWorkspaceLayoutNode *end = NULL;
    GcWorkspaceLayoutOrientation orientation;
    guint64 pane_index;
    char *number_end = NULL;

    skip_spaces(parser);

    if (parser->depth > GC_WORKSPACE_LAYOUT_MAX_DEPTH) {
        g_set_error(
            error,
            workspace_layout_error_quark(),
            GC_WORKSPACE_LAYOUT_ERROR,
            "Workspace layout exceeds maximum nesting depth"
        );
        return NULL;
    }

    if (*parser->cursor == 'H' || *parser->cursor == 'V') {
        orientation = *parser->cursor == 'H'
            ? GC_WORKSPACE_LAYOUT_HORIZONTAL
            : GC_WORKSPACE_LAYOUT_VERTICAL;
        parser->cursor++;
        skip_spaces(parser);

        if (*parser->cursor != '(') {
            goto malformed;
        }
        parser->cursor++;
        parser->depth++;

        start = parse_node(parser, error);
        if (start == NULL) {
            parser->depth--;
            return NULL;
        }

        skip_spaces(parser);
        if (*parser->cursor != ',') {
            gc_workspace_layout_free(start);
            parser->depth--;
            goto malformed;
        }
        parser->cursor++;

        end = parse_node(parser, error);
        parser->depth--;
        if (end == NULL) {
            gc_workspace_layout_free(start);
            return NULL;
        }

        skip_spaces(parser);
        if (*parser->cursor != ')') {
            gc_workspace_layout_free(start);
            gc_workspace_layout_free(end);
            goto malformed;
        }
        parser->cursor++;

        return gc_workspace_layout_split(orientation, start, end);
    }

    if (!g_ascii_isdigit(*parser->cursor)) {
        goto malformed;
    }

    pane_index = g_ascii_strtoull(parser->cursor, &number_end, 10);
    if (number_end == parser->cursor || pane_index >= parser->pane_count) {
        goto malformed;
    }

    if (parser->seen[pane_index]) {
        g_set_error(
            error,
            workspace_layout_error_quark(),
            GC_WORKSPACE_LAYOUT_ERROR,
            "Workspace layout repeats pane %" G_GUINT64_FORMAT,
            pane_index
        );
        return NULL;
    }

    parser->seen[pane_index] = TRUE;
    parser->cursor = number_end;
    return gc_workspace_layout_leaf((guint) pane_index);

malformed:
    g_set_error(
        error,
        workspace_layout_error_quark(),
        GC_WORKSPACE_LAYOUT_ERROR,
        "Workspace layout is malformed"
    );
    return NULL;
}

GcWorkspaceLayoutNode *
gc_workspace_layout_parse(
    const char *text,
    guint pane_count,
    GError **error
)
{
    LayoutParser parser;
    GcWorkspaceLayoutNode *root;
    gboolean *seen;

    if (text == NULL || pane_count == 0 ||
        pane_count > GC_WORKSPACE_LAYOUT_MAX_PANES) {
        g_set_error(
            error,
            workspace_layout_error_quark(),
            GC_WORKSPACE_LAYOUT_ERROR,
            "Workspace layout pane count is invalid"
        );
        return NULL;
    }

    seen = g_new0(gboolean, pane_count);
    parser.cursor = text;
    parser.pane_count = pane_count;
    parser.depth = 0;
    parser.seen = seen;

    root = parse_node(&parser, error);
    if (root == NULL) {
        g_free(seen);
        return NULL;
    }

    skip_spaces(&parser);
    if (*parser.cursor != '\0') {
        gc_workspace_layout_free(root);
        g_free(seen);
        g_set_error(
            error,
            workspace_layout_error_quark(),
            GC_WORKSPACE_LAYOUT_ERROR,
            "Workspace layout has trailing content"
        );
        return NULL;
    }

    for (guint i = 0; i < pane_count; i++) {
        if (!seen[i]) {
            gc_workspace_layout_free(root);
            g_free(seen);
            g_set_error(
                error,
                workspace_layout_error_quark(),
                GC_WORKSPACE_LAYOUT_ERROR,
                "Workspace layout does not reference every pane"
            );
            return NULL;
        }
    }

    g_free(seen);
    return root;
}

static void
append_serialized(const GcWorkspaceLayoutNode *node, GString *text)
{
    if (node->kind == GC_WORKSPACE_LAYOUT_LEAF) {
        g_string_append_printf(text, "%u", node->value.pane_index);
        return;
    }

    g_string_append_c(
        text,
        node->value.split.orientation == GC_WORKSPACE_LAYOUT_HORIZONTAL
            ? 'H'
            : 'V'
    );
    g_string_append_c(text, '(');
    append_serialized(node->value.split.start, text);
    g_string_append_c(text, ',');
    append_serialized(node->value.split.end, text);
    g_string_append_c(text, ')');
}

char *
gc_workspace_layout_serialize(const GcWorkspaceLayoutNode *node)
{
    GString *text;

    if (node == NULL) {
        return NULL;
    }

    text = g_string_new(NULL);
    append_serialized(node, text);
    return g_string_free(text, FALSE);
}
