/* spherepop_abi.h
 *
 * Spherepop Calculus ABI (C) — syscall surface for a relational-field OS.
 * Kernel provides: object graph storage + capability checks + event log +
 *                  constraint ticks (optional) + minimal primitives.
 * User space provides: semantics, layout, embeddings, rendering, policy.
 *
 * Design goals:
 *  - Small set of primitives: pop/merge/split/link/unlink/collapse (+query)
 *  - Deterministic, replayable: every mutating op can emit an event
 *  - Capability-secure: all handles are unforgeable + rights-scoped
 *  - Zero “implicit global state”: views/workspaces are explicit objects
 *
 * Compile: freestanding-friendly. No libc required.
 */
#ifndef SPHEREPOP_ABI_H
#define SPHEREPOP_ABI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* ============================================================
 *  Versioning
 * ============================================================ */
#define SP_ABI_VERSION_MAJOR 0u
#define SP_ABI_VERSION_MINOR 3u

/* ============================================================
 *  Core scalar types
 * ============================================================ */

/* Kernel object handle (capability-bearing). Never reuse values within a boot
 * epoch unless SP_F_HANDLE_REUSE is explicitly supported by the platform. */
typedef uint64_t sp_handle_t;

/* Event id in the append-only journal. Monotone increasing. */
typedef uint64_t sp_eid_t;

/* Kernel time tick (optional). */
typedef uint64_t sp_tick_t;

/* Object type tags (kernel-known; user types live in metadata). */
typedef uint32_t sp_objtype_t;

/* Relation/edge type tags (kernel-known; user types live in metadata). */
typedef uint32_t sp_reltype_t;

/* Rights bitmask (capabilities). */
typedef uint64_t sp_rights_t;

/* Status code (negative = error, non-negative = success / info). */
typedef int32_t sp_status_t;

/* ============================================================
 *  Status / errors
 * ============================================================ */
enum {
  SP_OK                 = 0,

  /* Generic */
  SP_EINVAL             = -1,   /* invalid arg / malformed struct */
  SP_EFAULT             = -2,   /* bad user pointer (if relevant) */
  SP_ENOSYS             = -3,   /* syscall/op not implemented */
  SP_ENOMEM             = -4,   /* out of memory */
  SP_EBUSY              = -5,   /* contention / retry suggested */
  SP_EAGAIN             = -6,   /* nonblocking would block */
  SP_ETIME              = -7,   /* timeout */
  SP_EOVERFLOW          = -8,   /* size/offset overflow */

  /* Capability / security */
  SP_EACCES             = -20,  /* missing right */
  SP_ECAP               = -21,  /* invalid/expired capability */
  SP_EAUTH              = -22,  /* authorization failure */
  SP_ELOCKED            = -23,  /* object locked by policy */

  /* Graph / topology */
  SP_ENOENT             = -40,  /* object/edge not found */
  SP_ECONFLICT          = -41,  /* merge conflict / constraint violation */
  SP_ECYCLE             = -42,  /* would create forbidden cycle */
  SP_EORDER             = -43,  /* violates partial order / causal mask */
  SP_EINVARIANT         = -44,  /* breaks invariant (e.g., gauge constraint) */
};

/* ============================================================
 *  Flags
 * ============================================================ */

/* General call flags */
enum {
  SP_F_NONE             = 0u,

  /* If set on mutating ops: kernel must append an event record and return eid. */
  SP_F_RECORD_EVENT     = 1u << 0,

  /* If set: operation must be deterministic w.r.t. event replay (no nondet). */
  SP_F_DETERMINISTIC    = 1u << 1,

  /* If set: kernel may perform internal compaction/normalization as part of op. */
  SP_F_NORMALIZE        = 1u << 2,

  /* If set: treat missing optional fields as defaults rather than error. */
  SP_F_LENIENT          = 1u << 3,

