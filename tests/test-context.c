#include "gc-context.h"

#include <glib.h>
#include <string.h>

static void
test_identity(void)
{
    g_autofree char *identity = gc_context_identity();
    g_assert_nonnull(identity);
    g_assert_nonnull(strchr(identity, '@'));
}

static void
test_shell(void)
{
    const char *shell = gc_context_shell();
    g_assert_nonnull(shell);
    g_assert_cmpuint(strlen(shell), >, 0);
}

static void
test_privilege(void)
{
    const char *label = gc_context_privilege_label();
    g_assert_true(g_strcmp0(label, "Local") == 0 || g_strcmp0(label, "Elevated local") == 0);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/context/identity", test_identity);
    g_test_add_func("/context/shell", test_shell);
    g_test_add_func("/context/privilege", test_privilege);
    return g_test_run();
}
