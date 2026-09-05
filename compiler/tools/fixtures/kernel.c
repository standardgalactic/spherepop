/* kernel.c — canonical event/state/arbiter model. See kernel.h. */
#include "kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *dupstr(const char *s) {
    if (!s) s = "";
    size_t n = strlen(s);
    char *out = malloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

/* ---------------- Event ---------------- */

Event event_pop(ObjectId a) {
    Event e = {0};
    e.kind = EV_POP;
    e.a = a;
    return e;
}

Event event_refuse(ObjectId a, const char *reason) {
    Event e = {0};
    e.kind = EV_REFUSE;
    e.a = a;
    e.reason = dupstr(reason);
    return e;
}

Event event_refuse_bind(ObjectId a, ObjectId b, const char *reason) {
    Event e = event_refuse(a, reason);
    e.has_b = true;
    e.b = b;
    return e;
}

Event event_bind(ObjectId a, ObjectId b, const char *tag) {
    Event e = {0};
    e.kind = EV_BIND;
    e.a = a;
    e.b = b;
    e.has_b = true;
    e.tag = dupstr(tag);
    return e;
}

Event event_collapse(const char *rule) {
    Event e = {0};
    e.kind = EV_COLLAPSE;
    e.rule = dupstr(rule);
    return e;
}

void event_free(Event *e) {
    if (!e) return;
    free(e->tag);
    free(e->reason);
    free(e->rule);
    e->tag = e->reason = e->rule = NULL;
}

/* ---------------- ObjectSet ---------------- */

void objset_init(ObjectSet *s) { s->items = NULL; s->len = 0; s->cap = 0; }
void objset_free(ObjectSet *s) { free(s->items); s->items = NULL; s->len = s->cap = 0; }

bool objset_contains(const ObjectSet *s, ObjectId x) {
    for (size_t i = 0; i < s->len; i++) if (s->items[i] == x) return true;
    return false;
}

void objset_add(ObjectSet *s, ObjectId x) {
    if (objset_contains(s, x)) return;
    if (s->len >= s->cap) { s->cap = s->cap ? s->cap * 2 : 4; s->items = realloc(s->items, s->cap * sizeof(ObjectId)); }
    s->items[s->len++] = x;
}

void objset_remove(ObjectSet *s, ObjectId x) {
    for (size_t i = 0; i < s->len; i++) {
        if (s->items[i] == x) {
            s->items[i] = s->items[s->len - 1];
            s->len--;
            return;
        }
    }
}

static int cmp_u64(const void *a, const void *b) {
    ObjectId x = *(const ObjectId *)a, y = *(const ObjectId *)b;
    return (x > y) - (x < y);
}

bool objset_equals_array(const ObjectSet *s, const ObjectId *expected, size_t n) {
    if (s->len != n) return false;
    if (n == 0) return true;
    ObjectId *sa = malloc(n * sizeof(ObjectId));
    ObjectId *sb = malloc(n * sizeof(ObjectId));
    memcpy(sa, s->items, n * sizeof(ObjectId));
    memcpy(sb, expected, n * sizeof(ObjectId));
    qsort(sa, n, sizeof(ObjectId), cmp_u64);
    qsort(sb, n, sizeof(ObjectId), cmp_u64);
    bool eq = memcmp(sa, sb, n * sizeof(ObjectId)) == 0;
    free(sa); free(sb);
    return eq;
}

/* ---------------- BoundList ---------------- */

void boundlist_init(BoundList *l) { l->items = NULL; l->len = 0; l->cap = 0; }
void boundlist_free(BoundList *l) {
    for (size_t i = 0; i < l->len; i++) free(l->items[i].tag);
    free(l->items);
    l->items = NULL; l->len = l->cap = 0;
}
void boundlist_add(BoundList *l, ObjectId a, ObjectId b, const char *tag) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 4; l->items = realloc(l->items, l->cap * sizeof(BoundFact)); }
    l->items[l->len].a = a;
    l->items[l->len].b = b;
    l->items[l->len].tag = dupstr(tag);
    l->len++;
}
bool boundlist_contains(const BoundList *l, ObjectId a, ObjectId b, const char *tag) {
    for (size_t i = 0; i < l->len; i++) {
        if (l->items[i].a == a && l->items[i].b == b && strcmp(l->items[i].tag, tag ? tag : "") == 0)
            return true;
    }
    return false;
}

