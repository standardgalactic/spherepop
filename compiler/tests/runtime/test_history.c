/* test_history.c — Unit tests for history. */
#include <stdio.h>
#include "../../src/runtime/history.h"

static int tests_run = 0, tests_passed = 0;
#define TEST(name, expr) do { \
    tests_run++; \
    if (expr) { tests_passed++; printf("PASS: %s\n", name); } \
    else { printf("FAIL: %s\n", name); } \
} while(0)

int main(void) {
    printf("=== History Tests ===\n");
    History *h = history_create();
    TEST("empty length 0",       h->length == 0);
    TEST("empty action 0",       h->total_action == 0.0);
    TEST("empty admissible",     history_is_admissible_trajectory(h));

    history_append(h, HE_POP_COMMIT, 1, 1.0, 1.0, 2.5, NULL);
    TEST("length 1",             h->length == 1);
    TEST("action 2.5",          h->total_action == 2.5);
    TEST("still admissible",     history_is_admissible_trajectory(h));

    history_append(h, HE_REFUSE, 2, 1.0, 0.0, 0.0, NULL);
    TEST("refuse = inadmissible",!history_is_admissible_trajectory(h));

    history_free(h);
    printf("\n%d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
