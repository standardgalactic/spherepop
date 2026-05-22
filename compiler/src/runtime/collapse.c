#define _POSIX_C_SOURCE 200809L
#define _POSIX_C_SOURCE 200809L
/* collapse.c — Collapse operators and quotient construction (§3.4, A.4). */
#include "collapse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static bool equiv_terminal(const History *a, const History *b, void *ctx) {
    (void)ctx;
    /* Two histories equivalent iff same total action (proxy for same value) */
    if (!a && !b) return true;
    if (!a || !b) return false;
    return fabs(a->total_action - b->total_action) < 1e-9;
}

static bool equiv_eval_order(const History *a, const History *b, void *ctx) {
    (void)ctx;
    if (!a && !b) return true;
    if (!a || !b) return false;
    if (a->length != b->length) return false;
    const HistoryEntry *ea = a->head, *eb = b->head;
    while (ea && eb) {
        if (ea->kind != eb->kind || ea->bubble_id != eb->bubble_id)
            return false;
        ea = ea->next; eb = eb->next;
    }
    return true;
}

static bool equiv_provenance(const History *a, const History *b, void *ctx) {
    (void)ctx;
    return a == b; /* Strict pointer equality — full provenance */
}

CollapseOp *collapse_terminal_value_op(void) {
    CollapseOp *op = calloc(1, sizeof(CollapseOp));
    op->level  = COLLAPSE_TERMINAL_VALUE;
    op->equiv  = equiv_terminal;
    op->label  = "terminal-value";
    return op;
}

CollapseOp *collapse_evaluation_order_op(void) {
    CollapseOp *op = calloc(1, sizeof(CollapseOp));
    op->level  = COLLAPSE_EVALUATION_ORDER;
    op->equiv  = equiv_eval_order;
    op->label  = "evaluation-order";
    return op;
}

CollapseOp *collapse_provenance_op(void) {
    CollapseOp *op = calloc(1, sizeof(CollapseOp));
    op->level  = COLLAPSE_PROVENANCE;
    op->equiv  = equiv_provenance;
    op->label  = "provenance";
    return op;
}

CollapseOp *collapse_semantic_op(void) {
    CollapseOp *op = calloc(1, sizeof(CollapseOp));
    op->level  = COLLAPSE_SEMANTIC;
    op->equiv  = equiv_terminal; /* placeholder: semantic equiv = terminal for now */
    op->label  = "semantic";
    return op;
}

void collapse_op_free(CollapseOp *op) { free(op); }

bool collapse_identifies(const CollapseOp *op, const History *a, const History *b) {
    if (!op || !op->equiv) return false;
    return op->equiv(a, b, op->ctx);
}

Bubble *collapse_apply(Bubble *b, const CollapseOp *op) {
    if (!b || !op) return b;
    b->state = BUBBLE_COLLAPSED;
    return b;
}
