/* spherepop_user.h
 * Userspace convenience layer for Spherepop ABI.
 */
#ifndef SPHEREPOP_USER_H
#define SPHEREPOP_USER_H

#include "spherepop_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------
 * Metadata helpers
 * ------------------------------ */

/* A stack-friendly metadata view: header + pointer to payload bytes. */
typedef struct sp_meta_view {
  uint32_t type;
  uint32_t len;
  const void* payload;
} sp_meta_view_t;

/* Build a meta_view from raw payload (payload may be NULL if len==0). */
static inline sp_meta_view_t sp_meta_make(uint32_t type, const void* payload, uint32_t len) {
  sp_meta_view_t mv;
  mv.type = type;
  mv.len = len;
  mv.payload = payload;
  return mv;
}

/* Pack meta_view into a contiguous buffer as sp_meta_blob_t + payload.
 * Returns SP_OK, and writes total bytes to *out_total.
 * Buffer must be at least sizeof(sp_meta_blob_t)+len.
 */
sp_status_t sp_meta_pack(const sp_meta_view_t* mv, void* out_buf, uint32_t out_cap, uint32_t* out_total);

/* ------------------------------
 * Common op wrappers
 * ------------------------------ */

typedef struct sp_pop_opts {
  sp_objtype_t objtype;
  sp_handle_t parent;          /* 0 for none */
  sp_reltype_t reltype_override;/* 0 for default contains */
  uint32_t link_flags;         /* SP_LF_* */
  sp_meta_view_t meta;         /* optional; meta.len==0 => none */
  uint32_t flags;              /* SP_F_* */
} sp_pop_opts_t;

sp_status_t sp_pop_obj(const sp_pop_opts_t* opts, sp_handle_t* out_obj, sp_eid_t* out_eid);

typedef struct sp_link_opts {
  sp_handle_t src;
  sp_handle_t dst;
  sp_reltype_t reltype;
  uint32_t link_flags;   /* SP_LF_* */
  sp_handle_t order;     /* 0 unless SP_LF_CAUSAL */
  sp_meta_view_t meta;   /* optional */
  uint32_t flags;        /* SP_F_* */
} sp_link_opts_t;

sp_status_t sp_link_edge(const sp_link_opts_t* opts, sp_eid_t* out_eid);

sp_status_t sp_unlink_edge(sp_handle_t src, sp_handle_t dst, sp_reltype_t reltype, uint32_t flags, sp_eid_t* out_eid);

typedef struct sp_merge_opts {
  sp_handle_t a;
  sp_handle_t b;
  uint32_t mode;       /* SP_MG_* */
  sp_handle_t into;    /* 0 = kernel default */
  sp_meta_view_t policy; /* optional */
  uint32_t flags;      /* SP_F_* */
} sp_merge_opts_t;

sp_status_t sp_merge_objs(const sp_merge_opts_t* opts, sp_handle_t* out_result, sp_eid_t* out_eid);

typedef struct sp_split_opts {
  sp_handle_t obj;
  uint32_t mode;          /* SP_SP_* */
  sp_reltype_t reltype;   /* for BY_RELTYPE */
  uint32_t meta_type;     /* for BY_META_TYPE */
  sp_handle_t order;      /* for BY_ORDER */
  sp_meta_view_t params;  /* optional */
  uint32_t flags;         /* SP_F_* */
} sp_split_opts_t;

sp_status_t sp_split_obj(const sp_split_opts_t* opts, sp_handle_t* out_region, uint32_t* out_count, sp_eid_t* out_eid);

typedef struct sp_collapse_opts {
  sp_handle_t region;
  uint32_t collapse_flags; /* SP_CF_* */
  sp_meta_view_t policy;   /* optional */
  uint32_t flags;          /* SP_F_* */
} sp_collapse_opts_t;

sp_status_t sp_collapse_region(const sp_collapse_opts_t* opts, sp_handle_t* out_rep, sp_eid_t* out_eid);

/* ------------------------------
 * Event log read convenience
 * ------------------------------ */

sp_status_t sp_read_event(sp_handle_t eventlog, sp_eid_t eid,
                          sp_event_hdr_t* out_hdr,
                          void* out_payload, uint32_t out_cap, uint32_t* out_len,
                          sp_status_t* out_op_status);

#ifdef __cplusplus
}
#endif

#endif /* SPHEREPOP_USER_H */