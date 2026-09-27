#include "gc-context.h"

#include <unistd.h>

char *
gc_context_identity(void)
{
    const char *user = g_get_user_name();
    const char *host = g_get_host_name();

    if (user == NULL || *user == '\0') {
        user = "unknown-user";
    }

    if (host == NULL || *host == '\0') {
        host = "unknown-host";
    }

    return g_strdup_printf("%s@%s", user, host);
}

const char *
gc_context_shell(void)
{
    const char *shell = g_getenv("SHELL");

    if (shell == NULL || *shell == '\0') {
        return "/bin/sh";
    }

    return shell;
}

const char *
gc_context_privilege_label(void)
{
    return geteuid() == 0 ? "Elevated local" : "Local";
}
