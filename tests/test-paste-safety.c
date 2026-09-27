#include "gc-paste-safety.h"

#include <glib.h>

static void
test_review_requirement(void)
{
    g_assert_false(gc_paste_text_requires_review(NULL));
    g_assert_false(gc_paste_text_requires_review(""));
    g_assert_false(gc_paste_text_requires_review("echo hello"));
    g_assert_true(gc_paste_text_requires_review("echo one\necho two"));
    g_assert_true(gc_paste_text_requires_review("echo one\r\necho two"));
    g_assert_true(gc_paste_text_requires_review("echo hello\n"));
}

static void
test_line_count(void)
{
    g_assert_cmpuint(gc_paste_text_line_count(NULL), ==, 0);
    g_assert_cmpuint(gc_paste_text_line_count(""), ==, 0);
    g_assert_cmpuint(gc_paste_text_line_count("one"), ==, 1);
    g_assert_cmpuint(gc_paste_text_line_count("one\ntwo"), ==, 2);
    g_assert_cmpuint(gc_paste_text_line_count("one\r\ntwo"), ==, 2);
    g_assert_cmpuint(gc_paste_text_line_count("one\rtwo\nthree"), ==, 3);
}

int
main(int argc, char **argv)
{
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/paste/review-requirement", test_review_requirement);
    g_test_add_func("/paste/line-count", test_line_count);
    return g_test_run();
}
