/* admissibility.h — Admissibility checking and manifold geometry.
 *
 * Local admissibility: type/scope/constraint correctness of a single pop.
 * Global admissibility: trajectory-level optimality over histories (§A.2–A.9).
 * These are distinct and must not be conflated.
 */
#ifndef SP_ADMISSIBILITY_H
#define SP_ADMISSIBILITY_H

#include <stdbool.h>
#include "bubble.h"

typedef struct AdmissibilityPoint {
    double   local;          /* Local constraint satisfaction in [0,1] */
    double   global;         /* Trajectory-level score */
    double   action;         /* Accumulated S[γ] */
    double   entropy;        /* Semantic entropy estimate (§12.9) */
    bool     strongly_admissible; /* §A.3 strong admissibility */
} AdmissibilityPoint;

typedef struct AdmissibilityRegion {
    double   lower_bound;
    double   upper_bound;
    Bubble  *anchor;
    int      dimension;      /* Dimensionality of local constraint space */
} AdmissibilityRegion;

/* Compute the admissibility point for a bubble prior to pop */
AdmissibilityPoint admissibility_evaluate(const Bubble *b);

/* Check strong admissibility (§A.3): local + history-coherent */
bool admissibility_strong_check(const Bubble *b, const struct History *h);

/* Compute global admissibility over a history trajectory */
double admissibility_global_score(const struct History *h);

/* Admissibility region for the current computation state */
AdmissibilityRegion admissibility_region(const Bubble *root);

/* Hamiltonians of admissibility (§12.5) */
double admissibility_hamiltonian(const Bubble *b);

#endif /* SP_ADMISSIBILITY_H */
