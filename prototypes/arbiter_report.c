#include "arbiter_report.h"

int arb_rpt_write(FILE* f, const arb_rpt_hdr_t* h, const arb_rpt_entry_t* e) {
  if (!f || !h || !e) return -1;
  if (fwrite(h, 1, sizeof(*h), f) != sizeof(*h)) return -1;
  if (h->n_entries) {
    if (fwrite(e, 1, sizeof(*e) * h->n_entries, f) != sizeof(*e) * h->n_entries) return -1;
  }
  return 0;
}