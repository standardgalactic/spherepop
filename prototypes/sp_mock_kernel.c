/* sp_mock_kernel.c */
#include "sp_mock_kernel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------------------------
 * Internal helpers
 * --------------------------- */

static sp_status_t ensure_objs(sp_mk_t* mk, uint32_t need) {
  if (mk->objs_cap >= need) return SP_OK;
  uint32_t newcap = mk->objs_cap ? mk->objs_cap : 64;
  while (newcap < need) newcap *= 2;
  sp_mk_obj_t* p = (sp_mk_obj_t*)realloc(mk->objs, newcap * sizeof(*mk->objs));
  if (!p) return SP_ENOMEM;
  mk->objs = p;
  mk->objs_cap = newcap;
  return SP_OK;
}

static sp_status_t ensure_edges(sp_mk_t* mk, uint32_t need) {
  if (mk->edges_cap >= need) return SP_OK;
  uint32_t newcap = mk->edges_cap ? mk->edges_cap : 128;
  while (newcap < need) newcap *= 2;
  sp_mk_edge_t* p = (sp_mk_edge_t*)realloc(mk->edges, newcap * sizeof(*mk->edges));
  if (!p) return SP_ENOMEM;
  mk->edges = p;
  mk->edges_cap = newcap;
  return SP_OK;
}

static sp_status_t meta_alloc(sp_mk_t* mk, const void* bytes, uint32_t len,
                             uint32_t type, uint32_t* out_off) {
  if (len == 0) { *out_off = 0; return SP_OK; }
  if (!bytes) return SP_EINVAL;
  if (mk->meta_len + len > mk->meta_cap) return SP_EOVERFLOW;
  uint32_t off = mk->meta_len;
  memcpy(mk->meta_pool + off, bytes, len);
  mk->meta_len += len;
  *out_off = off;
  (void)type;
  return SP_OK;
}

static sp_mk_obj_t* get_or_create_obj(sp_mk_t* mk, sp_handle_t id) {
  for (uint32_t i = 0; i < mk->objs_len; i++) {
    if (mk->objs[i].id == id) return &mk->objs[i];
  }
  if (ensure_objs(mk, mk->objs_len + 1) != SP_OK) return NULL;
  sp_mk_obj_t* o = &mk->objs[mk->objs_len++];
  memset(o, 0, sizeof(*o));
  o->id = id;
  o->objtype = SP_OT_GENERIC;
  o->parent = id; /* union-find */
  o->rank = 0;
  return o;
}

static sp_mk_obj_t* get_obj_mut(sp_mk_t* mk, sp_handle_t id) {
  for (uint32_t i = 0; i < mk->objs_len; i++) if (mk->objs[i].id == id) return &mk->objs[i];
  return NULL;
}

const sp_mk_obj_t* sp_mk_get_obj(const sp_mk_t* mk, sp_handle_t id) {
  for (uint32_t i = 0; i < mk->objs_len; i++) if (mk->objs[i].id == id) return &mk->objs[i];
  return NULL;
}

/* Union-find find with path compression. If unknown, returns id. */
sp_handle_t sp_mk_find(sp_mk_t* mk, sp_handle_t id) {
  sp_mk_obj_t* o = get_obj_mut(mk, id);
  if (!o) return id;
  if (o->parent == o->id) return o->id;
  o->parent = sp_mk_find(mk, o->parent);
  return o->parent;
}

static void uf_union(sp_mk_t* mk, sp_handle_t a, sp_handle_t b) {
  a = sp_mk_find(mk, a);
  b = sp_mk_find(mk, b);
  if (a == b) return;
  sp_mk_obj_t* oa = get_obj_mut(mk, a);
  sp_mk_obj_t* ob = get_obj_mut(mk, b);
  if (!oa || !ob) return;
  if (oa->rank < ob->rank) {
    oa->parent = b;
  } else if (oa->rank > ob->rank) {
    ob->parent = a;
  } else {
    ob->parent = a;
    oa->rank++;
  }
}

