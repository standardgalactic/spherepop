/* sp_inject_server.c
 *
 * Minimal injection arbiter:
 *  - reads proposal lines (stdin)
 *  - converts to canonical event (hdr + payload)
 *  - appends to events.bin
 *  - applies to mock kernel
 *  - emits incremental diffs (stdout)
 *
 * NOTE: NDJSON parsing here is intentionally simplistic.
 * In practice, swap with a small JSON parser (jsmn, yyjson).
 */
#include "sp_mock_kernel.h"
#include "sp_mk_diff.h"
#include "sp_layout_hint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static sp_eid_t g_seq = 1;

/* -------- Diff sink (NDJSON) with origin correlation -------- */

typedef struct {
  FILE* out;
  sp_eid_t seq;
  char origin_client[64];
  uint64_t origin_req;
} diff_ctx_t;

static void d_begin(void* ctx, sp_eid_t seq) { ((diff_ctx_t*)ctx)->seq = seq; }

static void d_add_obj(void* ctx, sp_handle_t id, sp_objtype_t type, sp_handle_t rep) {
  diff_ctx_t* c = ctx;
  fprintf(c->out,
    "{\"seq\":%llu,\"origin\":{\"client\":\"%s\",\"req\":%llu},"
    "\"add_obj\":{\"id\":%llu,\"type\":%u,\"rep\":%llu}}\n",
    (unsigned long long)c->seq, c->origin_client, (unsigned long long)c->origin_req,
    (unsigned long long)id, (unsigned)type, (unsigned long long)rep);
}

static void d_rem_obj(void* ctx, sp_handle_t id) {
  diff_ctx_t* c = ctx;
  fprintf(c->out,
    "{\"seq\":%llu,\"origin\":{\"client\":\"%s\",\"req\":%llu},\"rem_obj\":%llu}\n",
    (unsigned long long)c->seq, c->origin_client, (unsigned long long)c->origin_req,
    (unsigned long long)id);
}

static void d_add_edge(void* ctx, sp_handle_t src, sp_handle_t dst, sp_reltype_t rel, uint32_t flags) {
  diff_ctx_t* c = ctx;
  fprintf(c->out,
    "{\"seq\":%llu,\"origin\":{\"client\":\"%s\",\"req\":%llu},"
    "\"add_edge\":{\"src\":%llu,\"dst\":%llu,\"rel\":%u,\"flags\":%u}}\n",
    (unsigned long long)c->seq, c->origin_client, (unsigned long long)c->origin_req,
    (unsigned long long)src, (unsigned long long)dst, (unsigned)rel, flags);
}

static void d_rem_edge(void* ctx, sp_handle_t src, sp_handle_t dst, sp_reltype_t rel) {
  diff_ctx_t* c = ctx;
  fprintf(c->out,
    "{\"seq\":%llu,\"origin\":{\"client\":\"%s\",\"req\":%llu},"
    "\"rem_edge\":{\"src\":%llu,\"dst\":%llu,\"rel\":%u}}\n",
    (unsigned long long)c->seq, c->origin_client, (unsigned long long)c->origin_req,
    (unsigned long long)src, (unsigned long long)dst, (unsigned)rel);
}

static void d_end(void* ctx) { (void)ctx; fflush(((diff_ctx_t*)ctx)->out); }

static sp_mk_diff_sink_t mk_sink(diff_ctx_t* dc) {
  return (sp_mk_diff_sink_t){
    .ctx = dc,
    .begin = d_begin,
    .add_obj = d_add_obj,
    .rem_obj = d_rem_obj,
    .add_edge = d_add_edge,
    .rem_edge = d_rem_edge,
    .end = d_end
  };
}

/* -------- Canonical event writing -------- */

static int write_event(FILE* f, const sp_event_hdr_t* hdr, const void* payload) {
  if (fwrite(hdr, 1, sizeof(*hdr), f) != sizeof(*hdr)) return 0;
  if (hdr->payload_len) {
    if (fwrite(payload, 1, hdr->payload_len, f) != hdr->payload_len) return 0;
  }
  fflush(f);
  return 1;
}

/* -------- Minimal proposal parsing (MVP) --------
 * This is intentionally crude: expects lines containing op and numeric args.
 * Replace with real JSON parsing when ready.
 */
