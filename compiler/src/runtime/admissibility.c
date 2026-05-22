#define _POSIX_C_SOURCE 200809L
/* admissibility.c — Admissibility evaluation and Hamiltonian formulation. */
#include "admissibility.h"
#include "history.h"
#include "constraints.h"
#include <math.h>

AdmissibilityPoint admissibility_evaluate(const Bubble *b) {
    AdmissibilityPoint p = {0};
    if (!b) return p;
    p.local  = cset_admissibility_score(b->constraints);
    p.action = b->action_cost;
    p.strongly_admissible = bubble_is_innermost(b) && p.local >= 0.5;
    p.entropy = -p.local * log(p.local + 1e-9);  /* semantic entropy approx */
    return p;
}

bool admissibility_strong_check(const Bubble *b, const struct History *h) {
    if (!b) return false;
    if (!bubble_is_innermost(b)) return false;
    if (!cset_all_satisfied(b->constraints)) return false;
    return history_is_admissible_trajectory(h);
}

double admissibility_global_score(const struct History *h) {
    if (!h || h->length == 0) return 1.0;
    double sum = 0.0;
    int count = 0;
    const HistoryEntry *e = h->head;
    while (e) {
        sum += e->admissibility_after;
        count++;
        e = e->next;
    }
    return count > 0 ? sum / count : 1.0;
}

AdmissibilityRegion admissibility_region(const Bubble *root) {
    AdmissibilityRegion r = {0};
    if (!root) return r;
    r.anchor      = (Bubble *)root;
    r.lower_bound = 0.0;
    r.upper_bound = 1.0;
    r.dimension   = root->n_children + 1;
    return r;
}

double admissibility_hamiltonian(const Bubble *b) {
    if (!b) return 0.0;
    /* H = local_admissibility - action_cost / (depth + 1) */
    return b->local_admissibility - b->action_cost / (b->depth + 1.0);
}
