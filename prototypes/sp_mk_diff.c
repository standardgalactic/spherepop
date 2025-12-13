/* sp_mk_diff.c */
#include "sp_mk_diff.h"

/* Helpers */
static inline sp_handle_t rep(sp_mk_t* mk, sp_handle_t id) {
  return sp_mk_find(mk, id);
}

/* ------------------------------------------------------------
 * Apply wrappers
 * ------------------------------------------------------------ */

sp_status_t sp_mk_apply_with_diff(sp_mk_t* mk,
                                  const sp_event_hdr_t* hdr,
                                  const void* payload,
                                  const sp_mk_diff_sink_t* sink) {
  if (!mk || !hdr || !sink) return SP_EINVAL;

  const sp_eid_t seq = hdr->eid;
  sink->begin(sink->ctx, seq);

  const uint32_t op = hdr->op;
  const uint32_t len = hdr->payload_len;

  switch (op) {

    case SP_OP_POP: {
      const sp_evt_pop_t* e = (const sp_evt_pop_t*)payload;
      sp_handle_t id = e->p.a;

      sp_status_t s = sp_mk_apply(mk, hdr, payload);
      if (s != SP_OK) return s;

      const sp_mk_obj_t* o = sp_mk_get_obj(mk, id);
      sink->add_obj(sink->ctx, id, o->objtype, rep(mk, id));
      break;
    }

    case SP_OP_LINK: {
      const sp_evt_link_t* e = (const sp_evt_link_t*)payload;

      sp_status_t s = sp_mk_apply(mk, hdr, payload);
      if (s != SP_OK) return s;

      sink->add_edge(
        sink->ctx,
        rep(mk, e->p.a),
        rep(mk, e->p.b),
        e->reltype,
        e->link_flags
      );
      break;
    }

    case SP_OP_UNLINK: {
      const sp_evt_unlink_t* e = (const sp_evt_unlink_t*)payload;

      sink->rem_edge(
        sink->ctx,
        rep(mk, e->p.a),
        rep(mk, e->p.b),
        e->reltype
      );

      sp_status_t s = sp_mk_apply(mk, hdr, payload);
      if (s != SP_OK) return s;
      break;
    }

    case SP_OP_MERGE: {
      const sp_evt_merge_t* e = (const sp_evt_merge_t*)payload;

      sp_handle_t a = rep(mk, e->p.a);
      sp_handle_t b = rep(mk, e->p.b);

      sp_status_t s = sp_mk_apply(mk, hdr, payload);
      if (s != SP_OK) return s;

      sp_handle_t r = rep(mk, a);

      /* Emit representative collapse as object removal */
      if (a != r) sink->rem_obj(sink->ctx, a);
      if (b != r) sink->rem_obj(sink->ctx, b);

      /* Representative remains */
      break;
    }

    case SP_OP_COLLAPSE: {
      const sp_evt_collapse_t* e = (const sp_evt_collapse_t*)payload;
      sp_handle_t region = e->p.a;

      sink->rem_obj(sink->ctx, region);

      sp_status_t s = sp_mk_apply(mk, hdr, payload);
      if (s != SP_OK) return s;
      break;
    }

    default:
      /* Fallback: just apply */
      {
        sp_status_t s = sp_mk_apply(mk, hdr, payload);
        if (s != SP_OK) return s;
      }
      break;
  }

  sink->end(sink->ctx);
  return SP_OK;
}