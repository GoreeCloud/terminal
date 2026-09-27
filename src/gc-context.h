#pragma once

#include <glib.h>

G_BEGIN_DECLS

char *gc_context_identity(void);
const char *gc_context_shell(void);
const char *gc_context_privilege_label(void);

G_END_DECLS
