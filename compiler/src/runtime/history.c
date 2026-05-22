#define _POSIX_C_SOURCE 200809L
#define _POSIX_C_SOURCE 200809L
/* history.c — History construction, printing, and trajectory analysis. */
#include "history.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

History *history_create(void) {
    History *h = calloc(1, sizeof(History));
    if (!h) { perror("history_create"); exit(1); }
    return h;
}

void history_free(History *h) {
    if (!h) return;
    HistoryEntry *e = h->head;
    while (e) {
        HistoryEntry *next = e->next;
        free(e);
        e = next;
    }
    free(h);
}

void history_append(History *h, HistoryEventKind kind, BubbleID bid,
                    double adm_before, double adm_after,
                    double action_delta, void *payload) {
    static uint64_t t = 0;
    HistoryEntry *e = calloc(1, sizeof(HistoryEntry));
    if (!e) { perror("history_append"); exit(1); }
    e->kind               = kind;
    e->bubble_id          = bid;
    e->t                  = t++;
    e->admissibility_before = adm_before;
    e->admissibility_after  = adm_after;
    e->action_delta         = action_delta;
    e->payload              = payload;
    if (h->tail) h->tail->next = e;
    else         h->head = e;
    h->tail = e;
    h->length++;
    h->total_action += action_delta;
}

static const char *event_kind_name(HistoryEventKind k) {
    switch (k) {
        case HE_POP_BEGIN:            return "POP_BEGIN";
        case HE_POP_COMMIT:           return "POP_COMMIT";
        case HE_REFUSE:               return "REFUSE";
        case HE_COLLAPSE:             return "COLLAPSE";
        case HE_BIND:                 return "BIND";
        case HE_SCOPE_ENTER:          return "SCOPE_ENTER";
        case HE_SCOPE_EXIT:           return "SCOPE_EXIT";
        case HE_CONSTRAINT_UPDATE:    return "CONSTRAINT_UPDATE";
        case HE_ADMISSIBILITY_RECALC: return "ADMISSIBILITY_RECALC";
        default:                      return "UNKNOWN";
    }
}

void history_print(const History *h) {
    if (!h) { printf("<null history>\n"); return; }
    printf("History[%d entries, S=%.3f]:\n", h->length, h->total_action);
    HistoryEntry *e = h->head;
    int i = 0;
    while (e) {
        printf("  [%d] t=%llu bubble=%llu %s adm:%.3f->%.3f dS=%.3f\n",
               i++,
               (unsigned long long)e->t,
               (unsigned long long)e->bubble_id,
               event_kind_name(e->kind),
               e->admissibility_before,
               e->admissibility_after,
               e->action_delta);
        e = e->next;
    }
}

double history_action(const History *h) {
    return h ? h->total_action : 0.0;
}

bool history_is_admissible_trajectory(const History *h) {
    if (!h) return false;
    HistoryEntry *e = h->head;
    while (e) {
        /* A trajectory is inadmissible if any refused pop appears
         * before a committed pop on the same bubble. */
        if (e->kind == HE_REFUSE) return false;
        e = e->next;
    }
    return true;
}

bool history_equivalent(const History *a, const History *b, int collapse_level) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    /* Level 0: only terminal value matters */
    if (collapse_level == 0) {
        /* Compare last POP_COMMIT payloads — simplified */
        return a->total_action == b->total_action; /* placeholder */
    }
    /* Level 1: evaluation order must match */
    if (collapse_level == 1) {
        if (a->length != b->length) return false;
        HistoryEntry *ea = a->head, *eb = b->head;
        while (ea && eb) {
            if (ea->kind != eb->kind) return false;
            ea = ea->next; eb = eb->next;
        }
        return true;
    }
    /* Level 2+: full provenance equivalence */
    return a == b;
}

History *history_merge(const History *a, const History *b) {
    History *h = history_create();
    HistoryEntry *e = a ? a->head : NULL;
    while (e) {
        history_append(h, e->kind, e->bubble_id, e->admissibility_before,
                       e->admissibility_after, e->action_delta, e->payload);
        e = e->next;
    }
    e = b ? b->head : NULL;
    while (e) {
        history_append(h, e->kind, e->bubble_id, e->admissibility_before,
                       e->admissibility_after, e->action_delta, e->payload);
        e = e->next;
    }
    return h;
}
