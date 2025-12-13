/* spherepop.c */
#include "spherepop_user.h"

/* Local: safe min */
static inline uint32_t sp_u32_min(uint32_t a, uint32_t b) { return (a < b) ? a : b; }

sp_status_t sp_meta_pack(const sp_meta_view_t* mv, void* out_buf, uint32_t out_cap, uint32_t* out_total) {
  if (!mv || !out_buf || !out_total) return SP_EINVAL;
  if (mv->len > 0 && mv->payload == NULL) return SP_EINVAL;

  uint32_t need = (uint32_t)sizeof(sp_meta_blob_t) + mv->len;
  if (need < (uint32_t)sizeof(sp_meta_blob_t)) return SP_EOVERFLOW;
  if (out_cap < need) return SP_EOVERFLOW;

  sp_meta_blob_t* mb = (sp_meta_blob_t*)out_buf;
  mb->type = mv->type;
  mb->len  = mv->len;

  if (mv->len) {
    uint8_t* dst = (uint8_t*)out_buf + sizeof(sp_meta_blob_t);
    const uint8_t* src = (const uint8_t*)mv->payload;
    for (uint32_t i = 0; i < mv->len; i++) dst[i] = src[i];
  }

  *out_total = need;
  return SP_OK;
}

/* Small fixed buffer for meta packing (per-call). If you want zero stack usage,
 * switch these to caller-provided buffers. */
#define SP_META_SCRATCH_MAX (1024u)

sp_status_t sp_pop_obj(const sp_pop_opts_t* opts, sp_handle_t* out_obj, sp_eid_t* out_eid) {
  if (!opts || !out_obj) return SP_EINVAL;

  uint8_t meta_scratch[SP_META_SCRATCH_MAX];
  const sp_meta_blob_t* meta_ptr = NULL;
  uint32_t meta_total = 0;

  if (opts->meta.len) {
    if (sizeof(meta_scratch) < sizeof(sp_meta_blob_t) + opts->meta.len) return SP_EOVERFLOW;
    sp_status_t s = sp_meta_pack(&opts->meta, meta_scratch, (uint32_t)sizeof(meta_scratch), &meta_total);
    if (s != SP_OK) return s;
    meta_ptr = (const sp_meta_blob_t*)meta_scratch;
  }

  sp_pop_req_t req = {0};
  sp_pop_rsp_t rsp = {0};

  req.h.op = SP_OP_POP;
  req.h.flags = opts->flags;
  req.h.user_tag = 0;

  req.objtype = opts->objtype;
  req.parent = opts->parent;
  req.reltype_override = opts->reltype_override;
  req.link_flags = opts->link_flags;
  req.meta = meta_ptr;

  sp_status_t call = sp_syscall(&req, sizeof(req), &rsp, sizeof(rsp));
  if (call != SP_OK) return call;
  if (rsp.h.status != SP_OK) return rsp.h.status;

  *out_obj = rsp.obj;
  if (out_eid) *out_eid = rsp.h.eid;
  return SP_OK;
}

sp_status_t sp_link_edge(const sp_link_opts_t* opts, sp_eid_t* out_eid) {
  if (!opts) return SP_EINVAL;
  if (opts->src == 0 || opts->dst == 0) return SP_EINVAL;

  uint8_t meta_scratch[SP_META_SCRATCH_MAX];
  const sp_meta_blob_t* meta_ptr = NULL;
  uint32_t meta_total = 0;

  if (opts->meta.len) {
    if (sizeof(meta_scratch) < sizeof(sp_meta_blob_t) + opts->meta.len) return SP_EOVERFLOW;
    sp_status_t s = sp_meta_pack(&opts->meta, meta_scratch, (uint32_t)sizeof(meta_scratch), &meta_total);
    if (s != SP_OK) return s;
    meta_ptr = (const sp_meta_blob_t*)meta_scratch;
  }

  sp_link_req_t req = {0};
  sp_link_rsp_t rsp = {0};

  req.h.op = SP_OP_LINK;
  req.h.flags = opts->flags;
  req.h.user_tag = 0;

  req.src = opts->src;
  req.dst = opts->dst;
  req.reltype = opts->reltype;
  req.link_flags = opts->link_flags;
  req.meta = meta_ptr;
  req.order = opts->order;

  sp_status_t call = sp_syscall(&req, sizeof(req), &rsp, sizeof(rsp));
  if (call != SP_OK) return call;
  if (rsp.h.status != SP_OK) return rsp.h.status;

  if (out_eid) *out_eid = rsp.h.eid;
  return SP_OK;
}

sp_status_t sp_unlink_edge(sp_handle_t src, sp_handle_t dst, sp_reltype_t reltype, uint32_t flags, sp_eid_t* out_eid) {
  if (!src || !dst) return SP_EINVAL;

  sp_unlink_req_t req = {0};
  sp_unlink_rsp_t rsp = {0};

  req.h.op = SP_OP_UNLINK;
  req.h.flags = flags;
  req.h.user_tag = 0;

  req.src = src;
  req.dst = dst;
  req.reltype = reltype;

  sp_status_t call = sp_syscall(&req, sizeof(req), &rsp, sizeof(rsp));
  if (call != SP_OK) return call;
  if (rsp.h.status != SP_OK) return rsp.h.status;

  if (out_eid) *out_eid = rsp.h.eid;
  return SP_OK;
}