  /* If set: do not block (where meaningful). */
  SP_F_NONBLOCK         = 1u << 4,
};

/* Edge/link flags */
enum {
  SP_LF_NONE            = 0u,

  /* Enforce acyclicity under this relation type (kernel-maintained). */
  SP_LF_ACYCLIC         = 1u << 0,

  /* Enforce causal/time order: src must be <= dst in a chosen order object. */
  SP_LF_CAUSAL          = 1u << 1,

  /* Edge is “soft”: may be dropped by collapse/compaction unless pinned. */
  SP_LF_SOFT            = 1u << 2,

  /* Edge is “pinned”: preserved across collapse unless explicitly removed. */
  SP_LF_PINNED          = 1u << 3,
};

/* Collapse flags */
enum {
  SP_CF_NONE            = 0u,

  /* Collapse only if region is quiescent (no inflight refs/locks). */
  SP_CF_QUIESCENT_ONLY  = 1u << 0,

  /* Allow edge pruning (except pinned) during collapse. */
  SP_CF_PRUNE_SOFT      = 1u << 1,

  /* Compact metadata blobs (dedupe/pack). */
  SP_CF_COMPACT_META    = 1u << 2,
};

/* ============================================================
 *  Rights / capabilities
 * ============================================================ */

/* Minimal rights set. Extend as needed. */
enum : sp_rights_t {
  SP_R_READ             = 1ull << 0,
  SP_R_WRITE            = 1ull << 1,
  SP_R_LINK             = 1ull << 2,
  SP_R_UNLINK           = 1ull << 3,
  SP_R_MERGE            = 1ull << 4,
  SP_R_SPLIT            = 1ull << 5,
  SP_R_COLLAPSE         = 1ull << 6,
  SP_R_QUERY            = 1ull << 7,

  /* Rights over views/workspaces (gauge choices). */
  SP_R_VIEW             = 1ull << 8,
  SP_R_ORDER            = 1ull << 9,  /* manage partial orders / causal constraints */

  /* Administrative / debugging */
  SP_R_ADMIN            = 1ull << 63
};

/* A capability token is represented by a handle + implicit kernel-side rights.
 * Optionally, platforms can expose explicit rights via sp_cap_get(). */

/* ============================================================
 *  Object & relation type conventions (kernel-known)
 * ============================================================ */
enum : sp_objtype_t {
  SP_OT_GENERIC         = 0,
  SP_OT_VIEW            = 1,  /* a workspace/chart (gauge choice) */
  SP_OT_ORDER           = 2,  /* a partial order / time-order object */
  SP_OT_EVENTLOG        = 3,  /* handle to a journal stream */
  SP_OT_PROCESS         = 4,
  SP_OT_RESOURCE        = 5,
  SP_OT_REGION          = 6,  /* a set / subgraph handle (for collapse/split) */
};

enum : sp_reltype_t {
  SP_RT_GENERIC         = 0,
  SP_RT_CONTAINS        = 1,
  SP_RT_DEPENDS         = 2,
  SP_RT_SIMILAR         = 3,
  SP_RT_CAUSAL          = 4,
  SP_RT_OWNS            = 5,
};

/* ============================================================
 *  Metadata blobs
 * ============================================================ */

/* Simple typed metadata header. Payload is arbitrary bytes.
 * Kernel stores and optionally compacts; does not interpret except type/len. */
typedef struct sp_meta_blob {
  uint32_t type;     /* application-defined */
  uint32_t len;      /* bytes following header */
  /* uint8_t payload[len]; */
} sp_meta_blob_t;

/* ============================================================
 *  Vector fields (optional, kernel-agnostic)
 * ============================================================ */

/* Kernel treats these as opaque bytes unless a platform offers acceleration.
 * Still useful for standardizing user-kernel interchange for Phi/v/S snapshots. */
