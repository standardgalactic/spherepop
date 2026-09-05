/* collapse_rules.h — Named collapse rules over a History: pure functions
 * from event log to an external observation, never mutating H. Mirrors
 * spherepop-kernel/src/collapse.rs and run_python.py's collapse_* helpers.
 */
#ifndef SP_FIXTURES_COLLAPSE_RULES_H
#define SP_FIXTURES_COLLAPSE_RULES_H

#include "kernel.h"

/* Simple union-find over ObjectIds, built lazily as ids are seen. */
typedef struct { ObjectId *ids; ObjectId *parent; size_t len, cap; } UnionFind;

void   uf_init(UnionFind *uf);
void   uf_free(UnionFind *uf);
void   uf_union(UnionFind *uf, ObjectId a, ObjectId b);
bool   uf_same_class(UnionFind *uf, ObjectId a, ObjectId b);

/* Union(a, b) for every Bind event whose tag is not "__meta__". */
UnionFind collapse_quotient(const Event *history, size_t n);

/* Same, but skips any Bind(a, b) for which a later-irrelevant-order
 * Refuse(a, "relation withdrawn", b) (or Refuse(b, ..., a)) exists
 * anywhere in the history -- i.e. Unlink's withdrawal is honored. */
UnionFind collapse_quotient_honoring_refusals(const Event *history, size_t n);

/* Metadata map: for each Bind(a, b, "__meta__"), records that object `a`
 * has metadata (SetMeta's encoding -- see sugar.h). `has_meta` reports
 * whether an object ever appeared as the target of such a bind. */
bool collapse_meta_has_key(const Event *history, size_t n, ObjectId key);

#endif /* SP_FIXTURES_COLLAPSE_RULES_H */