sp_status_t sp_merge_objs(const sp_merge_opts_t* opts, sp_handle_t* out_result, sp_eid_t* out_eid) {
  if (!opts || !out_result) return SP_EINVAL;
  if (opts->a == 0 || opts->b == 0) return SP_EINVAL;

  uint8_t meta_scratch[SP_META_SCRATCH_MAX];
  const sp_meta_blob_t* policy_ptr = NULL;
  uint32_t meta_total = 0;

  if (opts->policy.len) {
    if (sizeof(meta_scratch) < sizeof(sp_meta_blob_t) + opts->policy.len) return SP_EOVERFLOW;
    sp_status_t s = sp_meta_pack(&opts->policy, meta_scratch, (uint32_t)sizeof(meta_scratch), &meta_total);
    if (s != SP_OK) return s;
    policy_ptr = (const sp_meta_blob_t*)meta_scratch;
  }

  sp_merge_req_t req = {0};
  sp_merge_rsp_t rsp = {0};

  req.h.op = SP_OP_MERGE;
  req.h.flags = opts->flags;
  req.h.user_tag = 0;

  req.a = opts->a;
  req.b = opts->b;
  req.mode = opts->mode;
  req.into = opts->into;
  req.policy = policy_ptr;

  sp_status_t call = sp_syscall(&req, sizeof(req), &rsp, sizeof(rsp));
  if (call != SP_OK) return call;
  if (rsp.h.status != SP_OK) return rsp.h.status;

  *out_result = rsp.result;
  if (out_eid) *out_eid = rsp.h.eid;
  return SP_OK;
}

sp_status_t sp_split_obj(const sp_split_opts_t* opts, sp_handle_t* out_region, uint32_t* out_count, sp_eid_t* out_eid) {
  if (!opts || !out_region) return SP_EINVAL;
  if (opts->obj == 0) return SP_EINVAL;

  uint8_t meta_scratch[SP_META_SCRATCH_MAX];
  const sp_meta_blob_t* params_ptr = NULL;
  uint32_t meta_total = 0;

  if (opts->params.len) {
    if (sizeof(meta_scratch) < sizeof(sp_meta_blob_t) + opts->params.len) return SP_EOVERFLOW;
    sp_status_t s = sp_meta_pack(&opts->params, meta_scratch, (uint32_t)sizeof(meta_scratch), &meta_total);
    if (s != SP_OK) return s;
    params_ptr = (const sp_meta_blob_t*)meta_scratch;
  }

  sp_split_req_t req = {0};
  sp_split_rsp_t rsp = {0};

  req.h.op = SP_OP_SPLIT;
  req.h.flags = opts->flags;
  req.h.user_tag = 0;

  req.obj = opts->obj;
  req.mode = opts->mode;
  req.reltype = opts->reltype;
  req.meta_type = opts->meta_type;
  req.order = opts->order;
  req.params = params_ptr;

  sp_status_t call = sp_syscall(&req, sizeof(req), &rsp, sizeof(rsp));
  if (call != SP_OK) return call;
  if (rsp.h.status != SP_OK) return rsp.h.status;

  *out_region = rsp.region;
  if (out_count) *out_count = rsp.count;
  if (out_eid) *out_eid = rsp.h.eid;
  return SP_OK;
}

sp_status_t sp_collapse_region(const sp_collapse_opts_t* opts, sp_handle_t* out_rep, sp_eid_t* out_eid) {
  if (!opts || !out_rep) return SP_EINVAL;
  if (opts->region == 0) return SP_EINVAL;

  uint8_t meta_scratch[SP_META_SCRATCH_MAX];
  const sp_meta_blob_t* policy_ptr = NULL;
  uint32_t meta_total = 0;

  if (opts->policy.len) {
    if (sizeof(meta_scratch) < sizeof(sp_meta_blob_t) + opts->policy.len) return SP_EOVERFLOW;
    sp_status_t s = sp_meta_pack(&opts->policy, meta_scratch, (uint32_t)sizeof(meta_scratch), &meta_total);
    if (s != SP_OK) return s;
    policy_ptr = (const sp_meta_blob_t*)meta_scratch;
  }

  sp_collapse_req_t req = {0};
  sp_collapse_rsp_t rsp = {0};

  req.h.op = SP_OP_COLLAPSE;
  req.h.flags = opts->flags;
  req.h.user_tag = 0;

  req.region = opts->region;
  req.collapse_flags = opts->collapse_flags;
  req.policy = policy_ptr;

  sp_status_t call = sp_syscall(&req, sizeof(req), &rsp, sizeof(rsp));
  if (call != SP_OK) return call;
  if (rsp.h.status != SP_OK) return rsp.h.status;

  *out_rep = rsp.representative;
  if (out_eid) *out_eid = rsp.h.eid;
  return SP_OK;
}

sp_status_t sp_read_event(sp_handle_t eventlog, sp_eid_t eid,
                          sp_event_hdr_t* out_hdr,
                          void* out_payload, uint32_t out_cap, uint32_t* out_len,
                          sp_status_t* out_op_status) {
  if (!out_hdr || !out_len) return SP_EINVAL;

  sp_read_event_req_t req = {0};
  sp_read_event_rsp_t rsp = {0};

  req.h.op = SP_OP_READ_EVENT;
  req.h.flags = SP_F_NONE;
  req.h.user_tag = 0;

  req.eventlog = eventlog;
  req.eid = eid;
  req.out_hdr = out_hdr;
  req.out_payload = out_payload;
  req.out_cap = out_cap;
  req.out_len = out_len;

  sp_status_t call = sp_syscall(&req, sizeof(req), &rsp, sizeof(rsp));
  if (call != SP_OK) return call;
  if (out_op_status) *out_op_status = rsp.h.status;
  return rsp.h.status;
}