#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "spherepop_abi.h"
#include "spherepop_event_layout.h"
#include "sp_mock_kernel.h"
#include "sp_replay.h"

/* -----------------------------
 * Small replay helper (base log)
 * ----------------------------- */

typedef struct file_io { FILE* f; } file_io_t;

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
  return (fseek(io->f, (long)abs_off, SEEK_SET) == 0) ? SP_OK : SP_EIO;
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

static sp_status_t replay_file_into_mk(const char* path, sp_mk_t* mk, uint64_t* out_count, sp_eid_t* out_max_eid) {
  if (out_count) *out_count = 0;
  if (out_max_eid) *out_max_eid = 0;

  FILE* f = fopen(path, "rb");
  if (!f) return SP_EIO;

  file_io_t fio = { .f = f };

  sp_replay_handlers_t h = {0};
  h.user = mk;
  h.on_event = mk_dispatch;

  sp_replay_ctx_t ctx = {0};
  ctx.io.user = &fio;
  ctx.io.read = file_read;
  ctx.io.seek = file_seek;
  ctx.io.tell = file_tell;

  uint64_t n = 0;
  sp_eid_t max_eid = 0;

  for (;;) {
    sp_status_t s = sp_replay_next(&ctx, &h);
    if (s == SP_ENOENT) break;
    if (s < 0) { fclose(f); return s; }
    /* ctx.last_hdr is not exposed; use mk_dispatch path only. So we recompute max eid by rereading:
       easiest is to read headers directly, but we already have replay infrastructure.
       We'll instead do a second pass for max eid below if needed. */
    n++;
  }

  fclose(f);

  if (out_count) *out_count = n;

  /* Second pass: compute max eid by scanning headers quickly. */
  f = fopen(path, "rb");
  if (!f) return SP_EIO;

  for (;;) {
    sp_event_hdr_t hdr;
    size_t r = fread(&hdr, 1, sizeof(hdr), f);
    if (r == 0 && feof(f)) break;
    if (r != sizeof(hdr)) { fclose(f); return SP_EIO; }
    if (hdr.eid > max_eid) max_eid = hdr.eid;
    if (hdr.payload_len) {
      if (fseek(f, (long)hdr.payload_len, SEEK_CUR) != 0) { fclose(f); return SP_EIO; }
    }
  }

  fclose(f);

  if (out_max_eid) *out_max_eid = max_eid;
  return SP_OK;
}

/* -----------------------------
 * CLI / parsing
 * ----------------------------- */

static void usage(void) {
  fprintf(stderr,
    "Usage:\n"
    "  spcommit-mock --in proposals.bin --out committed.bin [--base base.bin] [--start-handle N]\n"
    "\n"
    "What it does:\n"
    "  - Copies --base (if provided) into --out\n"
    "  - Reads proposal records from --in\n"
    "  - Assigns sequential EIDs starting after base.max_eid\n"
    "  - For POP payloads: if NEW_OBJ (p.a) == 0, fills it deterministically\n"
    "\n"
    "Notes:\n"
    "  - This is a prototype commit tool for mock/testing workflows.\n"
    "  - It does NOT perform arbitration; it deterministically rewrites proposals.\n"
  );
}

static int parse_u64(const char* s, uint64_t* out) {
  if (!s || !out) return 0;
  char* end = NULL;
  unsigned long long v = 0;
  if (s[0]=='0' && (s[1]=='x' || s[1]=='X')) v = strtoull(s+2, &end, 16);
  else v = strtoull(s, &end, 10);
  if (!end || *end != '\0') return 0;
  *out = (uint64_t)v;
  return 1;
}

static sp_handle_t compute_next_handle_from_mk(const sp_mk_t* mk, sp_handle_t floor_min) {
  sp_handle_t max_id = 0;
  for (uint32_t i = 0; i < mk->objs_len; i++) {
    if (mk->objs[i].id > max_id) max_id = mk->objs[i].id;
  }
  sp_handle_t next = max_id + 1;
  if (next < floor_min) next = floor_min;
  if (next == 0) next = floor_min ? floor_min : 1;
  return next;
}

static int copy_file(const char* src, FILE* out) {
  FILE* f = fopen(src, "rb");
  if (!f) return 0;
  uint8_t buf[1<<16];
  for (;;) {
    size_t n = fread(buf, 1, sizeof(buf), f);
    if (n == 0) break;
    if (fwrite(buf, 1, n, out) != n) { fclose(f); return 0; }
  }
  fclose(f);
  return 1;
}

/* -----------------------------
 * Main
 * ----------------------------- */

