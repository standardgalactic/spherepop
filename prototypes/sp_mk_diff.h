/* sp_mk_diff.h
 * Incremental diff emitter for sp_mock_kernel.
 */
#ifndef SP_MK_DIFF_H
#define SP_MK_DIFF_H

#include "sp_mock_kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------
 * Diff sink interface
 * ------------------------------------------------------------ */

typedef struct sp_mk_diff_sink {
  void* ctx;

  void (*begin)(void* ctx, sp_eid_t seq);
  void (*add_obj)(void* ctx, sp_handle_t id, sp_objtype_t type, sp_handle_t rep);
  void (*rem_obj)(void* ctx, sp_handle_t id);

  void (*add_edge)(void* ctx,
                   sp_handle_t src, sp_handle_t dst,
                   sp_reltype_t rel, uint32_t flags);
  void (*rem_edge)(void* ctx,
                   sp_handle_t src, sp_handle_t dst,
                   sp_reltype_t rel);

  void (*end)(void* ctx);
} sp_mk_diff_sink_t;

/* ------------------------------------------------------------
 * Apply + diff
 * ------------------------------------------------------------ */

/* Apply an event and emit incremental diffs.
 * This wraps sp_mk_apply().
 */
sp_status_t sp_mk_apply_with_diff(sp_mk_t* mk,
                                  const sp_event_hdr_t* hdr,
                                  const void* payload,
                                  const sp_mk_diff_sink_t* sink);

#ifdef __cplusplus
}
#endif

#endif /* SP_MK_DIFF_H */