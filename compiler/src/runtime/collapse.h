/* collapse.h — Collapse operators and quotient spaces (§3.4, §A.4).
 *
 * A collapse operator quotienting a history H by equivalence relation ~
 * produces H/~ — a coarsened representation that identifies histories
 * that agree on what ~ preserves. The choice of collapse operator is
 * never arbitrary; it depends on what structural invariants matter.
 */
#ifndef SP_COLLAPSE_H
#define SP_COLLAPSE_H

#include <stdbool.h>
#include "history.h"
#include "bubble.h"

typedef enum {
    COLLAPSE_TERMINAL_VALUE,    /* Identifies all histories with same result */
    COLLAPSE_EVALUATION_ORDER,  /* Identifies histories with same pop sequence */
    COLLAPSE_PROVENANCE,        /* Preserves full provenance */
    COLLAPSE_SEMANTIC,          /* Semantic equivalence class */
    COLLAPSE_CUSTOM             /* User-defined relation */
} CollapseLevel;

typedef struct CollapseOp {
    CollapseLevel level;
    bool (*equiv)(const History *a, const History *b, void *ctx);
    void *ctx;
    char *label;
} CollapseOp;

/* Apply collapse operator to a bubble: returns new (collapsed) bubble */
Bubble  *collapse_apply(Bubble *b, const CollapseOp *op);

/* Check whether two histories are identified under this operator */
bool     collapse_identifies(const CollapseOp *op,
                             const History *a, const History *b);

/* Standard collapse operators */
CollapseOp *collapse_terminal_value_op(void);
CollapseOp *collapse_evaluation_order_op(void);
CollapseOp *collapse_provenance_op(void);
CollapseOp *collapse_semantic_op(void);
void        collapse_op_free(CollapseOp *op);

#endif /* SP_COLLAPSE_H */
