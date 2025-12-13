#ifndef ARBITER_REPORT_H
#define ARBITER_REPORT_H

#include <stdint.h>
#include <stdio.h>

#define ARB_RPT_MAGIC 0x52504241u /* 'ABPR' */

typedef struct arb_rpt_hdr {
  uint32_t magic;
  uint32_t version;     /* 1 */
  uint32_t n_entries;
  uint32_t reserved;
} arb_rpt_hdr_t;

typedef struct arb_rpt_entry {
  uint32_t accepted;    /* 1 accepted, 0 rejected */
  uint32_t op;
  int32_t  err;         /* 0 if accepted, else error code */
  uint32_t weight;
  uint32_t authority_hash;
  uint64_t eid_assigned; /* after commit ordering */
  uint64_t a, b, c;      /* best-effort extracted handles */
  int32_t  d_objs;       /* after-before */
  int32_t  d_rels;
  int32_t  d_reps;       /* optional; 0 if unknown */
  uint32_t payload_len;
} arb_rpt_entry_t;

int arb_rpt_write(FILE* f, const arb_rpt_hdr_t* h, const arb_rpt_entry_t* e);

#endif