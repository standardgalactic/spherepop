#define _POSIX_C_SOURCE 200809L
#define _POSIX_C_SOURCE 200809L
/* provenance.c — Global provenance store. */
#include "provenance.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ProvenanceStore *prov_store_create(void) {
    ProvenanceStore *ps = calloc(1, sizeof(ProvenanceStore));
    ps->cap     = 64;
    ps->next_id = 1;
    ps->records = malloc(ps->cap * sizeof(ProvenanceRecord *));
    return ps;
}

void prov_store_free(ProvenanceStore *ps) {
    if (!ps) return;
    for (int i = 0; i < ps->n; i++) {
        free(ps->records[i]->operation_label);
        free(ps->records[i]);
    }
    free(ps->records);
    free(ps);
}

ProvenanceID prov_record(ProvenanceStore *ps, uint64_t bubble_id,
                         ProvenanceID parent, int step,
                         double admissibility, double action_delta,
                         const char *op_label, void *value_snap) {
    if (ps->n == ps->cap) {
        ps->cap *= 2;
        ps->records = realloc(ps->records, ps->cap * sizeof(ProvenanceRecord *));
    }
    ProvenanceRecord *r = calloc(1, sizeof(ProvenanceRecord));
    r->id              = ps->next_id++;
    r->bubble_id       = bubble_id;
    r->parent_prov     = parent;
    r->step            = step;
    r->admissibility   = admissibility;
    r->action_delta    = action_delta;
    r->operation_label = op_label ? strdup(op_label) : NULL;
    r->value_snapshot  = value_snap;
    ps->records[ps->n++] = r;
    return r->id;
}

ProvenanceRecord *prov_lookup(const ProvenanceStore *ps, ProvenanceID id) {
    for (int i = 0; i < ps->n; i++)
        if (ps->records[i]->id == id) return ps->records[i];
    return NULL;
}

void prov_print_chain(const ProvenanceStore *ps, ProvenanceID id) {
    ProvenanceRecord *r = prov_lookup(ps, id);
    if (!r) { printf("<prov:%llu not found>\n", (unsigned long long)id); return; }
    if (r->parent_prov != PROV_ID_NULL)
        prov_print_chain(ps, r->parent_prov);
    printf("  prov[%llu] bubble=%llu step=%d adm=%.3f dS=%.3f op=%s\n",
           (unsigned long long)r->id,
           (unsigned long long)r->bubble_id,
           r->step, r->admissibility, r->action_delta,
           r->operation_label ? r->operation_label : "(none)");
}

bool prov_verify_chain(const ProvenanceStore *ps, ProvenanceID id) {
    ProvenanceRecord *r = prov_lookup(ps, id);
    if (!r) return false;
    if (r->parent_prov == PROV_ID_NULL) return true;
    return prov_verify_chain(ps, r->parent_prov);
}
