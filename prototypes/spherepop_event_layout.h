/* spherepop_event_layout.h
 *
 * Canonical event payload schemas for SP_OP_* records.
 * Intended for deterministic replay tooling.
 *
 * Kernel SHOULD log:
 *   sp_event_hdr_t (from spherepop_abi.h)
 *   followed by exactly one of these payloads (op-specific)
 *   plus inline blob bytes as described.
 *
 * All structs are packed as little-endian, natural alignment (recommend pragma pack 1).
 */
#ifndef SPHEREPOP_EVENT_LAYOUT_H
#define SPHEREPOP_EVENT_LAYOUT_H

#include "spherepop_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
  #pragma pack(push, 1)
#elif defined(__GNUC__)
  #define SP_PACKED __attribute__((packed))
#else
  #define SP_PACKED
#endif

/* A reference to an inline blob inside the event payload. */
typedef struct SP_PACKED sp_evt_blob_ref {
  uint32_t type;    /* sp_meta_blob_t.type */
  uint32_t len;     /* payload bytes */
  uint32_t off;     /* offset from start of *payload* to payload bytes (not header) */
  uint32_t _pad;
} sp_evt_blob_ref_t;

/* Common prelude for graph-mutating ops (optional but useful). */
typedef struct SP_PACKED sp_evt_prelude {
  sp_handle_t a;       /* primary handle (e.g., src or obj) */
  sp_handle_t b;       /* secondary handle (e.g., dst) */
  sp_handle_t c;       /* tertiary (e.g., order, parent, into) */
  uint32_t u0;         /* op-specific small field (mode/flags/reltype) */
  uint32_t u1;         /* op-specific small field */
  uint64_t reserved;
} sp_evt_prelude_t;

/* ------------------------------
 * POP payload
 * ------------------------------ */
typedef struct SP_PACKED sp_evt_pop {
  sp_evt_prelude_t p;    /* a=NEW_OBJ (filled by kernel), b=parent, c=0 */
  uint32_t objtype;
  uint32_t link_flags;
  uint32_t reltype_override;
  uint32_t has_meta;     /* 0/1 */
  sp_evt_blob_ref_t meta;/* valid if has_meta */
  /* followed by meta payload bytes */
} sp_evt_pop_t;

/* ------------------------------
 * LINK payload
 * ------------------------------ */
typedef struct SP_PACKED sp_evt_link {
  sp_evt_prelude_t p;     /* a=src, b=dst, c=order */
  uint32_t reltype;
  uint32_t link_flags;
  uint32_t has_meta;
  uint32_t _pad;
  sp_evt_blob_ref_t meta; /* valid if has_meta */
} sp_evt_link_t;

/* ------------------------------
 * UNLINK payload
 * ------------------------------ */
typedef struct SP_PACKED sp_evt_unlink {
  sp_evt_prelude_t p;     /* a=src, b=dst */
  uint32_t reltype;
  uint32_t _pad0;
  uint64_t _pad1;
} sp_evt_unlink_t;

/* ------------------------------
 * MERGE payload
 * ------------------------------ */
typedef struct SP_PACKED sp_evt_merge {
  sp_evt_prelude_t p;     /* a=a, b=b, c=into */
  uint32_t mode;          /* SP_MG_* */
  uint32_t has_policy;
  sp_evt_blob_ref_t policy; /* valid if has_policy */
  /* kernel writes resulting representative handle in event header user_tag or
     in p.reserved via platform convention; recommended: store in p.reserved */
} sp_evt_merge_t;

/* ------------------------------
 * SPLIT payload
 * ------------------------------ */
typedef struct SP_PACKED sp_evt_split {
  sp_evt_prelude_t p;     /* a=obj, b=region (filled by kernel), c=order */
  uint32_t mode;          /* SP_SP_* */
  uint32_t reltype;       /* if BY_RELTYPE */
  uint32_t meta_type;     /* if BY_META_TYPE */
  uint32_t has_params;
  sp_evt_blob_ref_t params; /* valid if has_params */
  uint32_t count;         /* number of produced children (if known) */
  uint32_t _pad;
} sp_evt_split_t;

/* ------------------------------
 * COLLAPSE payload
 * ------------------------------ */
typedef struct SP_PACKED sp_evt_collapse {
  sp_evt_prelude_t p;      /* a=region, b=rep (filled by kernel) */
  uint32_t collapse_flags; /* SP_CF_* */
  uint32_t has_policy;
  sp_evt_blob_ref_t policy;
} sp_evt_collapse_t;

/* ------------------------------
 * SET_META payload
 * ------------------------------ */
typedef struct SP_PACKED sp_evt_set_meta {
  sp_evt_prelude_t p;      /* a=obj */
  uint32_t has_meta;
  uint32_t _pad;
  sp_evt_blob_ref_t meta;
} sp_evt_set_meta_t;

/* ------------------------------
 * ORDER: create / constrain
 * ------------------------------ */
typedef struct SP_PACKED sp_evt_order_create {
  sp_evt_prelude_t p;     /* a=order (filled by kernel), b=view */
  uint32_t has_meta;
  uint32_t _pad;
  sp_evt_blob_ref_t meta;
} sp_evt_order_create_t;

typedef struct SP_PACKED sp_evt_order_constrain {
  sp_evt_prelude_t p;     /* a=order */
  uint32_t i;
  uint32_t j;
  /* constraint is i <= j */
  uint64_t _pad;
} sp_evt_order_constrain_t;

/* ------------------------------
 * TICK payload (optional)
 * ------------------------------ */
typedef struct SP_PACKED sp_evt_tick {
  sp_evt_prelude_t p;     /* a=view */
  sp_tick_t dt;
  sp_tick_t now;          /* filled by kernel, optional */
} sp_evt_tick_t;

#if defined(_MSC_VER)
  #pragma pack(pop)
#elif defined(__GNUC__)
  #undef SP_PACKED
#endif

/* ------------------------------------------------------------
 * Parsing guidance
 * ------------------------------------------------------------
 * - Read sp_event_hdr_t first (gives op + payload_len).
 * - Cast payload to the appropriate sp_evt_* type based on hdr.op.
 * - For each blob_ref with len>0:
 *     payload_bytes = ((uint8_t*)payload) + blob_ref.off;
 *     blob_ref.type identifies the app-defined meta type.
 * - Kernel should ensure blob_ref.off + blob_ref.len <= payload_len.
 */

#ifdef __cplusplus
}
#endif

#endif /* SPHEREPOP_EVENT_LAYOUT_H */