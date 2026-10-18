/* Regression gates for the standalone canonical C event kernel.
 * These verify VALUE equality and Bind set semantics, not fixture counts.
 */
#include "kernel.h"
#include <stdio.h>

static int failures = 0;
#define CHECK(expr, message) do { if (!(expr)) {     fprintf(stderr, "FAIL: %s\n", (message)); failures++; } } while (0)

static void state_pair(State *a, State *b, ObjectSet *initial) {
    objset_init(initial);
    objset_add(initial, 1);
    objset_add(initial, 2);
    state_init(a, initial);
    state_init(b, initial);
}

static void test_refusal_values(void) {
    State a, b;
    ObjectSet omega;
    state_pair(&a, &b, &omega);
    refusedlist_add(&a.refused, 1, "blocked");
    refusedlist_add(&b.refused, 1, "deferred");
    CHECK(!state_equals(&a, &b),
          "same refusal count with different reasons must not compare equal");
    state_free(&a);
    state_free(&b);
    objset_free(&omega);

    state_pair(&a, &b, &omega);
    refusedlist_add(&a.refused, 1, "blocked");
    refusedlist_add(&b.refused, 2, "blocked");
    CHECK(!state_equals(&a, &b),
          "same refusal count with different targets must not compare equal");
    state_free(&a);
    state_free(&b);
    objset_free(&omega);

    state_pair(&a, &b, &omega);
    refusedlist_add(&a.refused, 1, "x");
    refusedlist_add(&a.refused, 2, "y");
    refusedlist_add(&b.refused, 2, "y");
    refusedlist_add(&b.refused, 1, "x");
    CHECK(!state_equals(&a, &b),
          "ordered refusal histories must retain their relative order");
    state_free(&a);
    state_free(&b);
    objset_free(&omega);
}

static void test_event_positions_are_observable(void) {
    State left, right;
    ObjectSet omega;
    state_pair(&left, &right, &omega);
    Event refuse = event_refuse(1, "not ready");
    Event bind = event_bind(1, 2, "adjacent");
    state_apply(&left, &refuse);
    state_apply(&left, &bind);
    state_apply(&right, &bind);
    state_apply(&right, &refuse);
    CHECK(!state_equals(&left, &right),
          "same refusal values at distinct event positions must not compare equal");
    event_free(&refuse);
    event_free(&bind);
    state_free(&left);
    state_free(&right);
    objset_free(&omega);

    state_pair(&left, &right, &omega);
    Event collapse = event_collapse("identity");
    bind = event_bind(1, 2, "adjacent");
    state_apply(&left, &collapse);
    state_apply(&left, &bind);
    state_apply(&right, &bind);
    state_apply(&right, &collapse);
    CHECK(!state_equals(&left, &right),
          "same Collapse rule at distinct event positions must not compare equal");
    event_free(&collapse);
    event_free(&bind);
    state_free(&left);
    state_free(&right);
    objset_free(&omega);
}

static void test_duplicate_binds_are_a_set(void) {
    State a, b;
    ObjectSet omega;
    state_pair(&a, &b, &omega);
    Event bind = event_bind(1, 2, "related");
    state_apply(&a, &bind);
    state_apply(&a, &bind);
    state_apply(&b, &bind);
    CHECK(a.bound.len == 1, "repeated Bind must not increase derived set cardinality");
    CHECK(state_equals(&a, &b),
          "two replay histories with duplicate Bind events must derive the same set");
    event_free(&bind);
    state_free(&a);
    state_free(&b);
    objset_free(&omega);
}

int main(void) {
    test_refusal_values();
    test_duplicate_binds_are_a_set();
    test_event_positions_are_observable();
    if (failures) { fprintf(stderr, "%d canonical state regressions\n", failures); return 1; }
    puts("PASS: canonical C refusal equality and Bind-set invariants");
    return 0;
}
