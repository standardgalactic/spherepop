/* test_admissibility.c */
#include <stdio.h>
#include "../../src/runtime/admissibility.h"
#include "../../src/runtime/scope.h"

static int tests_run = 0, tests_passed = 0;
#define TEST(name, expr) do { \
    tests_run++; \
    if (expr) { tests_passed++; printf("PASS: %s\n", name); } \
    else { printf("FAIL: %s\n", name); } \
} while(0)

int main(void) {
    printf("=== Admissibility Tests ===\n");
    Scope  *s = scope_create(NULL);
    Bubble *b = bubble_alloc(NULL, s);

    AdmissibilityPoint ap = admissibility_evaluate(b);
    TEST("local adm = 1.0",   ap.local >= 0.999);
    TEST("strongly admissible", ap.strongly_admissible);

    AdmissibilityRegion ar = admissibility_region(b);
    TEST("lower = 0",         ar.lower_bound == 0.0);
    TEST("upper = 1",         ar.upper_bound == 1.0);

    double h = admissibility_hamiltonian(b);
    TEST("Hamiltonian finite", h > -1e9 && h < 1e9);

    bubble_free(b);
    scope_free(s);
    printf("\n%d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
