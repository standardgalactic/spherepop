#ifndef SP_INJECT_CLIENT_H
#define SP_INJECT_CLIENT_H

#include "spherepop_event_layout.h"
#include "sp_layout_hint.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Serialize a proposal as NDJSON into a caller buffer.
 * Returns bytes written or negative error.
 */

int sp_inject_build_merge(char* out, int cap,
                          const char* client, uint64_t req,
                          sp_handle_t a, sp_handle_t b, uint32_t mode);

int sp_inject_build_link(char* out, int cap,
                         const char* client, uint64_t req,
                         sp_handle_t src, sp_handle_t dst,
                         sp_reltype_t rel, uint32_t flags);

int sp_inject_build_unlink(char* out, int cap,
                           const char* client, uint64_t req,
                           sp_handle_t src, sp_handle_t dst,
                           sp_reltype_t rel);

int sp_inject_build_collapse(char* out, int cap,
                             const char* client, uint64_t req,
                             sp_handle_t region, uint32_t collapse_flags);

int sp_inject_build_set_layout(char* out, int cap,
                               const char* client, uint64_t req,
                               sp_handle_t id, const sp_layout_hint_t* h);

#ifdef __cplusplus
}
#endif
#endif