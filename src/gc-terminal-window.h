#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

GtkWindow *gc_terminal_window_new(GtkApplication *application);
void gc_terminal_window_maybe_show_onboarding(GtkWindow *window);

G_END_DECLS
