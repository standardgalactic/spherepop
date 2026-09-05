/* kernel.h — A standalone C port of the canonical four-primitive kernel
 * (Pop, Refuse, Bind, Collapse) described in Spherepop_Specifications.tex
 * and already implemented independently in Rust
 * (spherepop-kernel/src/{event,history,arbiter,collapse,sugar}.rs) and
 * Python (experiments/flat/run_python.py).
 *
 * This is deliberately NOT built on top of compiler/'s Bubble/EvalContext
 * runtime: those model a general-purpose expression language whose own
 * Bind/Collapse are known to diverge from the canonical semantics (see
 * IMPLEMENTATIONS.md). This module instead ports the canonical event-log
 * model directly, exactly like spherepop-kernel does in Rust, so that a
 * genuine third implementation of experiments/flat/'s fixtures exists in
 * C, agreeing (or not) with the Rust and Python ones on the same inputs.
 *
 * Object ids are small non-negative integers throughout these fixtures,
 * so every collection here is a flat, linearly-searched dynamic array --
 * correct and simple for fixture-sized inputs, not intended as a
 * general-purpose high-performance kernel.
 */
#ifndef SP_FIXTURES_KERNEL_H
#define SP_FIXTURES_KERNEL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef uint64_t ObjectId;

typedef enum { EV_POP, EV_REFUSE, EV_BIND, EV_COLLAPSE } EventKind;

/* An Event is the sole unit ever appended to a History. Optional fields
 * not used by a given `kind` are left NULL/0/false; `has_b` distinguishes
 * a plain Refuse(a) from a Refuse(a) that also names a related object b
 * (used by Unlink, and read back by the "honoring refusals" collapse
 * rule -- see collapse_rules.h). */
typedef struct Event {
    EventKind kind;
    ObjectId  a;
    bool      has_b;
    ObjectId  b;
    char     *tag;     /* Bind only; owned, may be "" */
    char     *reason;  /* Refuse only; owned, may be "" */
    char     *rule;    /* Collapse only; owned */
} Event;

Event event_pop(ObjectId a);
Event event_refuse(ObjectId a, const char *reason);
Event event_refuse_bind(ObjectId a, ObjectId b, const char *reason);
Event event_bind(ObjectId a, ObjectId b, const char *tag);
Event event_collapse(const char *rule);
void  event_free(Event *e); /* frees owned strings; does not free `e` itself */

/* --- Dynamic collections used by State/History --- */

typedef struct { ObjectId *items; size_t len, cap; } ObjectSet;
void   objset_init(ObjectSet *s);
void   objset_free(ObjectSet *s);
void   objset_add(ObjectSet *s, ObjectId x);       /* no-op if already present */
void   objset_remove(ObjectSet *s, ObjectId x);
bool   objset_contains(const ObjectSet *s, ObjectId x);
/* Order-independent equality against a fixed expected array. */
bool   objset_equals_array(const ObjectSet *s, const ObjectId *expected, size_t n);

typedef struct { ObjectId a, b; char *tag; } BoundFact;
typedef struct { BoundFact *items; size_t len, cap; } BoundList;
void   boundlist_init(BoundList *l);
void   boundlist_free(BoundList *l);
void   boundlist_add(BoundList *l, ObjectId a, ObjectId b, const char *tag);
bool   boundlist_contains(const BoundList *l, ObjectId a, ObjectId b, const char *tag);

typedef struct { ObjectId a; char *reason; } RefusedFact;
typedef struct { RefusedFact *items; size_t len, cap; } RefusedList;
void   refusedlist_init(RefusedList *l);
void   refusedlist_free(RefusedList *l);
void   refusedlist_add(RefusedList *l, ObjectId a, const char *reason);

typedef struct { char *rule; } ObservedFact;
typedef struct { ObservedFact *items; size_t len, cap; } ObservedList;
void   observedlist_init(ObservedList *l);
void   observedlist_free(ObservedList *l);
void   observedlist_add(ObservedList *l, const char *rule);

/* State is what a History replays to: (Omega, committed, bound, refused,
 * observed) -- see history.rs's State and run_python.py's State. */
typedef struct State {
    ObjectSet    option_space;
    ObjectSet    committed;
    BoundList    bound;
    RefusedList  refused;
    ObservedList observed;
} State;

void state_init(State *s, const ObjectSet *omega0);
void state_free(State *s);
/* The single, pure per-event transition function. */
void state_apply(State *s, const Event *e);
/* Deep value-equality (for the deterministic_replay check). */
bool state_equals(const State *a, const State *b);

/* --- Arbiter: the only path by which a History is ever extended --- */

typedef struct { char **items; size_t len, cap; } StringSet;
void strset_init(StringSet *s);
void strset_free(StringSet *s);
void strset_add(StringSet *s, const char *v);
bool strset_contains(const StringSet *s, const char *v);

typedef struct {
    ObjectSet omega0;
    StringSet rules;
    Event    *history;
    size_t    history_len, history_cap;
} Arbiter;

void   arbiter_init(Arbiter *arb, const ObjectId *omega0, size_t n_omega0,
                     const char **rules, size_t n_rules);
void   arbiter_free(Arbiter *arb);
size_t arbiter_len(const Arbiter *arb);
/* Computes fresh State by replaying the whole history from omega0. */
State  arbiter_state(const Arbiter *arb);

/* Validates the whole batch first (all-or-nothing); only appends to
 * history if every event in `events` is individually admissible. On
 * rejection, writes a human-readable error (matching the Rust
 * ArbiterError Debug format closely enough for fixture 10's
 * `expect_error` prefix check, e.g. "PopOutsideOptionSpace(1)") into
 * `err_out` (caller-owned buffer of at least 128 bytes) and returns
 * false; ownership of `events` is NOT taken on rejection (caller must
 * free them). On acceptance, the Arbiter takes ownership of the Event
 * values (moves them into its history) and returns true; the caller's
 * `events` array itself (not the Events' owned strings) may still be
 * freed by the caller. */
bool arbiter_submit(Arbiter *arb, Event *events, size_t n, char *err_out);

#endif /* SP_FIXTURES_KERNEL_H */
