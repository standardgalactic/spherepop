/* collapse_rules.c — union-find + named collapse rules. See
 * collapse_rules.h.
 */
#include "collapse_rules.h"
#include <stdlib.h>
#include <string.h>

void uf_init(UnionFind *uf) { uf->ids = NULL; uf->parent = NULL; uf->len = 0; uf->cap = 0; }
void uf_free(UnionFind *uf) { free(uf->ids); free(uf->parent); uf->ids = uf->parent = NULL; uf->len = uf->cap = 0; }

static size_t uf_index_of(UnionFind *uf, ObjectId x) {
    for (size_t i = 0; i < uf->len; i++) if (uf->ids[i] == x) return i;
    if (uf->len >= uf->cap) {
        uf->cap = uf->cap ? uf->cap * 2 : 8;
        uf->ids    = realloc(uf->ids,    uf->cap * sizeof(ObjectId));
        uf->parent = realloc(uf->parent, uf->cap * sizeof(ObjectId));
    }
    uf->ids[uf->len]    = x;
    uf->parent[uf->len] = x; /* self-parent: fresh singleton class */
    return uf->len++;
}

static size_t uf_root(UnionFind *uf, size_t i) {
    while (uf->parent[i] != uf->ids[i]) {
        /* path compression: point straight at grandparent's slot */
        size_t p = 0;
        for (size_t k = 0; k < uf->len; k++) if (uf->ids[k] == uf->parent[i]) { p = k; break; }
        uf->parent[i] = uf->parent[p];
        i = p;
    }
    return i;
}

void uf_union(UnionFind *uf, ObjectId a, ObjectId b) {
    size_t ia = uf_index_of(uf, a);
    size_t ib = uf_index_of(uf, b);
    size_t ra = uf_root(uf, ia);
    size_t rb = uf_root(uf, ib);
    if (ra != rb) uf->parent[ra] = uf->ids[rb];
}

bool uf_same_class(UnionFind *uf, ObjectId a, ObjectId b) {
    size_t ia = uf_index_of(uf, a);
    size_t ib = uf_index_of(uf, b);
    return uf->ids[uf_root(uf, ia)] == uf->ids[uf_root(uf, ib)];
}

UnionFind collapse_quotient(const Event *history, size_t n) {
    UnionFind uf;
    uf_init(&uf);
    for (size_t i = 0; i < n; i++) {
        const Event *e = &history[i];
        if (e->kind == EV_BIND && strcmp(e->tag ? e->tag : "", "__meta__") != 0) {
            uf_union(&uf, e->a, e->b);
        }
    }
    return uf;
}

static bool pair_withdrawn(const Event *history, size_t n, ObjectId a, ObjectId b) {
    for (size_t i = 0; i < n; i++) {
        const Event *e = &history[i];
        if (e->kind == EV_REFUSE && e->has_b &&
            e->reason && strcmp(e->reason, "relation withdrawn") == 0) {
            if ((e->a == a && e->b == b) || (e->a == b && e->b == a)) return true;
        }
    }
    return false;
}

UnionFind collapse_quotient_honoring_refusals(const Event *history, size_t n) {
    UnionFind uf;
    uf_init(&uf);
    for (size_t i = 0; i < n; i++) {
        const Event *e = &history[i];
        if (e->kind == EV_BIND && strcmp(e->tag ? e->tag : "", "__meta__") != 0) {
            if (!pair_withdrawn(history, n, e->a, e->b)) {
                uf_union(&uf, e->a, e->b);
            }
        }
    }
    return uf;
}

bool collapse_meta_has_key(const Event *history, size_t n, ObjectId key) {
    for (size_t i = 0; i < n; i++) {
        const Event *e = &history[i];
        if (e->kind == EV_BIND && e->tag && strcmp(e->tag, "__meta__") == 0 && e->a == key) {
            return true;
        }
    }
    return false;
}
