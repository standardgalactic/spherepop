/* sp_mock_kernel.h
 * Minimal functional in-memory “kernel” for Spherepop event replay.
 */
#ifndef SP_MOCK_KERNEL_H
#define SP_MOCK_KERNEL_H

#include "spherepop_abi.h"
#include "spherepop_event_layout.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Tunables */
#ifndef SP_MK_MAX_META_BYTES
#define SP_MK_MAX_META_BYTES (1u << 20) /* 1 MiB total meta storage */
#endif

typedef struct sp_mk_edge {
  sp_handle_t src;
  sp_handle_t dst;
  sp_reltype_t reltype;
  uint32_t link_flags;
  /* Optional: edge meta pointer into meta pool (can be 0) */
  uint32_t meta_off;
  uint32_t meta_len;
  uint32_t meta_type;
} sp_mk_edge_t;

typedef struct sp_mk_obj {
  sp_handle_t id;
  sp_objtype_t objtype;

  /* Union-find representative for MERGE/COLLAPSE semantics */
  sp_handle_t parent;
  uint32_t rank;

  /* Optional object meta pointer into meta pool (can be 0) */
  uint32_t meta_off;
  uint32_t meta_len;
  uint32_t meta_type;

  /* Region bookkeeping: if objtype==SP_OT_REGION, region_root points to a handle */
  sp_handle_t region_root;
} sp_mk_obj_t;

typedef struct sp_mk {
  /* Objects and edges */
  sp_mk_obj_t* objs;
  uint32_t objs_len;
  uint32_t objs_cap;

  sp_mk_edge_t* edges;
  uint32_t edges_len;
  uint32_t edges_cap;

  /* Simple meta pool */
  uint8_t* meta_pool;
  uint32_t meta_len;
  uint32_t meta_cap;
} sp_mk_t;

/* Lifecycle */
sp_status_t sp_mk_init(sp_mk_t* mk, uint32_t obj_cap_hint, uint32_t edge_cap_hint);
void        sp_mk_free(sp_mk_t* mk);

/* Apply one event payload by op */
sp_status_t sp_mk_apply(sp_mk_t* mk, const sp_event_hdr_t* hdr, const void* payload);

/* Query-ish helpers */
const sp_mk_obj_t* sp_mk_get_obj(const sp_mk_t* mk, sp_handle_t id);
sp_handle_t sp_mk_find(sp_mk_t* mk, sp_handle_t id); /* union-find rep */

/* Debug dump */
void sp_mk_dump_summary(const sp_mk_t* mk);

#ifdef __cplusplus
}
#endif

#endif /* SP_MOCK_KERNEL_H */