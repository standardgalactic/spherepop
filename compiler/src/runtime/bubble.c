#define _POSIX_C_SOURCE 200809L
/* bubble.c — Bubble lifecycle, pop, refuse, collapse, bind.
 *
 * The pop() implementation follows the event-driven model: it emits
 * EV_POP_BEGIN and EV_POP_COMMIT rather than directly invoking provenance,
 * admissibility, or topology subsystems. Those subsystems are listeners.
 */
#include "bubble.h"
#include "history.h"
#include "constraints.h"
#include "provenance.h"
#include "scope.h"
#include "admissibility.h"
#include "collapse.h"
#include "evaluator.h"
#include "../parser/ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* --- Value type (minimal inline definition for self-containment) --- */
#include "value.h"

static uint64_t s_next_id = 1;

BubbleID bubble_next_id(void) {
    return s_next_id++;
}

Bubble *bubble_alloc(Bubble *parent, Scope *scope) {
    Bubble *b = calloc(1, sizeof(Bubble));
    if (!b) { perror("bubble_alloc"); exit(1); }
    b->id                  = bubble_next_id();
    b->state               = BUBBLE_ACTIVE;
    b->parent              = parent;
    b->scope               = scope;
    b->constraints         = cset_create();
    b->history             = history_create();
    b->prov_id             = PROV_ID_NULL;
    b->local_admissibility = 1.0;
    b->action_cost         = 0.0;
    b->depth               = parent ? parent->depth + 1 : 0;
    b->children_cap        = 4;
    b->children            = malloc(b->children_cap * sizeof(Bubble *));
    if (!b->children) { perror("bubble_alloc children"); exit(1); }
    if (parent) bubble_add_child(parent, b);
    return b;
}

void bubble_free(Bubble *b) {
    if (!b) return;
    for (int i = 0; i < b->n_children; i++)
        bubble_free(b->children[i]);
    free(b->children);
    cset_free(b->constraints);
    history_free(b->history);
    free(b);
}

void bubble_add_child(Bubble *parent, Bubble *child) {
    if (parent->n_children == parent->children_cap) {
        parent->children_cap *= 2;
        parent->children = realloc(parent->children,
                                   parent->children_cap * sizeof(Bubble *));
    }
    parent->children[parent->n_children++] = child;
}

void bubble_remove_child(Bubble *parent, Bubble *child) {
    for (int i = 0; i < parent->n_children; i++) {
        if (parent->children[i] == child) {
            memmove(&parent->children[i], &parent->children[i+1],
                    (parent->n_children - i - 1) * sizeof(Bubble *));
            parent->n_children--;
            return;
        }
    }
}

bool bubble_is_innermost(const Bubble *b) {
    /* Innermost: no active (not yet popped) child bubbles */
    for (int i = 0; i < b->n_children; i++)
        if (b->children[i]->state == BUBBLE_ACTIVE)
            return false;
    return true;
}

bool bubble_constraints_satisfied(const Bubble *b) {
    return cset_all_satisfied(b->constraints);
}

bool bubble_is_admissible(const Bubble *b) {
    return bubble_is_innermost(b) && bubble_constraints_satisfied(b);
}

void bubble_recalc_admissibility(Bubble *b) {
    b->local_admissibility = cset_admissibility_score(b->constraints);
}

double bubble_global_admissibility(const Bubble *root) {
    if (!root) return 0.0;
    double sum = root->local_admissibility;
    for (int i = 0; i < root->n_children; i++)
        sum += bubble_global_admissibility(root->children[i]);
    return sum; /* Caller normalizes as appropriate */
}

/* --- Pop: the primary operator (§1.2, §3.2) ---
 *
 * 1. Check local admissibility
 * 2. Emit EV_POP_BEGIN (listeners: admissibility_listener, history_listener)
 * 3. Evaluate expression
 * 4. Record pop in global history
 * 5. Emit EV_POP_COMMIT (listeners: provenance_listener, topology_listener)
 * 6. Update action cost
 * 7. Propagate constraints outward
 */
Value *bubble_pop(Bubble *b, EvalContext *ctx) {
    if (!bubble_is_admissible(b)) {
        fprintf(stderr, "pop: bubble %llu is not admissible (local=%.3f)\n",
                (unsigned long long)b->id, b->local_admissibility);
        return NULL;
    }

    /* Emit POP_BEGIN */
    SphereEvent ev_begin = {
        .type      = EV_POP_BEGIN,
        .bubble_id = b->id,
        .timestamp = (uint64_t)ctx->step,
        .payload   = b
    };
    event_emit(ctx->bus, &ev_begin);

    /* Evaluate contents */
    Value *result = eval_node(b->expression, b, ctx);
    b->result = result;
    b->state  = BUBBLE_POPPED;

    /* Accumulate action cost */
    double action_delta = 1.0 + b->depth * 0.1;
    b->action_cost += action_delta;
    if (b->parent) b->parent->action_cost += action_delta;

    /* Append to local history */
    history_append(b->history, HE_POP_COMMIT, b->id,
                   b->local_admissibility, 1.0, action_delta, result);

    /* Remove from parent child list (topology update) */
    if (b->parent)
        bubble_remove_child(b->parent, b);

    /* Propagate constraints outward */
    if (b->parent && b->constraints)
        cset_propagate(b->parent->constraints, b->constraints, CPROP_OUTWARD);

    /* Emit POP_COMMIT */
    SphereEvent ev_commit = {
        .type      = EV_POP_COMMIT,
        .bubble_id = b->id,
        .timestamp = (uint64_t)ctx->step++,
        .payload   = result
    };
    event_emit(ctx->bus, &ev_commit);

    return result;
}

/* --- Refuse: mark bubble as inadmissible, record refusal (§3.3) --- */
bool bubble_refuse(Bubble *b, EvalContext *ctx) {
    b->state = BUBBLE_REFUSED;
    history_append(b->history, HE_REFUSE, b->id,
                   b->local_admissibility, 0.0, 0.0, NULL);
    SphereEvent ev = {
        .type      = EV_REFUSE,
        .bubble_id = b->id,
        .timestamp = (uint64_t)ctx->step++,
        .payload   = NULL
    };
    event_emit(ctx->bus, &ev);
    return true;
}

/* --- Collapse: apply quotient operator (§3.4) --- */
Bubble *bubble_collapse(Bubble *b, CollapseOp *op) {
    if (!op) return b;
    b->state = BUBBLE_COLLAPSED;
    history_append(b->history, HE_COLLAPSE, b->id, b->local_admissibility,
                   b->local_admissibility, 0.0, op);
    return b;
}

/* --- Bind: establish constraint membrane (§3.5) --- */
void bubble_bind(Bubble *b, ConstraintSet *cs) {
    b->state = BUBBLE_BOUND;
    if (cs) {
        /* Merge cs into b->constraints */
        ConstraintSet *merged = cset_merge(b->constraints, cs);
        cset_free(b->constraints);
        b->constraints = merged;
    }
    history_append(b->history, HE_BIND, b->id, b->local_admissibility,
                   b->local_admissibility, 0.0, cs);
}

void bubble_print(const Bubble *b, int indent) {
    if (!b) return;
    for (int i = 0; i < indent; i++) printf("  ");
    printf("Bubble[%llu] state=%d depth=%d adm=%.3f action=%.3f\n",
           (unsigned long long)b->id, b->state, b->depth,
           b->local_admissibility, b->action_cost);
    for (int i = 0; i < b->n_children; i++)
        bubble_print(b->children[i], indent + 1);
}
