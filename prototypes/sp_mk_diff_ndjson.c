/* sp_mk_diff_ndjson.c */
#include "sp_mk_diff.h"
#include <stdio.h>

typedef struct {
  FILE* out;
  sp_eid_t seq;
} ndjson_ctx_t;

static void begin(void* ctx, sp_eid_t seq) {
  ((ndjson_ctx_t*)ctx)->seq = seq;
}

static void add_obj(void* ctx, sp_handle_t id, sp_objtype_t type, sp_handle_t rep) {
  ndjson_ctx_t* c = ctx;
  fprintf(c->out,
    "{\"seq\":%llu,\"add_obj\":{\"id\":%llu,\"type\":%u,\"rep\":%llu}}\n",
    (unsigned long long)c->seq,
    (unsigned long long)id,
    (unsigned)type,
    (unsigned long long)rep);
}

static void rem_obj(void* ctx, sp_handle_t id) {
  ndjson_ctx_t* c = ctx;
  fprintf(c->out,
    "{\"seq\":%llu,\"rem_obj\":%llu}\n",
    (unsigned long long)c->seq,
    (unsigned long long)id);
}

static void add_edge(void* ctx,
                     sp_handle_t src, sp_handle_t dst,
                     sp_reltype_t rel, uint32_t flags) {
  ndjson_ctx_t* c = ctx;
  fprintf(c->out,
    "{\"seq\":%llu,\"add_edge\":{\"src\":%llu,\"dst\":%llu,\"rel\":%u,\"flags\":%u}}\n",
    (unsigned long long)c->seq,
    (unsigned long long)src,
    (unsigned long long)dst,
    (unsigned)rel,
    flags);
}

static void rem_edge(void* ctx,
                     sp_handle_t src, sp_handle_t dst,
                     sp_reltype_t rel) {
  ndjson_ctx_t* c = ctx;
  fprintf(c->out,
    "{\"seq\":%llu,\"rem_edge\":{\"src\":%llu,\"dst\":%llu,\"rel\":%u}}\n",
    (unsigned long long)c->seq,
    (unsigned long long)src,
    (unsigned long long)dst,
    (unsigned)rel);
}

static void end(void* ctx) { (void)ctx; }

sp_mk_diff_sink_t sp_mk_ndjson_sink(FILE* out) {
  static ndjson_ctx_t ctx;
  ctx.out = out;
  return (sp_mk_diff_sink_t){
    .ctx = &ctx,
    .begin = begin,
    .add_obj = add_obj,
    .rem_obj = rem_obj,
    .add_edge = add_edge,
    .rem_edge = rem_edge,
    .end = end
  };
}