#include "sp_cli_common.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* -----------------------------
 * Replay: file IO adapter
 * ----------------------------- */

typedef struct file_io {
  FILE* f;
} file_io_t;

static sp_status_t file_read(void* user, void* out_buf, uint32_t cap, uint32_t* out_n) {
  file_io_t* io = (file_io_t*)user;
  if (!io || !io->f || !out_buf || !out_n) return SP_EINVAL;
  size_t n = fread(out_buf, 1, cap, io->f);
  if (n == 0) {
    if (feof(io->f)) { *out_n = 0; return SP_OK; }
    return SP_EIO;
  }
  *out_n = (uint32_t)n;
  return SP_OK;
}

static sp_status_t file_seek(void* user, uint64_t abs_off) {
  file_io_t* io = (file_io_t*)user;
  if (!io || !io->f) return SP_EINVAL;
  if (fseek(io->f, (long)abs_off, SEEK_SET) != 0) return SP_EIO;
  return SP_OK;
}

static sp_status_t file_tell(void* user, uint64_t* out_off) {
  file_io_t* io = (file_io_t*)user;
  if (!io || !io->f || !out_off) return SP_EINVAL;
  long p = ftell(io->f);
  if (p < 0) return SP_EIO;
  *out_off = (uint64_t)p;
  return SP_OK;
}

static sp_status_t mk_dispatch(void* user, const sp_event_hdr_t* hdr, const void* payload) {
  sp_mk_t* mk = (sp_mk_t*)user;
  return sp_mk_apply(mk, hdr, payload);
}

sp_status_t sp_cli_replay_file(const char* path, sp_cli_replay_result_t* out) {
  if (!path || !out) return SP_EINVAL;
  memset(out, 0, sizeof(*out));

  FILE* f = fopen(path, "rb");
  if (!f) return SP_EIO;

  sp_status_t s = sp_mk_init(&out->mk);
  if (s < 0) { fclose(f); return s; }

  file_io_t fio = { .f = f };

  sp_replay_handlers_t h = {0};
  h.user = &out->mk;
  h.on_event = mk_dispatch;

  sp_replay_ctx_t ctx = {0};
  ctx.io.user = &fio;
  ctx.io.read = file_read;
  ctx.io.seek = file_seek;
  ctx.io.tell = file_tell;

  uint64_t count = 0;
  for (;;) {
    s = sp_replay_next(&ctx, &h);
    if (s == SP_ENOENT) break;
    if (s < 0) break;
    count++;
  }

  out->events_replayed = count;
  fclose(f);

  if (s == SP_ENOENT) return SP_OK;
  return s;
}

void sp_cli_replay_free(sp_cli_replay_result_t* r) {
  if (!r) return;
  sp_mk_free(&r->mk);
  memset(r, 0, sizeof(*r));
}

bool sp_cli_parse_handle(const char* s, sp_handle_t* out) {
  if (!s || !out) return false;
  errno = 0;
  char* end = NULL;
  unsigned long long v = 0;

  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    v = strtoull(s + 2, &end, 16);
  } else {
    v = strtoull(s, &end, 10);
  }

  if (errno != 0) return false;
  if (!end || *end != '\0') return false;
  *out = (sp_handle_t)v;
  return true;
}

sp_status_t sp_cli_write_record(FILE* out,
                                uint32_t op,
                                uint32_t flags,
                                uint64_t user_tag,
                                const void* payload,
                                uint32_t payload_len) {
  if (!out) return SP_EINVAL;
  if (payload_len && !payload) return SP_EINVAL;

  sp_event_hdr_t hdr = {0};
  hdr.eid = 0;      /* proposals: 0 */
  hdr.tick = 0;     /* proposals: 0 */
  hdr.op = op;
  hdr.flags = flags;
  hdr.user_tag = user_tag;
  hdr.payload_len = payload_len;

  if (fwrite(&hdr, 1, sizeof(hdr), out) != sizeof(hdr)) return SP_EIO;
  if (payload_len) {
    if (fwrite(payload, 1, payload_len, out) != payload_len) return SP_EIO;
  }
  return SP_OK;
}