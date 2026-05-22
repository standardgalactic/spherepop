/* constraints.h — Constraint sets and relational propagation.
 *
 * Constraints are relational and directional; they compose and have
 * directed propagation. A ConstraintRef indexes into a region-scoped
 * table rather than storing flat bitmasks inline. See §1.5, §3.5, §12.5.
 */
#ifndef SP_CONSTRAINTS_H
#define SP_CONSTRAINTS_H

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t ConstraintID;

typedef enum {
    CONSTRAINT_TYPE,
    CONSTRAINT_SCOPE,
    CONSTRAINT_ADMISSIBILITY,
    CONSTRAINT_BIND,
    CONSTRAINT_SEMANTIC,
    CONSTRAINT_THERMODYNAMIC
} ConstraintKind;

typedef enum {
    CPROP_INWARD,
    CPROP_OUTWARD,
    CPROP_LATERAL
} ConstraintPropDir;

typedef struct Constraint {
    ConstraintID      id;
    ConstraintKind    kind;
    ConstraintPropDir direction;
    double            weight;
    void             *data;
    bool              satisfied;
} Constraint;

typedef struct ConstraintEdge {
    ConstraintID from;
    ConstraintID to;
    double       interaction_weight;
} ConstraintEdge;

typedef struct ConstraintSet {
    Constraint   **constraints;
    int            n;
    int            cap;
    ConstraintEdge *edges;
    int            n_edges;
    int            edge_cap;
} ConstraintSet;

ConstraintSet *cset_create(void);
void           cset_free(ConstraintSet *cs);
ConstraintID   cset_add(ConstraintSet *cs, ConstraintKind kind,
                        ConstraintPropDir dir, double weight, void *data);
void           cset_add_edge(ConstraintSet *cs, ConstraintID from,
                             ConstraintID to, double weight);
bool           cset_all_satisfied(const ConstraintSet *cs);
double         cset_admissibility_score(const ConstraintSet *cs);
void           cset_propagate(ConstraintSet *dst, const ConstraintSet *src,
                              ConstraintPropDir dir);
ConstraintSet *cset_merge(const ConstraintSet *a, const ConstraintSet *b);
void           cset_print(const ConstraintSet *cs);

#endif /* SP_CONSTRAINTS_H */
