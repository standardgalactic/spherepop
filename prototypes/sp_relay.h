/* sp_replay.h
 *
 * Deterministic event replayer for Spherepop.
 */
#ifndef SP_REPLAY_H
#define SP_REPLAY_H

#include "spherepop_abi.h"
#include "spherepop_event_layout.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------
 * I/O abstraction
 * ------------------------------------------------------------ */

/* Read exactly len bytes into buf.
 * Return SP_OK or error. */
typedef sp_status_t (*sp_replay_read_fn)(
    void* ctx, void* buf, uint32_t len);

/* ------------------------------------------------------------
 * Replay context
 * ------------------------------------------------------------ */

typedef struct sp_replay_ctx {
  void* io_ctx;
  sp_replay_read_fn read;

  /* Current event header */
  sp_event_hdr_t hdr;

  /* Scratch buffer for payload */
  void* payload;
  uint32_t payload_cap;

  /* User-defined dispatch table */
  void* user;
} sp_replay_ctx_t;

/* ------------------------------------------------------------
 * Event handlers
 * ------------------------------------------------------------ */

/* All handlers return SP_OK to continue, or error to stop replay. */
typedef sp_status_t (*sp_evt_handler_fn)(
    sp_replay_ctx_t* ctx,
    const sp_event_hdr_t* hdr,
    const void* payload);

/* Dispatch table indexed by sp_op_t (sparse allowed). */
typedef struct sp_replay_handlers {
  sp_evt_handler_fn on_pop;
  sp_evt_handler_fn on_link;
  sp_evt_handler_fn on_unlink;
  sp_evt_handler_fn on_merge;
  sp_evt_handler_fn on_split;
  sp_evt_handler_fn on_collapse;
  sp_evt_handler_fn on_set_meta;
  sp_evt_handler_fn on_order_create;
  sp_evt_handler_fn on_order_constrain;
  sp_evt_handler_fn on_tick;
} sp_replay_handlers_t;

/* ------------------------------------------------------------
 * API
 * ------------------------------------------------------------ */

/* Initialize replay context. payload_buf must be provided by caller. */
void sp_replay_init(sp_replay_ctx_t* ctx,
                    void* io_ctx,
                    sp_replay_read_fn read_fn,
                    void* payload_buf,
                    uint32_t payload_cap,
                    void* user);

/* Replay next event from stream.
 * Returns:
 *   SP_OK        -> event dispatched
 *   SP_ENOENT    -> end of stream
 *   <0           -> error
 */
sp_status_t sp_replay_next(sp_replay_ctx_t* ctx,
                           const sp_replay_handlers_t* handlers);

/* ------------------------------------------------------------
 * Validation helpers
 * ------------------------------------------------------------ */

sp_status_t sp_evt_validate_blobs(const void* payload,
                                  uint32_t payload_len,
                                  const sp_evt_blob_ref_t* refs,
                                  uint32_t ref_count);

#ifdef __cplusplus
}
#endif

#endif /* SP_REPLAY_H */