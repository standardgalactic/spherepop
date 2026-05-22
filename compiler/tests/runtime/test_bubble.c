/* test_bubble.c — Unit tests for bubble lifecycle. */
#include <stdio.h>
#include "../../src/runtime/bubble.h"
#include "../../src/runtime/scope.h"
#include "../../src/runtime/evaluator.h"

static int tests_run = 0, tests_passed = 0;
#define TEST(name, expr) do { \
    tests_run++; \
    if (expr) { tests_passed++; printf("PASS: %s\n", name); } \
    else { printf("FAIL: %s\n", name); } \
} while(0)

int main(void) {
    printf("=== Bubble Tests ===\n");
    Scope  *s    = scope_create(NULL);
    Bubble *root = bubble_alloc(NULL, s);
    TEST("root id nonzero",          root->id >= 1);
    TEST("root depth 0",             root->depth == 0);
    TEST("root active",              root->state == BUBBLE_ACTIVE);
    TEST("root is innermost",        bubble_is_innermost(root));

    Scope  *cs    = scope_create(s);
    Bubble *child = bubble_alloc(root, cs);
    TEST("child depth 1",            child->depth == 1);
    TEST("child parent correct",     child->parent == root);
    TEST("root not innermost",       !bubble_is_innermost(root));
    TEST("child is innermost",       bubble_is_innermost(child));

    bubble_recalc_admissibility(child);
    TEST("child adm 1.0",            child->local_admissibility >= 0.999);

    EvalContext *ctx = eval_context_create(false);
    bubble_refuse(child, ctx);
    TEST("child REFUSED",            child->state == BUBBLE_REFUSED);

    eval_context_free(ctx);
    bubble_free(root);
    scope_free(s);
    printf("\n%d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
