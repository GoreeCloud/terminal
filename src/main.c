#include "gc-terminal-window.h"

#include <gtk/gtk.h>

static void
activate(GtkApplication *application, gpointer user_data)
{
    GtkWindow *window;
    (void) user_data;

    window = gc_terminal_window_new(application);
    gtk_window_present(window);
    gc_terminal_window_maybe_show_onboarding(window);
}

int
main(int argc, char **argv)
{
    GtkApplication *application;
    int status;

    application = gtk_application_new(
        "com.goreecloud.Terminal",
        G_APPLICATION_DEFAULT_FLAGS
    );
    g_signal_connect(application, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);

    return status;
}