/* ---------------- RefusedList ---------------- */

void refusedlist_init(RefusedList *l) { l->items = NULL; l->len = 0; l->cap = 0; }
void refusedlist_free(RefusedList *l) {
    for (size_t i = 0; i < l->len; i++) free(l->items[i].reason);
    free(l->items);
    l->items = NULL; l->len = l->cap = 0;
}
void refusedlist_add(RefusedList *l, ObjectId a, const char *reason) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 4; l->items = realloc(l->items, l->cap * sizeof(RefusedFact)); }
    l->items[l->len].a = a;
    l->items[l->len].reason = dupstr(reason);
    l->len++;
}

/* ---------------- ObservedList ---------------- */

void observedlist_init(ObservedList *l) { l->items = NULL; l->len = 0; l->cap = 0; }
void observedlist_free(ObservedList *l) {
    for (size_t i = 0; i < l->len; i++) free(l->items[i].rule);
    free(l->items);
    l->items = NULL; l->len = l->cap = 0;
}
void observedlist_add(ObservedList *l, const char *rule) {
    if (l->len >= l->cap) { l->cap = l->cap ? l->cap * 2 : 4; l->items = realloc(l->items, l->cap * sizeof(ObservedFact)); }
    l->items[l->len].rule = dupstr(rule);
    l->len++;
}

/* ---------------- State ---------------- */

void state_init(State *s, const ObjectSet *omega0) {
    objset_init(&s->option_space);
    for (size_t i = 0; i < omega0->len; i++) objset_add(&s->option_space, omega0->items[i]);
    objset_init(&s->committed);
    boundlist_init(&s->bound);
    refusedlist_init(&s->refused);
    observedlist_init(&s->observed);
}

void state_free(State *s) {
    objset_free(&s->option_space);
    objset_free(&s->committed);
    boundlist_free(&s->bound);
    refusedlist_free(&s->refused);
    observedlist_free(&s->observed);
}

void state_apply(State *s, const Event *e) {
    switch (e->kind) {
        case EV_POP:
            objset_remove(&s->option_space, e->a);
            objset_add(&s->committed, e->a);
            break;
        case EV_REFUSE:
            refusedlist_add(&s->refused, e->a, e->reason);
            /* Omega untouched: refusal documents, it does not foreclose. */
            break;
        case EV_BIND:
            boundlist_add(&s->bound, e->a, e->b, e->tag);
            break;
        case EV_COLLAPSE:
            observedlist_add(&s->observed, e->rule);
            break;
    }
}

bool state_equals(const State *a, const State *b) {
    if (!objset_equals_array(&a->option_space, b->option_space.items, b->option_space.len)) return false;
    if (!objset_equals_array(&a->committed, b->committed.items, b->committed.len)) return false;
    if (a->bound.len != b->bound.len) return false;
    for (size_t i = 0; i < a->bound.len; i++)
        if (!boundlist_contains(&b->bound, a->bound.items[i].a, a->bound.items[i].b, a->bound.items[i].tag)) return false;
    if (a->refused.len != b->refused.len) return false;
    if (a->observed.len != b->observed.len) return false;
    for (size_t i = 0; i < a->observed.len; i++)
        if (strcmp(a->observed.items[i].rule, b->observed.items[i].rule) != 0) return false;
    return true;
}

/* ---------------- StringSet ---------------- */