typedef struct sp_field_view {
  uint32_t elem_type;   /* e.g. 1=float32, 2=float16, 3=int32 */
  uint32_t elem_size;   /* bytes per element */
  uint32_t rows;        /* N */
  uint32_t cols;        /* C (or 1 for S) */
  uint64_t stride_bytes;/* row stride */
  const void* data;     /* user pointer */
} sp_field_view_t;

/* ============================================================
 *  Requests / responses
 * ============================================================ */

typedef struct sp_req_header {
  uint32_t op;        /* sp_op_t */
  uint32_t flags;     /* SP_F_* */
  uint64_t user_tag;  /* echoed back; for matching async/batched calls */
} sp_req_header_t;

typedef struct sp_rsp_header {
  sp_status_t status; /* SP_OK or negative error */
  uint32_t _pad;
  uint64_t user_tag;  /* echoed from request */
  sp_eid_t eid;       /* valid if SP_F_RECORD_EVENT */
} sp_rsp_header_t;

/* ============================================================
 *  Syscall operation codes
 * ============================================================ */
typedef enum sp_op {
  /* Introspection */
  SP_OP_ABI_INFO        = 0x0001, /* query ABI version + features */
  SP_OP_CAP_GET         = 0x0002, /* get explicit rights (optional platform support) */

  /* Spherepop primitives */
  SP_OP_POP             = 0x0100, /* create object */
  SP_OP_LINK            = 0x0101, /* create edge */
  SP_OP_UNLINK          = 0x0102, /* remove edge */
  SP_OP_MERGE           = 0x0103, /* unify objects (colimit-style) */
  SP_OP_SPLIT           = 0x0104, /* factor object into region/children */
  SP_OP_COLLAPSE        = 0x0105, /* commit/quotient/GC a region */

  /* Queries / traversal */
  SP_OP_GET_META        = 0x0200,
  SP_OP_SET_META        = 0x0201,
  SP_OP_LIST_EDGES      = 0x0202,
  SP_OP_READ_EVENT      = 0x0203, /* read event log by eid */

  /* Orders / causality */
  SP_OP_ORDER_CREATE    = 0x0300, /* create partial order object */
  SP_OP_ORDER_CONSTRAIN = 0x0301, /* add constraint i <= j */
  SP_OP_ORDER_CHECK     = 0x0302, /* check i <= j / detect violation */

  /* Optional: scheduling/ticks (platform-defined) */
  SP_OP_TICK            = 0x0400, /* advance integrator / scheduler */
} sp_op_t;

/* ============================================================
 *  ABI info / feature bits
 * ============================================================ */
typedef struct sp_abi_info_req {
  sp_req_header_t h;
} sp_abi_info_req_t;

typedef struct sp_abi_info_rsp {
  sp_rsp_header_t h;
  uint32_t abi_major;
  uint32_t abi_minor;
  uint64_t feature_bits;
} sp_abi_info_rsp_t;

/* Feature bits (returned in sp_abi_info_rsp.feature_bits) */
enum : uint64_t {
  SP_FEAT_EVENTLOG          = 1ull << 0,
  SP_FEAT_DERIVED_STACKS    = 1ull << 1, /* kernel supports higher symmetries/ghost plumbing (minimal) */
  SP_FEAT_ORDER_OBJECTS     = 1ull << 2,
  SP_FEAT_CAP_EXPLICIT      = 1ull << 3, /* SP_OP_CAP_GET supported */
  SP_FEAT_HANDLE_REUSE      = 1ull << 4,
  SP_FEAT_BATCH_CALL        = 1ull << 5, /* SP_OP_BATCH supported (not defined here) */
};

/* ============================================================
 *  Capability get (optional)
 * ============================================================ */
typedef struct sp_cap_get_req {
  sp_req_header_t h;
  sp_handle_t obj;
} sp_cap_get_req_t;

typedef struct sp_cap_get_rsp {
  sp_rsp_header_t h;
  sp_rights_t rights;
} sp_cap_get_rsp_t;

