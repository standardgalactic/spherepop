/* evaluator.h — Event-driven evaluator (not recursive descent).
 *
 * The evaluator emits events rather than performing all downstream
 * actions directly. Subsystems register as listeners. This keeps
 * provenance, admissibility, and topology decoupled from evaluation
 * logic. See ARCHITECTURE.md §Event Dispatch.
 */
#ifndef SP_EVALUATOR_H
#define SP_EVALUATOR_H

#include <stdbool.h>
#include "bubble.h"
#include "history.h"
#include "constraints.h"
#include "provenance.h"

typedef enum {
    EV_POP_BEGIN,
    EV_POP_COMMIT,
    EV_REFUSE,
    EV_COLLAPSE_APPLY,
    EV_BIND,
    EV_CONSTRAINT_UPDATE,
    EV_HISTORY_APPEND,
    EV_ADMISSIBILITY_RECALC,
    EV_SCOPE_EXIT
} EventType;

typedef struct SphereEvent {
    EventType type;
    BubbleID  bubble_id;
    uint64_t  timestamp;
    void     *payload;   /* Type-specific: Value*, CollapseOp*, ConstraintSet* */
} SphereEvent;

typedef void (*EventListener)(const SphereEvent *ev, void *userdata);

typedef struct EventBus {
    struct {
        EventListener *listeners;
        void         **userdata;
        int            n;
        int            cap;
    } slots[9]; /* One slot per EventType */
    bool lazy;  /* If true, listeners defer until output queried */
} EventBus;

typedef struct EvalContext {
    Bubble          *root;
    ProvenanceStore *prov;
    History         *global_history;
    EventBus        *bus;
    int              step;
    bool             trace;   /* Print events during evaluation */
} EvalContext;

EvalContext *eval_context_create(bool trace);
void         eval_context_free(EvalContext *ctx);

void event_subscribe(EventBus *bus, EventType type,
                     EventListener listener, void *userdata);
void event_emit(EventBus *bus, const SphereEvent *ev);

/* Standard listeners registered by default */
void provenance_listener(const SphereEvent *ev, void *userdata);
void topology_listener(const SphereEvent *ev, void *userdata);
void admissibility_listener(const SphereEvent *ev, void *userdata);
void history_listener(const SphereEvent *ev, void *userdata);

/* Top-level evaluation entry point */
struct Value *eval_bubble(Bubble *b, EvalContext *ctx);
struct Value *eval_node(struct Node *node, Bubble *b, EvalContext *ctx);

#endif /* SP_EVALUATOR_H */