void strset_init(StringSet *s) { s->items = NULL; s->len = 0; s->cap = 0; }
void strset_free(StringSet *s) {
    for (size_t i = 0; i < s->len; i++) free(s->items[i]);
    free(s->items);
    s->items = NULL; s->len = s->cap = 0;
}
bool strset_contains(const StringSet *s, const char *v) {
    for (size_t i = 0; i < s->len; i++) if (strcmp(s->items[i], v) == 0) return true;
    return false;
}
void strset_add(StringSet *s, const char *v) {
    if (strset_contains(s, v)) return;
    if (s->len >= s->cap) { s->cap = s->cap ? s->cap * 2 : 4; s->items = realloc(s->items, s->cap * sizeof(char *)); }
    s->items[s->len++] = dupstr(v);
}

/* ---------------- Arbiter ---------------- */

void arbiter_init(Arbiter *arb, const ObjectId *omega0, size_t n_omega0,
                   const char **rules, size_t n_rules) {
    objset_init(&arb->omega0);
    for (size_t i = 0; i < n_omega0; i++) objset_add(&arb->omega0, omega0[i]);
    strset_init(&arb->rules);
    for (size_t i = 0; i < n_rules; i++) strset_add(&arb->rules, rules[i]);
    arb->history = NULL;
    arb->history_len = 0;
    arb->history_cap = 0;
}

void arbiter_free(Arbiter *arb) {
    objset_free(&arb->omega0);
    strset_free(&arb->rules);
    for (size_t i = 0; i < arb->history_len; i++) event_free(&arb->history[i]);
    free(arb->history);
    arb->history = NULL;
    arb->history_len = arb->history_cap = 0;
}

size_t arbiter_len(const Arbiter *arb) { return arb->history_len; }

State arbiter_state(const Arbiter *arb) {
    State s;
    state_init(&s, &arb->omega0);
    for (size_t i = 0; i < arb->history_len; i++) state_apply(&s, &arb->history[i]);
    return s;
}

bool arbiter_submit(Arbiter *arb, Event *events, size_t n, char *err_out) {
    State s = arbiter_state(arb);
    ObjectSet hypothetical_committed;
    objset_init(&hypothetical_committed);

    bool ok = true;
    for (size_t i = 0; i < n && ok; i++) {
        const Event *e = &events[i];
        switch (e->kind) {
            case EV_POP: {
                bool still_available = objset_contains(&s.option_space, e->a) &&
                                        !objset_contains(&hypothetical_committed, e->a);
                if (!still_available) {
                    snprintf(err_out, 128, "PopOutsideOptionSpace(%llu)", (unsigned long long)e->a);
                    ok = false;
                    break;
                }
                objset_add(&hypothetical_committed, e->a);
                break;
            }
            case EV_REFUSE:
                if (!e->reason || e->reason[0] == '\0') {
                    snprintf(err_out, 128, "RefuseWithoutReason");
                    ok = false;
                }
                break;
            case EV_BIND:
                /* a/b are plain ObjectId (uint64_t), always "present";
                 * the Rust/Python originals guard against a JSON-level
                 * null, which our required-field JSON parsing already
                 * rejects earlier in main.c, so there is nothing further
                 * to check here. */
                break;
            case EV_COLLAPSE:
                if (!e->rule) {
                    snprintf(err_out, 128, "Malformed(Collapse missing rule)");
                    ok = false;
                    break;
                }
                if (!strset_contains(&arb->rules, e->rule)) {
                    snprintf(err_out, 128, "UncertifiedCollapseRule(%s)", e->rule);
                    ok = false;
                }
                break;
        }
    }

    objset_free(&hypothetical_committed);
    state_free(&s);

    if (!ok) return false;

    /* All-or-nothing: only mutate history after every event validates. */
    for (size_t i = 0; i < n; i++) {
        if (arb->history_len >= arb->history_cap) {
            arb->history_cap = arb->history_cap ? arb->history_cap * 2 : 8;
            arb->history = realloc(arb->history, arb->history_cap * sizeof(Event));
        }
        arb->history[arb->history_len++] = events[i]; /* move: takes ownership of strings */
    }
    return true;
}