/* ============================================================
 *  POP: create object
 * ============================================================ */
typedef struct sp_pop_req {
  sp_req_header_t h;

  sp_objtype_t objtype;
  uint32_t _pad;

  /* Parent container (optional). If nonzero, kernel may link parent->new via
   * SP_RT_CONTAINS unless overridden by reltype_override. */
  sp_handle_t parent;

  /* Optional: override containment relation type (0 = default contains). */
  sp_reltype_t reltype_override;
  uint32_t link_flags; /* SP_LF_* for the implicit parent link */

  /* Optional metadata blob (can be NULL). */
  const sp_meta_blob_t* meta;
} sp_pop_req_t;

typedef struct sp_pop_rsp {
  sp_rsp_header_t h;
  sp_handle_t obj; /* newly created object handle */
} sp_pop_rsp_t;

/* ============================================================
 *  LINK / UNLINK: edge operations
 * ============================================================ */
typedef struct sp_link_req {
  sp_req_header_t h;
  sp_handle_t src;
  sp_handle_t dst;
  sp_reltype_t reltype;
  uint32_t link_flags; /* SP_LF_* */
  /* Optional edge metadata */
  const sp_meta_blob_t* meta;
  /* Optional: order object if SP_LF_CAUSAL is set (0 = default order in view). */
  sp_handle_t order;
} sp_link_req_t;

typedef struct sp_link_rsp {
  sp_rsp_header_t h;
} sp_link_rsp_t;

typedef struct sp_unlink_req {
  sp_req_header_t h;
  sp_handle_t src;
  sp_handle_t dst;
  sp_reltype_t reltype;
} sp_unlink_req_t;

typedef struct sp_unlink_rsp {
  sp_rsp_header_t h;
} sp_unlink_rsp_t;

/* ============================================================
 *  MERGE: unify objects (semantic/structural colimit)
 * ============================================================ */

/* Merge modes */
enum : uint32_t {
  SP_MG_DEFAULT         = 0,
  SP_MG_KEEP_LEFT       = 1,  /* prefer left metadata on conflict */
  SP_MG_KEEP_RIGHT      = 2,  /* prefer right metadata on conflict */
  SP_MG_REQUIRE_EQUAL   = 3,  /* fail if conflicts */
};

typedef struct sp_merge_req {
  sp_req_header_t h;
  sp_handle_t a;
  sp_handle_t b;
  uint32_t mode;     /* SP_MG_* */
  uint32_t _pad;

  /* Optional: result container (0 = kernel chooses, often a). */
  sp_handle_t into;

  /* Optional: merge policy blob (opaque to kernel unless platform policy module). */
  const sp_meta_blob_t* policy;
} sp_merge_req_t;

typedef struct sp_merge_rsp {
  sp_rsp_header_t h;
  sp_handle_t result; /* merged representative */
} sp_merge_rsp_t;

/* ============================================================
 *  SPLIT: factor object into sub-objects / region
 * ============================================================ */

/* Split modes */
enum : uint32_t {
  SP_SP_DEFAULT         = 0,
  SP_SP_BY_RELTYPE      = 1, /* split along edges of a given reltype */
  SP_SP_BY_META_TYPE    = 2, /* split by metadata type */
  SP_SP_BY_ORDER        = 3, /* split by causal order frontier */
};

typedef struct sp_split_req {
  sp_req_header_t h;
  sp_handle_t obj;
  uint32_t mode;          /* SP_SP_* */
  sp_reltype_t reltype;   /* used if mode == SP_SP_BY_RELTYPE */
  uint32_t meta_type;     /* used if mode == SP_SP_BY_META_TYPE */
  sp_handle_t order;      /* used if mode == SP_SP_BY_ORDER */

  /* Optional: parameters for split (opaque). */
  const sp_meta_blob_t* params;
} sp_split_req_t;

