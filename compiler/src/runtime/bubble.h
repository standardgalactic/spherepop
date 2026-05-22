/* bubble.h — Core bubble (admissibility region) type.
 *
 * A bubble is a topological object: a bounded region of the computation's
 * admissibility manifold with its own scope, history, and constraint
 * structure. Bubbles are popped (transformed into recorded events) — not
 * erased. See Monograph §1.2, §1.4, Appendix A.
 */
#ifndef SP_BUBBLE_H
#define SP_BUBBLE_H

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t BubbleID;
#define BUBBLE_ID_NULL UINT64_MAX

typedef enum {
    BUBBLE_ACTIVE,
    BUBBLE_POPPED,
    BUBBLE_REFUSED,
    BUBBLE_COLLAPSED,
    BUBBLE_BOUND
} BubbleState;

struct Node;
struct Value;
struct Scope;
struct ConstraintSet;
struct History;
struct EvalContext;
struct CollapseOp;

typedef struct Bubble {
    BubbleID        id;
    BubbleState     state;

    struct Bubble  *parent;
    struct Bubble **children;
    int             n_children;
    int             children_cap;

    struct Node    *expression;
    struct Value   *result;

    struct Scope         *scope;
    struct ConstraintSet *constraints;
    struct History       *history;
    uint64_t              prov_id;

    double   local_admissibility;  /* [0,1] local constraint satisfaction */
    double   action_cost;          /* S[γ] = Σ Lagrangian cost, §12.3 */
    uint32_t boundary_flags;
    int      depth;
} Bubble;

Bubble *bubble_alloc(Bubble *parent, struct Scope *scope);
void    bubble_free(Bubble *b);
void    bubble_add_child(Bubble *parent, Bubble *child);
void    bubble_remove_child(Bubble *parent, Bubble *child);

bool    bubble_is_innermost(const Bubble *b);
bool    bubble_is_admissible(const Bubble *b);
bool    bubble_constraints_satisfied(const Bubble *b);

struct Value *bubble_pop(Bubble *b, struct EvalContext *ctx);
bool          bubble_refuse(Bubble *b, struct EvalContext *ctx);
Bubble       *bubble_collapse(Bubble *b, struct CollapseOp *op);
void          bubble_bind(Bubble *b, struct ConstraintSet *cs);

void   bubble_recalc_admissibility(Bubble *b);
double bubble_global_admissibility(const Bubble *root);
void   bubble_print(const Bubble *b, int indent);
BubbleID bubble_next_id(void);

#endif /* SP_BUBBLE_H */
