#ifndef SP_CLI_COMMON_H
#define SP_CLI_COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "spherepop_abi.h"
#include "spherepop_event_layout.h"
#include "sp_mock_kernel.h"
#include "sp_replay.h"

typedef struct sp_cli_replay_result {
  sp_mk_t mk;
  uint64_t events_replayed;
} sp_cli_replay_result_t;

/* Replay an existing events.bin file into the mock kernel. */
sp_status_t sp_cli_replay_file(const char* path, sp_cli_replay_result_t* out);

/* Free replay result. */
void sp_cli_replay_free(sp_cli_replay_result_t* r);

/* Parse a sp_handle_t from argv token (supports decimal or 0xHEX). */
bool sp_cli_parse_handle(const char* s, sp_handle_t* out);

/* Write an event record (hdr + payload + optional inline bytes) to a stream. */
sp_status_t sp_cli_write_record(FILE* out,
                                uint32_t op,
                                uint32_t flags,
                                uint64_t user_tag,
                                const void* payload,
                                uint32_t payload_len);

#endif