/* Remove edge by exact match (first occurrence). */
static sp_status_t remove_edge(sp_mk_t* mk, sp_handle_t src, sp_handle_t dst, sp_reltype_t reltype) {
  for (uint32_t i = 0; i < mk->edges_len; i++) {
    sp_mk_edge_t* e = &mk->edges[i];
    if (e->src == src && e->dst == dst && e->reltype == reltype) {
      mk->edges[i] = mk->edges[mk->edges_len - 1];
      mk->edges_len--;
      return SP_OK;
    }
  }
  return SP_ENOENT;
}

/* Add edge */
static sp_status_t add_edge(sp_mk_t* mk, const sp_mk_edge_t* edge) {
  sp_status_t s = ensure_edges(mk, mk->edges_len + 1);
  if (s != SP_OK) return s;
  mk->edges[mk->edges_len++] = *edge;
  return SP_OK;
}

/* Extract inline blob bytes from payload using blob_ref (offset relative to payload base). */
static sp_status_t blob_bytes(const void* payload, uint32_t payload_len,
                              const sp_evt_blob_ref_t* ref,
                              const void** out_ptr, uint32_t* out_len, uint32_t* out_type) {
  if (ref->len == 0) { *out_ptr = NULL; *out_len = 0; *out_type = ref->type; return SP_OK; }
  if (ref->off > payload_len) return SP_EINVAL;
  if (ref->off + ref->len > payload_len) return SP_EOVERFLOW;
  *out_ptr = (const uint8_t*)payload + ref->off;
  *out_len = ref->len;
  *out_type = ref->type;
  return SP_OK;
}

/* ---------------------------
 * Public lifecycle
 * --------------------------- */
sp_status_t sp_mk_init(sp_mk_t* mk, uint32_t obj_cap_hint, uint32_t edge_cap_hint) {
  if (!mk) return SP_EINVAL;
  memset(mk, 0, sizeof(*mk));

  mk->objs_cap = obj_cap_hint ? obj_cap_hint : 256;
  mk->edges_cap = edge_cap_hint ? edge_cap_hint : 512;

  mk->objs = (sp_mk_obj_t*)calloc(mk->objs_cap, sizeof(*mk->objs));
  mk->edges = (sp_mk_edge_t*)calloc(mk->edges_cap, sizeof(*mk->edges));
  mk->meta_cap = SP_MK_MAX_META_BYTES;
  mk->meta_pool = (uint8_t*)malloc(mk->meta_cap);

  if (!mk->objs || !mk->edges || !mk->meta_pool) {
    sp_mk_free(mk);
    return SP_ENOMEM;
  }
  return SP_OK;
}

void sp_mk_free(sp_mk_t* mk) {
  if (!mk) return;
  free(mk->objs);
  free(mk->edges);
  free(mk->meta_pool);
  memset(mk, 0, sizeof(*mk));
}

/* ---------------------------
 * Apply events
 * --------------------------- */
static sp_status_t apply_pop(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  if (payload_len < sizeof(sp_evt_pop_t)) return SP_EINVAL;
  const sp_evt_pop_t* e = (const sp_evt_pop_t*)payload;

  sp_handle_t new_id = e->p.a;
  sp_handle_t parent = e->p.b;

  sp_mk_obj_t* o = get_or_create_obj(mk, new_id);
  if (!o) return SP_ENOMEM;
  o->objtype = (sp_objtype_t)e->objtype;

  /* Optional object meta */
  if (e->has_meta) {
    const void* p; uint32_t len, type;
    sp_status_t s = blob_bytes(payload, payload_len, &e->meta, &p, &len, &type);
    if (s != SP_OK) return s;
    uint32_t off = 0;
    s = meta_alloc(mk, p, len, type, &off);
    if (s != SP_OK) return s;
    o->meta_off = off;
    o->meta_len = len;
    o->meta_type = type;
  }

  /* Implicit contains edge parent -> new if parent!=0 */
  if (parent != 0) {
    get_or_create_obj(mk, parent);
    sp_mk_edge_t ed = {0};
    ed.src = parent;
    ed.dst = new_id;
    ed.reltype = (e->reltype_override ? (sp_reltype_t)e->reltype_override : SP_RT_CONTAINS);
    ed.link_flags = e->link_flags;
    return add_edge(mk, &ed);
  }
  return SP_OK;
}

