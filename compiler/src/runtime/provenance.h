/* provenance.h — Global provenance store.
 *
 * Provenance is the full record of how a result was reached. Two
 * computations with identical results may differ in provenance; that
 * difference is semantically significant under collapse operators that
 * preserve more than terminal values. See §1.3, §21.2.
 */
#ifndef SP_PROVENANCE_H
#define SP_PROVENANCE_H

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t ProvenanceID;
#define PROV_ID_NULL UINT64_MAX

typedef struct ProvenanceRecord {
    ProvenanceID     id;
    uint64_t         bubble_id;
    ProvenanceID     parent_prov;
    int              step;
    double           admissibility;
    double           action_delta;
    char            *operation_label;
    void            *value_snapshot;
    struct ProvenanceRecord *next;
} ProvenanceRecord;

typedef struct ProvenanceStore {
    ProvenanceRecord **records;
    int               n;
    int               cap;
    ProvenanceID      next_id;
} ProvenanceStore;

ProvenanceStore  *prov_store_create(void);
void              prov_store_free(ProvenanceStore *ps);
ProvenanceID      prov_record(ProvenanceStore *ps, uint64_t bubble_id,
                              ProvenanceID parent, int step,
                              double admissibility, double action_delta,
                              const char *op_label, void *value_snap);
ProvenanceRecord *prov_lookup(const ProvenanceStore *ps, ProvenanceID id);
void              prov_print_chain(const ProvenanceStore *ps, ProvenanceID id);
bool              prov_verify_chain(const ProvenanceStore *ps, ProvenanceID id);

#endif /* SP_PROVENANCE_H */