typedef struct sp_split_rsp {
  sp_rsp_header_t h;
  sp_handle_t region;   /* a region handle representing the split result */
  uint32_t count;       /* number of produced children (if enumerated) */
  uint32_t _pad;
} sp_split_rsp_t;

/* ============================================================
 *  COLLAPSE: commit/quotient/GC a region
 * ============================================================ */
typedef struct sp_collapse_req {
  sp_req_header_t h;
  sp_handle_t region;     /* region handle (from split or explicit region object) */
  uint32_t collapse_flags;/* SP_CF_* */
  uint32_t _pad;

  /* Optional: collapse policy / projection rule (opaque) */
  const sp_meta_blob_t* policy;
} sp_collapse_req_t;

typedef struct sp_collapse_rsp {
  sp_rsp_header_t h;
  sp_handle_t representative; /* collapsed representative (may equal region) */
} sp_collapse_rsp_t;

/* ============================================================
 *  Metadata get/set
 * ============================================================ */
typedef struct sp_get_meta_req {
  sp_req_header_t h;
  sp_handle_t obj;
  uint32_t type;      /* metadata type to fetch; 0 = first/default */
  uint32_t _pad;
  void* out_buf;      /* user buffer for payload */
  uint32_t out_cap;   /* capacity in bytes (payload only) */
  uint32_t* out_len;  /* receives payload length */
} sp_get_meta_req_t;

typedef struct sp_get_meta_rsp {
  sp_rsp_header_t h;
} sp_get_meta_rsp_t;

typedef struct sp_set_meta_req {
  sp_req_header_t h;
  sp_handle_t obj;
  const sp_meta_blob_t* meta; /* header+payload */
} sp_set_meta_req_t;

typedef struct sp_set_meta_rsp {
  sp_rsp_header_t h;
} sp_set_meta_rsp_t;

/* ============================================================
 *  List edges
 * ============================================================ */

/* Edge record returned by SP_OP_LIST_EDGES */
typedef struct sp_edge_rec {
  sp_handle_t src;
  sp_handle_t dst;
  sp_reltype_t reltype;
  uint32_t link_flags;
} sp_edge_rec_t;

typedef struct sp_list_edges_req {
  sp_req_header_t h;
  sp_handle_t obj;        /* list edges incident to obj */
  uint32_t direction;     /* 0=outgoing, 1=incoming, 2=both */
  sp_reltype_t reltype;   /* 0=all or filter by reltype */
  sp_edge_rec_t* out;     /* array */
  uint32_t out_cap;       /* number of records capacity */
  uint32_t* out_count;    /* receives number written */
} sp_list_edges_req_t;

typedef struct sp_list_edges_rsp {
  sp_rsp_header_t h;
} sp_list_edges_rsp_t;

/* ============================================================
 *  Event log read (deterministic replay support)
 * ============================================================ */

/* Event header (fixed) followed by op-specific payload, both written by kernel.
 * Payload layout is platform-defined but should include request parameters. */
typedef struct sp_event_hdr {
  sp_eid_t eid;
  sp_tick_t tick;
  uint32_t op;       /* sp_op_t */
  uint32_t flags;    /* SP_F_* used */
  uint64_t user_tag; /* copied */
  uint32_t payload_len;
  uint32_t _pad;
} sp_event_hdr_t;

typedef struct sp_read_event_req {
  sp_req_header_t h;
  sp_handle_t eventlog;  /* SP_OT_EVENTLOG, or 0=default system journal */
  sp_eid_t eid;          /* event id to read */
  sp_event_hdr_t* out_hdr;
  void* out_payload;
  uint32_t out_cap;      /* payload capacity */
  uint32_t* out_len;     /* receives payload length */
} sp_read_event_req_t;

typedef struct sp_read_event_rsp {
  sp_rsp_header_t h;
} sp_read_event_rsp_t;

/* ============================================================
 *  Order objects (partial order / causal constraints)
 * ============================================================ */