static int find_u64(const char* s, const char* key, uint64_t* out);
static int find_str(const char* s, const char* key, char* out, size_t cap);

static int find_u64(const char* s, const char* key, uint64_t* out) {
  const char* p = strstr(s, key);
  if (!p) return 0;
  p = strchr(p, ':');
  if (!p) return 0;
  p++;
  while (*p == ' ' || *p == '\"') p++;
  *out = (uint64_t)strtoull(p, NULL, 10);
  return 1;
}

static int find_str(const char* s, const char* key, char* out, size_t cap) {
  const char* p = strstr(s, key);
  if (!p) return 0;
  p = strchr(p, ':');
  if (!p) return 0;
  p++;
  while (*p == ' ' || *p == '\"') p++;
  const char* q = strchr(p, '\"');
  if (!q) return 0;
  size_t n = (size_t)(q - p);
  if (n + 1 > cap) return 0;
  memcpy(out, p, n);
  out[n] = 0;
  return 1;
}

int main(int argc, char** argv) {
  const char* log_path = (argc >= 2) ? argv[1] : "events.bin";
  FILE* flog = fopen(log_path, "ab");
  if (!flog) { perror("events.bin"); return 2; }

  sp_mk_t mk;
  if (sp_mk_init(&mk, 256, 512) != SP_OK) return 2;

  diff_ctx_t dc = { .out = stdout, .origin_req = 0 };
  dc.origin_client[0] = 0;
  sp_mk_diff_sink_t sink = mk_sink(&dc);

  char line[4096];
  while (fgets(line, sizeof(line), stdin)) {
    /* Extract origin */
    find_str(line, "\"client\"", dc.origin_client, sizeof(dc.origin_client));
    find_u64(line, "\"req\"", &dc.origin_req);

    char op[32] = {0};
    if (!find_str(line, "\"op\"", op, sizeof(op))) {
      fprintf(stdout, "{\"t\":\"ack\",\"req\":%llu,\"status\":\"err\",\"code\":\"EINVAL\"}\n",
              (unsigned long long)dc.origin_req);
      continue;
    }

    sp_event_hdr_t hdr = {0};
    hdr.eid = g_seq++;
    hdr.tick = 0;
    hdr.flags = SP_F_DETERMINISTIC | SP_F_RECORD_EVENT;
    hdr.user_tag = dc.origin_req;

    uint8_t payload[512];
    memset(payload, 0, sizeof(payload));

    if (strcmp(op, "MERGE") == 0) {
      uint64_t a,b,mode=SP_MG_DEFAULT;
      if (!find_u64(line, "\"a\"", &a) || !find_u64(line, "\"b\"", &b)) goto bad;
      find_u64(line, "\"mode\"", &mode);

      sp_evt_merge_t* e = (sp_evt_merge_t*)payload;
      e->p.a = (sp_handle_t)a;
      e->p.b = (sp_handle_t)b;
      e->p.c = 0;
      e->mode = (uint32_t)mode;
      e->has_policy = 0;

      hdr.op = SP_OP_MERGE;
      hdr.payload_len = sizeof(sp_evt_merge_t);

    } else if (strcmp(op, "LINK") == 0) {
      uint64_t src,dst,rel,flags=0;
      if (!find_u64(line, "\"src\"", &src) || !find_u64(line, "\"dst\"", &dst) || !find_u64(line, "\"rel\"", &rel)) goto bad;
      find_u64(line, "\"flags\"", &flags);

      sp_evt_link_t* e = (sp_evt_link_t*)payload;
      e->p.a = (sp_handle_t)src;
      e->p.b = (sp_handle_t)dst;
      e->p.c = 0;
      e->reltype = (uint32_t)rel;
      e->link_flags = (uint32_t)flags;
      e->has_meta = 0;

      hdr.op = SP_OP_LINK;
      hdr.payload_len = sizeof(sp_evt_link_t);

    } else if (strcmp(op, "UNLINK") == 0) {
      uint64_t src,dst,rel;
      if (!find_u64(line, "\"src\"", &src) || !find_u64(line, "\"dst\"", &dst) || !find_u64(line, "\"rel\"", &rel)) goto bad;

      sp_evt_unlink_t* e = (sp_evt_unlink_t*)payload;
      e->p.a = (sp_handle_t)src;
      e->p.b = (sp_handle_t)dst;
      e->reltype = (uint32_t)rel;

      hdr.op = SP_OP_UNLINK;
      hdr.payload_len = sizeof(sp_evt_unlink_t);

    } else if (strcmp(op, "COLLAPSE") == 0) {
      uint64_t region, cflags=0;
      if (!find_u64(line, "\"region\"", &region)) goto bad;
      find_u64(line, "\"collapse_flags\"", &cflags);

      sp_evt_collapse_t* e = (sp_evt_collapse_t*)payload;
      e->p.a = (sp_handle_t)region;
      e->p.b = 0; /* representative (optional) */
      e->collapse_flags = (uint32_t)cflags;
      e->has_policy = 0;

      hdr.op = SP_OP_COLLAPSE;
      hdr.payload_len = sizeof(sp_evt_collapse_t);

    } else if (strcmp(op, "SET_LAYOUT") == 0) {
      /* We encode SET_LAYOUT as SET_META with SP_MT_LAYOUT_HINT bytes.
         For MVP we accept x,y,z,radius,flags as float-ish numbers. */
      uint64_t id;
      if (!find_u64(line, "\"id\"", &id)) goto bad;

      /* crude float parsing (replace later) */
      float x=0,y=0,z=0,r=0.5f;
      uint64_t lhflags=0;
      /* parse as integers if present; keep MVP simple */
      uint64_t xi=0, yi=0, zi=0, ri=0;
      find_u64(line, "\"x\"", &xi);
      find_u64(line, "\"y\"", &yi);
      find_u64(line, "\"z\"", &zi);
      find_u64(line, "\"radius\"", &ri);
      find_u64(line, "\"lhflags\"", &lhflags);
      x = (float)((int64_t)xi) / 1000.0f;
      y = (float)((int64_t)yi) / 1000.0f;
      z = (float)((int64_t)zi) / 1000.0f;
      if (ri) r = (float)ri / 1000.0f;

      /* Build payload: sp_evt_set_meta_t + inline layout bytes */
      struct __attribute__((packed)) {
        sp_evt_set_meta_t e;
        sp_layout_hint_t lh;
      } p2;

      memset(&p2, 0, sizeof(p2));
      p2.e.p.a = (sp_handle_t)id;
      p2.e.has_meta = 1;
      p2.e.meta.type = SP_MT_LAYOUT_HINT;
      p2.e.meta.len  = sizeof(sp_layout_hint_t);
      p2.e.meta.off  = (uint32_t)offsetof(typeof(p2), lh);

      p2.lh.version = SP_LAYOUT_VERSION;
      p2.lh.flags   = (uint32_t)lhflags;
      p2.lh.x = x; p2.lh.y = y; p2.lh.z = z;
      p2.lh.radius = r;

      hdr.op = SP_OP_SET_META;
      hdr.payload_len = sizeof(p2);

      /* copy to payload buffer */
      if (hdr.payload_len > sizeof(payload)) { goto bad; }
      memcpy(payload, &p2, sizeof(p2));

    } else {
      goto bad;
    }

    /* Commit: write event, apply, emit diffs */
    if (!write_event(flog, &hdr, payload)) {
      fprintf(stdout, "{\"t\":\"ack\",\"req\":%llu,\"status\":\"err\",\"code\":\"EIO\"}\n",
              (unsigned long long)dc.origin_req);
      continue;
    }

    sp_status_t as = sp_mk_apply_with_diff(&mk, &hdr, payload, &sink);
    if (as != SP_OK) {
      fprintf(stdout, "{\"t\":\"ack\",\"req\":%llu,\"status\":\"err\",\"code\":\"EAPPLY\"}\n",
              (unsigned long long)dc.origin_req);
      continue;
    }

    fprintf(stdout, "{\"t\":\"ack\",\"req\":%llu,\"status\":\"ok\",\"seq\":%llu}\n",
            (unsigned long long)dc.origin_req,
            (unsigned long long)hdr.eid);
    fflush(stdout);
    continue;

  bad:
    fprintf(stdout, "{\"t\":\"ack\",\"req\":%llu,\"status\":\"err\",\"code\":\"EINVAL\"}\n",
            (unsigned long long)dc.origin_req);
    fflush(stdout);
  }

  sp_mk_free(&mk);
  fclose(flog);
  return 0;
}