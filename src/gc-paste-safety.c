#include "gc-paste-safety.h"

gboolean
gc_paste_text_requires_review(const char *text)
{
    if (text == NULL || *text == '\0') {
        return FALSE;
    }

    return strchr(text, '\n') != NULL || strchr(text, '\r') != NULL;
}

guint
gc_paste_text_line_count(const char *text)
{
    guint lines = 0;

    if (text == NULL || *text == '\0') {
        return 0;
    }

    lines = 1;
    for (const char *cursor = text; *cursor != '\0'; cursor++) {
        if (*cursor == '\r') {
            lines++;
            if (cursor[1] == '\n') {
                cursor++;
            }
        } else if (*cursor == '\n') {
            lines++;
        }
    }

    return lines;
}
