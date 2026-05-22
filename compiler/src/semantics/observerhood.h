/* observerhood.h — Semantic observer and CLIO projection (§E).
 *
 * An observer is a stabilized compression of a trajectory. The interface
 * is established now so the runtime does not harden around assumptions
 * that make semantic extensibility impossible later. See §E.14.
 *
 * AdmissibilityPoint is returned by evaluate(), not a scalar double,
 * because admissibility is a manifold position, not a score. Returning
 * a scalar collapses the geometry immediately — the exact failure mode
 * the monograph documents. See design discussion in ARCHITECTURE.md.
 */
#ifndef SP_OBSERVERHOOD_H
#define SP_OBSERVERHOOD_H

#include <stdint.h>
#include "../runtime/admissibility.h"
#include "../runtime/history.h"
#include "../runtime/bubble.h"

typedef uint64_t ObserverID;

typedef struct SemanticProjection {
    double *coords;     /* Position in semantic configuration space */
    int     dim;
    double  entropy;
    double  compression_ratio;
} SemanticProjection;

typedef struct ConstraintView {
    double  admissibility;
    int     n_active;
    int     n_violated;
    double  weighted_score;
} ConstraintView;

typedef struct SemanticObserver {
    ObserverID id;

    /* Collapse operator: apply a quotient to a bubble */
    Bubble *(*collapse_op)(Bubble *b, void *ctx);

    /* View the constraint structure of a bubble */
    ConstraintView (*view)(Bubble *b);

    /* Project a history onto the semantic configuration space */
    SemanticProjection (*project)(History *h);

    /* Evaluate a trajectory: returns AdmissibilityPoint, not scalar */
    AdmissibilityPoint (*evaluate)(History *h);

    void *ctx;
} SemanticObserver;

SemanticObserver *observer_create(ObserverID id);
void              observer_free(SemanticObserver *obs);
SemanticObserver *observer_default(void);

ConstraintView     observer_view(const SemanticObserver *obs, Bubble *b);
SemanticProjection observer_project(const SemanticObserver *obs, History *h);
AdmissibilityPoint observer_evaluate(const SemanticObserver *obs, History *h);

#endif /* SP_OBSERVERHOOD_H */