typedef struct sp_order_create_req {
  sp_req_header_t h;
  /* Optional: attach to a view/workspace (0 = global). */
  sp_handle_t view;
  /* Optional metadata describing order semantics (e.g. total order, DAG). */
  const sp_meta_blob_t* meta;
} sp_order_create_req_t;

typedef struct sp_order_create_rsp {
  sp_rsp_header_t h;
  sp_handle_t order;
} sp_order_create_rsp_t;

typedef struct sp_order_constrain_req {
  sp_req_header_t h;
  sp_handle_t order;
  uint32_t i; /* token index i */
  uint32_t j; /* token index j */
  /* constraint: i <= j */
} sp_order_constrain_req_t;

typedef struct sp_order_constrain_rsp {
  sp_rsp_header_t h;
} sp_order_constrain_rsp_t;

typedef struct sp_order_check_req {
  sp_req_header_t h;
  sp_handle_t order;
  uint32_t i;
  uint32_t j;
} sp_order_check_req_t;

typedef struct sp_order_check_rsp {
  sp_rsp_header_t h;
  /* 1 if i <= j, 0 otherwise */
  uint32_t holds;
  uint32_t _pad;
} sp_order_check_rsp_t;

/* ============================================================
 *  Tick (optional): advance integrator/scheduler
 * ============================================================ */
typedef struct sp_tick_req {
  sp_req_header_t h;
  sp_tick_t dt;          /* delta ticks */
  sp_handle_t view;      /* optional: tick within a view/workspace */
} sp_tick_req_t;

typedef struct sp_tick_rsp {
  sp_rsp_header_t h;
  sp_tick_t now;
} sp_tick_rsp_t;

/* ============================================================
 *  Syscall entry point(s)
 * ============================================================ */

/* Minimal syscall: one request -> one response. The kernel copies request data
 * from user memory and writes response. Platforms may implement this with an
 * interrupt, a fast sysenter, or a message-passing gate.
 *
 * Returns SP_OK if the syscall mechanism succeeded; response.status carries the
 * operation result.
 */
sp_status_t sp_syscall(const void* req, size_t req_len, void* rsp, size_t rsp_len);

/* ============================================================
 *  Convenience: typed wrappers (optional, user-space)
 * ============================================================ */

static inline sp_status_t sp_abi_info(sp_abi_info_rsp_t* out) {
  sp_abi_info_req_t req = {0};
  req.h.op = SP_OP_ABI_INFO;
  req.h.flags = SP_F_NONE;
  req.h.user_tag = 0;
  return sp_syscall(&req, sizeof(req), out, sizeof(*out));
}

static inline sp_status_t sp_pop(const sp_pop_req_t* req, sp_pop_rsp_t* rsp) {
  return sp_syscall(req, sizeof(*req), rsp, sizeof(*rsp));
}

static inline sp_status_t sp_link(const sp_link_req_t* req, sp_link_rsp_t* rsp) {
  return sp_syscall(req, sizeof(*req), rsp, sizeof(*rsp));
}

static inline sp_status_t sp_unlink(const sp_unlink_req_t* req, sp_unlink_rsp_t* rsp) {
  return sp_syscall(req, sizeof(*req), rsp, sizeof(*rsp));
}

static inline sp_status_t sp_merge(const sp_merge_req_t* req, sp_merge_rsp_t* rsp) {
  return sp_syscall(req, sizeof(*req), rsp, sizeof(*rsp));
}

static inline sp_status_t sp_split(const sp_split_req_t* req, sp_split_rsp_t* rsp) {
  return sp_syscall(req, sizeof(*req), rsp, sizeof(*rsp));
}

static inline sp_status_t sp_collapse(const sp_collapse_req_t* req, sp_collapse_rsp_t* rsp) {
  return sp_syscall(req, sizeof(*req), rsp, sizeof(*rsp));
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SPHEREPOP_ABI_H *