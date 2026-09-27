#pragma once

#include <glib.h>

G_BEGIN_DECLS

gboolean gc_paste_text_requires_review(const char *text);
guint gc_paste_text_line_count(const char *text);

G_END_DECLS