static sp_status_t apply_link(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  if (payload_len < sizeof(sp_evt_link_t)) return SP_EINVAL;
  const sp_evt_link_t* e = (const sp_evt_link_t*)payload;

  get_or_create_obj(mk, e->p.a);
  get_or_create_obj(mk, e->p.b);

  sp_mk_edge_t ed = {0};
  ed.src = e->p.a;
  ed.dst = e->p.b;
  ed.reltype = (sp_reltype_t)e->reltype;
  ed.link_flags = e->link_flags;

  if (e->has_meta) {
    const void* p; uint32_t len, type;
    sp_status_t s = blob_bytes(payload, payload_len, &e->meta, &p, &len, &type);
    if (s != SP_OK) return s;
    uint32_t off = 0;
    s = meta_alloc(mk, p, len, type, &off);
    if (s != SP_OK) return s;
    ed.meta_off = off;
    ed.meta_len = len;
    ed.meta_type = type;
  }
  return add_edge(mk, &ed);
}

static sp_status_t apply_unlink(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  if (payload_len < sizeof(sp_evt_unlink_t)) return SP_EINVAL;
  const sp_evt_unlink_t* e = (const sp_evt_unlink_t*)payload;
  return remove_edge(mk, e->p.a, e->p.b, (sp_reltype_t)e->reltype);
}

static sp_status_t apply_merge(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  if (payload_len < sizeof(sp_evt_merge_t)) return SP_EINVAL;
  const sp_evt_merge_t* e = (const sp_evt_merge_t*)payload;

  get_or_create_obj(mk, e->p.a);
  get_or_create_obj(mk, e->p.b);

  /* Union-find unify */
  uf_union(mk, e->p.a, e->p.b);

  /* Optional: policy ignored by mock kernel (stored nowhere). */
  (void)payload_len;
  return SP_OK;
}

static sp_status_t apply_split(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  if (payload_len < sizeof(sp_evt_split_t)) return SP_EINVAL;
  const sp_evt_split_t* e = (const sp_evt_split_t*)payload;

  sp_handle_t obj = e->p.a;
  sp_handle_t region = e->p.b; /* kernel-filled in events */
  if (region == 0) return SP_EINVAL;

  get_or_create_obj(mk, obj);
  sp_mk_obj_t* r = get_or_create_obj(mk, region);
  if (!r) return SP_ENOMEM;
  r->objtype = SP_OT_REGION;
  r->region_root = obj;

  /* In the mock: create a contains edge region -> obj to indicate membership */
  sp_mk_edge_t ed = {0};
  ed.src = region;
  ed.dst = obj;
  ed.reltype = SP_RT_CONTAINS;
  ed.link_flags = SP_LF_PINNED;
  return add_edge(mk, &ed);
}

static sp_status_t apply_collapse(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  if (payload_len < sizeof(sp_evt_collapse_t)) return SP_EINVAL;
  const sp_evt_collapse_t* e = (const sp_evt_collapse_t*)payload;

  /* Region collapses to representative handle e->p.b (kernel-filled). In mock, we:
   *  - union region with rep
   *  - optionally prune SOFT edges incident to region if PRUNE_SOFT flag set
   */
  get_or_create_obj(mk, e->p.a);
  if (e->p.b) get_or_create_obj(mk, e->p.b);

  if (e->p.b) uf_union(mk, e->p.a, e->p.b);

  if (e->collapse_flags & SP_CF_PRUNE_SOFT) {
    sp_handle_t region = e->p.a;
    for (uint32_t i = 0; i < mk->edges_len; ) {
      sp_mk_edge_t* ed = &mk->edges[i];
      int incident = (ed->src == region || ed->dst == region);
      int soft = (ed->link_flags & SP_LF_SOFT) && !(ed->link_flags & SP_LF_PINNED);
      if (incident && soft) {
        mk->edges[i] = mk->edges[mk->edges_len - 1];
        mk->edges_len--;
        continue;
      }
      i++;
    }
  }
  (void)payload_len;
  return SP_OK;
}

