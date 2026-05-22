/* scope.h — Scope as physical boundary (§2.2).
 *
 * In Spherepop, scope is not a stack frame; it is a physical boundary
 * of a bubble. What is inside a boundary is local to it. Scope chains
 * follow bubble containment, not call stacks.
 */
#ifndef SP_SCOPE_H
#define SP_SCOPE_H

#include <stdint.h>
#include <stdbool.h>

struct Value;

typedef struct ScopeEntry {
    char          *name;
    struct Value  *value;
    bool           mutable;
    struct ScopeEntry *next;
} ScopeEntry;

typedef struct Scope {
    ScopeEntry   *entries;
    struct Scope *parent;
    int           depth;
} Scope;

Scope        *scope_create(Scope *parent);
void          scope_free(Scope *s);
void          scope_define(Scope *s, const char *name, struct Value *val, bool mutable);
struct Value *scope_lookup(const Scope *s, const char *name);
bool          scope_assign(Scope *s, const char *name, struct Value *val);
bool          scope_contains_local(const Scope *s, const char *name);
void          scope_print(const Scope *s);

#endif /* SP_SCOPE_H */
