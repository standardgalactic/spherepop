#define _POSIX_C_SOURCE 200809L
/* constraints.c — Constraint set operations and propagation. */
#include "constraints.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static uint32_t s_next_cid = 1;

ConstraintSet *cset_create(void) {
    ConstraintSet *cs = calloc(1, sizeof(ConstraintSet));
    cs->cap      = 8;
    cs->edge_cap = 8;
    cs->constraints = malloc(cs->cap * sizeof(Constraint *));
    cs->edges       = malloc(cs->edge_cap * sizeof(ConstraintEdge));
    return cs;
}

void cset_free(ConstraintSet *cs) {
    if (!cs) return;
    for (int i = 0; i < cs->n; i++) {
        free(cs->constraints[i]);
    }
    free(cs->constraints);
    free(cs->edges);
    free(cs);
}

ConstraintID cset_add(ConstraintSet *cs, ConstraintKind kind,
                      ConstraintPropDir dir, double weight, void *data) {
    if (cs->n == cs->cap) {
        cs->cap *= 2;
        cs->constraints = realloc(cs->constraints, cs->cap * sizeof(Constraint *));
    }
    Constraint *c = calloc(1, sizeof(Constraint));
    c->id         = s_next_cid++;
    c->kind       = kind;
    c->direction  = dir;
    c->weight     = weight;
    c->data       = data;
    c->satisfied  = true; /* optimistic by default */
    cs->constraints[cs->n++] = c;
    return c->id;
}

void cset_add_edge(ConstraintSet *cs, ConstraintID from, ConstraintID to,
                   double weight) {
    if (cs->n_edges == cs->edge_cap) {
        cs->edge_cap *= 2;
        cs->edges = realloc(cs->edges, cs->edge_cap * sizeof(ConstraintEdge));
    }
    cs->edges[cs->n_edges++] = (ConstraintEdge){ from, to, weight };
}

bool cset_all_satisfied(const ConstraintSet *cs) {
    if (!cs) return true;
    for (int i = 0; i < cs->n; i++)
        if (!cs->constraints[i]->satisfied) return false;
    return true;
}

double cset_admissibility_score(const ConstraintSet *cs) {
    if (!cs || cs->n == 0) return 1.0;
    double total_weight = 0.0, sat_weight = 0.0;
    for (int i = 0; i < cs->n; i++) {
        total_weight += cs->constraints[i]->weight;
        if (cs->constraints[i]->satisfied)
            sat_weight += cs->constraints[i]->weight;
    }
    return total_weight > 0 ? sat_weight / total_weight : 1.0;
}

void cset_propagate(ConstraintSet *dst, const ConstraintSet *src,
                    ConstraintPropDir dir) {
    if (!src || !dst) return;
    for (int i = 0; i < src->n; i++) {
        if (src->constraints[i]->direction == dir)
            cset_add(dst, src->constraints[i]->kind, dir,
                     src->constraints[i]->weight * 0.5, /* attenuated */
                     src->constraints[i]->data);
    }
}

ConstraintSet *cset_merge(const ConstraintSet *a, const ConstraintSet *b) {
    ConstraintSet *cs = cset_create();
    if (a) for (int i = 0; i < a->n; i++)
        cset_add(cs, a->constraints[i]->kind, a->constraints[i]->direction,
                 a->constraints[i]->weight, a->constraints[i]->data);
    if (b) for (int i = 0; i < b->n; i++)
        cset_add(cs, b->constraints[i]->kind, b->constraints[i]->direction,
                 b->constraints[i]->weight, b->constraints[i]->data);
    return cs;
}

void cset_print(const ConstraintSet *cs) {
    if (!cs) { printf("<null constraints>\n"); return; }
    printf("ConstraintSet[%d]:\n", cs->n);
    for (int i = 0; i < cs->n; i++) {
        Constraint *c = cs->constraints[i];
        printf("  [%u] kind=%d dir=%d weight=%.2f sat=%s\n",
               c->id, c->kind, c->direction, c->weight,
               c->satisfied ? "yes" : "NO");
    }
}