int main(int argc, char** argv) {
  const char* in_path = NULL;
  const char* out_path = NULL;
  const char* base_path = NULL;
  uint64_t start_handle_arg = 0; /* optional */
  int have_start_handle = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--in") == 0 && i+1 < argc) { in_path = argv[++i]; continue; }
    if (strcmp(argv[i], "--out") == 0 && i+1 < argc) { out_path = argv[++i]; continue; }
    if (strcmp(argv[i], "--base") == 0 && i+1 < argc) { base_path = argv[++i]; continue; }
    if (strcmp(argv[i], "--start-handle") == 0 && i+1 < argc) {
      if (!parse_u64(argv[++i], &start_handle_arg)) { fprintf(stderr, "bad --start-handle\n"); return 2; }
      have_start_handle = 1;
      continue;
    }
    if (strcmp(argv[i], "--help") == 0) { usage(); return 0; }
    fprintf(stderr, "Unknown arg: %s\n", argv[i]);
    usage();
    return 2;
  }

  if (!in_path || !out_path) { usage(); return 2; }

  /* Replay base (if any) to get max_eid and max_handle */
  sp_mk_t mk;
  sp_status_t s = sp_mk_init(&mk, 1024, 1024);
  if (s < 0) { fprintf(stderr, "mk_init failed: %d\n", (int)s); return 1; }

  sp_eid_t base_max_eid = 0;
  uint64_t base_count = 0;

  if (base_path) {
    s = replay_file_into_mk(base_path, &mk, &base_count, &base_max_eid);
    if (s < 0) {
      fprintf(stderr, "base replay failed: %d\n", (int)s);
      sp_mk_free(&mk);
      return 1;
    }
  }

  /* Choose next handle */
  sp_handle_t floor_min = 0x100; /* readable default */
  if (have_start_handle) floor_min = (sp_handle_t)start_handle_arg;
  sp_handle_t next_handle = compute_next_handle_from_mk(&mk, floor_min);

  /* Open output and copy base */
  FILE* out = fopen(out_path, "wb");
  if (!out) { fprintf(stderr, "cannot open out: %s\n", out_path); sp_mk_free(&mk); return 1; }

  if (base_path) {
    if (!copy_file(base_path, out)) {
      fprintf(stderr, "failed copying base\n");
      fclose(out);
      sp_mk_free(&mk);
      return 1;
    }
  }

  /* Now read proposals and rewrite */
  FILE* in = fopen(in_path, "rb");
  if (!in) { fprintf(stderr, "cannot open in: %s\n", in_path); fclose(out); sp_mk_free(&mk); return 1; }

  sp_eid_t next_eid = base_max_eid + 1;
  sp_tick_t tick = 0;

  for (;;) {
    sp_event_hdr_t hdr;
    size_t r = fread(&hdr, 1, sizeof(hdr), in);
    if (r == 0 && feof(in)) break;
    if (r != sizeof(hdr)) { fprintf(stderr, "truncated header\n"); fclose(in); fclose(out); sp_mk_free(&mk); return 1; }

    uint32_t len = hdr.payload_len;
    uint8_t* payload = NULL;
    if (len) {
      payload = (uint8_t*)malloc(len);
      if (!payload) { fprintf(stderr, "oom\n"); fclose(in); fclose(out); sp_mk_free(&mk); return 1; }
      if (fread(payload, 1, len, in) != len) {
        fprintf(stderr, "truncated payload\n");
        free(payload);
        fclose(in); fclose(out); sp_mk_free(&mk);
        return 1;
      }
    }

    /* Rewrite header fields */
    hdr.eid = next_eid++;
    hdr.tick = ++tick;

    /* Fill POP NEW_OBJ if needed */
    if (hdr.op == SP_OP_POP && len >= sizeof(sp_evt_pop_t)) {
      sp_evt_pop_t* e = (sp_evt_pop_t*)payload;
      if (e->p.a == 0) {
        e->p.a = next_handle++;
      }
    }

    /* Write out rewritten record */
    if (fwrite(&hdr, 1, sizeof(hdr), out) != sizeof(hdr)) {
      fprintf(stderr, "write header failed\n");
      free(payload);
      fclose(in); fclose(out); sp_mk_free(&mk);
      return 1;
    }
    if (len) {
      if (fwrite(payload, 1, len, out) != len) {
        fprintf(stderr, "write payload failed\n");
        free(payload);
        fclose(in); fclose(out); sp_mk_free(&mk);
        return 1;
      }
      free(payload);
    }
  }

  fclose(in);
  fclose(out);

  fprintf(stderr,
    "spcommit-mock: wrote %s (base_eid=%llu, start_eid=%llu, start_handle=0x%llx)\n",
    out_path,
    (unsigned long long)base_max_eid,
    (unsigned long long)(base_max_eid + 1),
    (unsigned long long)floor_min
  );

  sp_mk_free(&mk);
  return 0;
}