static sp_status_t apply_set_meta(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  if (payload_len < sizeof(sp_evt_set_meta_t)) return SP_EINVAL;
  const sp_evt_set_meta_t* e = (const sp_evt_set_meta_t*)payload;

  sp_mk_obj_t* o = get_or_create_obj(mk, e->p.a);
  if (!o) return SP_ENOMEM;

  if (e->has_meta) {
    const void* p; uint32_t len, type;
    sp_status_t s = blob_bytes(payload, payload_len, &e->meta, &p, &len, &type);
    if (s != SP_OK) return s;
    uint32_t off = 0;
    s = meta_alloc(mk, p, len, type, &off);
    if (s != SP_OK) return s;
    o->meta_off = off;
    o->meta_len = len;
    o->meta_type = type;
  } else {
    o->meta_off = 0; o->meta_len = 0; o->meta_type = 0;
  }
  return SP_OK;
}

/* For order objects, mock kernel just stores objects; constraints ignored for now. */
static sp_status_t apply_order_create(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  if (payload_len < sizeof(sp_evt_order_create_t)) return SP_EINVAL;
  const sp_evt_order_create_t* e = (const sp_evt_order_create_t*)payload;
  sp_mk_obj_t* o = get_or_create_obj(mk, e->p.a);
  if (!o) return SP_ENOMEM;
  o->objtype = SP_OT_ORDER;

  if (e->has_meta) {
    const void* p; uint32_t len, type;
    sp_status_t s = blob_bytes(payload, payload_len, &e->meta, &p, &len, &type);
    if (s != SP_OK) return s;
    uint32_t off = 0;
    s = meta_alloc(mk, p, len, type, &off);
    if (s != SP_OK) return s;
    o->meta_off = off;
    o->meta_len = len;
    o->meta_type = type;
  }
  return SP_OK;
}

static sp_status_t apply_order_constrain(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  if (payload_len < sizeof(sp_evt_order_constrain_t)) return SP_EINVAL;
  const sp_evt_order_constrain_t* e = (const sp_evt_order_constrain_t*)payload;
  /* In a real kernel, you’d store i<=j constraints in the order object.
     Mock kernel: no-op but ensure order object exists. */
  get_or_create_obj(mk, e->p.a)->objtype = SP_OT_ORDER;
  (void)e; (void)payload_len;
  return SP_OK;
}

static sp_status_t apply_tick(sp_mk_t* mk, const void* payload, uint32_t payload_len) {
  (void)mk;
  if (payload_len < sizeof(sp_evt_tick_t)) return SP_EINVAL;
  return SP_OK;
}

sp_status_t sp_mk_apply(sp_mk_t* mk, const sp_event_hdr_t* hdr, const void* payload) {
  if (!mk || !hdr) return SP_EINVAL;
  const uint32_t len = hdr->payload_len;

  switch (hdr->op) {
    case SP_OP_POP:             return apply_pop(mk, payload, len);
    case SP_OP_LINK:            return apply_link(mk, payload, len);
    case SP_OP_UNLINK:          return apply_unlink(mk, payload, len);
    case SP_OP_MERGE:           return apply_merge(mk, payload, len);
    case SP_OP_SPLIT:           return apply_split(mk, payload, len);
    case SP_OP_COLLAPSE:        return apply_collapse(mk, payload, len);
    case SP_OP_SET_META:        return apply_set_meta(mk, payload, len);
    case SP_OP_ORDER_CREATE:    return apply_order_create(mk, payload, len);
    case SP_OP_ORDER_CONSTRAIN: return apply_order_constrain(mk, payload, len);
    case SP_OP_TICK:            return apply_tick(mk, payload, len);
    default:                    return SP_OK; /* unknown ops ignored */
  }
}

void sp_mk_dump_summary(const sp_mk_t* mk) {
  printf("MockKernel: objs=%u edges=%u meta=%u bytes\n",
         mk ? mk->objs_len : 0,
         mk ? mk->edges_len : 0,
         mk ? mk->meta_len : 0);
  if (!mk) return;

  /* Print a few objects */
  uint32_t n = mk->objs_len < 12 ? mk->objs_len : 12;
  for (uint32_t i = 0; i < n; i++) {
    const sp_mk_obj_t* o = &mk->objs[i];
    printf("  obj[%u] id=%llu type=%u rep=%llu\n",
           i,
           (unsigned long long)o->id,
           (unsigned)o->objtype,
           (unsigned long long) ( (o->parent==o->id) ? o->id : o->parent ));
  }
}