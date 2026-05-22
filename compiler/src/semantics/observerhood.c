/* observerhood.c — Semantic observer implementation stub.
 * The interface is stable; the category-theoretic internals will
 * be layered on top once sheaf_semantics.c matures. */
#include "observerhood.h"
#include <stdlib.h>

static Bubble *default_collapse(Bubble *b, void *ctx) { (void)ctx; return b; }

static ConstraintView default_view(Bubble *b) {
    ConstraintView cv = {0};
    if (!b) return cv;
    cv.admissibility = b->local_admissibility;
    cv.weighted_score = b->local_admissibility;
    return cv;
}

static SemanticProjection default_project(History *h) {
    SemanticProjection sp = {0};
    sp.dim = 1;
    sp.coords = calloc(1, sizeof(double));
    sp.coords[0] = h ? h->total_action : 0.0;
    sp.entropy = 0.0;
    sp.compression_ratio = 1.0;
    return sp;
}

static AdmissibilityPoint default_evaluate(History *h) {
    AdmissibilityPoint ap = {0};
    ap.global = admissibility_global_score(h);
    ap.action = h ? h->total_action : 0.0;
    ap.strongly_admissible = (ap.global >= 0.5);
    return ap;
}

SemanticObserver *observer_create(ObserverID id) {
    SemanticObserver *obs = calloc(1, sizeof(SemanticObserver));
    obs->id          = id;
    obs->collapse_op = default_collapse;
    obs->view        = default_view;
    obs->project     = default_project;
    obs->evaluate    = default_evaluate;
    return obs;
}

SemanticObserver *observer_default(void) { return observer_create(0); }
void              observer_free(SemanticObserver *obs) { free(obs); }

ConstraintView     observer_view(const SemanticObserver *obs, Bubble *b) {
    return obs && obs->view ? obs->view(b) : (ConstraintView){0};
}
SemanticProjection observer_project(const SemanticObserver *obs, History *h) {
    return obs && obs->project ? obs->project(h) : (SemanticProjection){0};
}
AdmissibilityPoint observer_evaluate(const SemanticObserver *obs, History *h) {
    return obs && obs->evaluate ? obs->evaluate(h) : (AdmissibilityPoint){0};
}
