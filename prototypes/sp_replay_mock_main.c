/* sp_replay_mock_main.c
 * Usage:
 *   ./sp_replay_mock_main events.bin
 *
 * events.bin must be a concatenation of:
 *   [sp_event_hdr_t][payload bytes]...
 */
#include "sp_replay.h"
#include "sp_mock_kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct file_io {
  FILE* f;
} file_io_t;

static sp_status_t file_read_exact(void* ctx, void* buf, uint32_t len) {
  file_io_t* io = (file_io_t*)ctx;
  size_t n = fread(buf, 1, len, io->f);
  if (n != len) return SP_ENOENT; /* treat short read as EOF */
  return SP_OK;
}

/* Handler: apply each event to mock kernel */
static sp_status_t apply_any(sp_replay_ctx_t* ctx, const sp_event_hdr_t* hdr, const void* payload) {
  sp_mk_t* mk = (sp_mk_t*)ctx->user;
  sp_status_t s = sp_mk_apply(mk, hdr, payload);
  if (s != SP_OK) {
    fprintf(stderr, "apply failed eid=%llu op=0x%x err=%d\n",
            (unsigned long long)hdr->eid, hdr->op, s);
    return s;
  }
  return SP_OK;
}

static void handlers_all(sp_replay_handlers_t* h) {
  memset(h, 0, sizeof(*h));
  h->on_pop = apply_any;
  h->on_link = apply_any;
  h->on_unlink = apply_any;
  h->on_merge = apply_any;
  h->on_split = apply_any;
  h->on_collapse = apply_any;
  h->on_set_meta = apply_any;
  h->on_order_create = apply_any;
  h->on_order_constrain = apply_any;
  h->on_tick = apply_any;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s events.bin\n", argv[0]);
    return 2;
  }

  FILE* f = fopen(argv[1], "rb");
  if (!f) {
    perror("fopen");
    return 2;
  }

  sp_mk_t mk;
  sp_status_t s = sp_mk_init(&mk, 256, 512);
  if (s != SP_OK) {
    fprintf(stderr, "mk_init failed: %d\n", s);
    return 1;
  }

  uint8_t* payload = (uint8_t*)malloc(1u << 20); /* 1 MiB payload cap */
  if (!payload) {
    fprintf(stderr, "malloc payload failed\n");
    return 1;
  }

  file_io_t io = { .f = f };
  sp_replay_ctx_t rctx;
  sp_replay_init(&rctx, &io, file_read_exact, payload, 1u << 20, &mk);

  sp_replay_handlers_t h;
  handlers_all(&h);

  uint64_t count = 0;
  while (1) {
    s = sp_replay_next(&rctx, &h);
    if (s == SP_ENOENT) break;
    if (s != SP_OK) {
      fprintf(stderr, "replay error: %d\n", s);
      break;
    }
    count++;
    if ((count % 10000ull) == 0) {
      fprintf(stderr, "replayed %llu events...\n", (unsigned long long)count);
    }
  }

  fprintf(stderr, "done. events=%llu\n", (unsigned long long)count);
  sp_mk_dump_summary(&mk);

  free(payload);
  sp_mk_free(&mk);
  fclose(f);
  return (s == SP_OK || s == SP_ENOENT) ? 0 : 1;
}