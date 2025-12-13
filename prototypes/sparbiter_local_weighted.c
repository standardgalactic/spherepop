#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "spherepop_abi.h"
#include "spherepop_event_layout.h"
#include "sp_mock_kernel.h"
#include "arbiter_report.h"

typedef struct cand {
  char auth[64];
  uint32_t auth_hash;
  uint32_t weight;
  uint32_t arrival;
  sp_event_hdr_t hdr;
  uint8_t* payload;
} cand_t;

static uint32_t fnv1a(const char* s){
  uint32_t h = 2166136261u;
  for (; *s; s++){ h ^= (unsigned char)(*s); h *= 16777619u; }
  return h;
}

static int read_records(const char* path, cand_t** io, size_t* n_io, size_t* cap_io,
                        const char* auth, uint32_t weight, uint32_t* arrival_counter){
  FILE* f = fopen(path, "rb");
  if (!f) return -1;

  for(;;){
    sp_event_hdr_t hdr;
    if (fread(&hdr,1,sizeof(hdr),f) != sizeof(hdr)){
      if (feof(f)) break;
      fclose(f); return -1;
    }
    uint8_t* payload = NULL;
    if (hdr.payload_len){
      payload = (uint8_t*)malloc(hdr.payload_len);
      if (!payload){ fclose(f); return -1; }
      if (fread(payload,1,hdr.payload_len,f) != hdr.payload_len){
        free(payload); fclose(f); return -1;
      }
    }
    if (*n_io == *cap_io){
      size_t nc = (*cap_io ? *cap_io*2 : 64);
      cand_t* nn = (cand_t*)realloc(*io, nc*sizeof(cand_t));
      if (!nn){ free(payload); fclose(f); return -1; }
      *io = nn; *cap_io = nc;
    }
    cand_t* c = &(*io)[(*n_io)++];
    memset(c,0,sizeof(*c));
    strncpy(c->auth, auth, sizeof(c->auth)-1);
    c->auth_hash = fnv1a(auth);
    c->weight = weight;
    c->arrival = (*arrival_counter)++;
    c->hdr = hdr;
    c->payload = payload;
  }

  fclose(f);
  return 0;
}

static int cmp_cand(const void* A, const void* B){
  const cand_t* a = (const cand_t*)A;
  const cand_t* b = (const cand_t*)B;
  if (a->weight != b->weight) return (a->weight > b->weight) ? -1 : 1;
  int s = strcmp(a->auth, b->auth);
  if (s != 0) return s;
  if (a->arrival < b->arrival) return -1;
  if (a->arrival > b->arrival) return 1;
  return 0;
}

static void extract_abc(uint32_t op, const void* payload, uint64_t* a, uint64_t* b, uint64_t* c){
  *a=*b=*c=0;
  if (!payload) return;
  // best-effort: your event layout structs use e->p.{a,b,c} consistently
  if (op == SP_OP_POP) {
    const sp_evt_pop_t* e = (const sp_evt_pop_t*)payload;
    *a = e->p.a; *b = e->p.b; *c = e->p.c;
  } else if (op == SP_OP_MERGE) {
    const sp_evt_merge_t* e = (const sp_evt_merge_t*)payload;
    *a = e->p.a; *b = e->p.b; *c = e->p.c;
  } else if (op == SP_OP_LINK) {
    const sp_evt_link_t* e = (const sp_evt_link_t*)payload;
    *a = e->p.a; *b = e->p.b; *c = e->p.c;
  } else if (op == SP_OP_UNLINK) {
    const sp_evt_unlink_t* e = (const sp_evt_unlink_t*)payload;
    *a = e->p.a; *b = e->p.b; *c = e->p.c;
  }
}

static int copy_file(const char* src, FILE* out){
  FILE* f = fopen(src, "rb");
  if (!f) return -1;
  uint8_t buf[1<<16];
  for(;;){
    size_t n = fread(buf,1,sizeof(buf),f);
    if (!n) break;
    if (fwrite(buf,1,n,out) != n){ fclose(f); return -1; }
  }
  fclose(f);
  return 0;
}

static sp_eid_t scan_max_eid(const char* path){
  FILE* f = fopen(path, "rb");
  if (!f) return 0;
  sp_eid_t max=0;
  for(;;){
    sp_event_hdr_t h;
    if (fread(&h,1,sizeof(h),f) != sizeof(h)){
      break;
    }
    if (h.eid > max) max = h.eid;
    if (h.payload_len) fseek(f, (long)h.payload_len, SEEK_CUR);
  }
  fclose(f);
  return max;
}

static void usage(void){
  fprintf(stderr,
    "Usage:\n"
    "  sparbiter-local-w --base base.bin --out out.bin --report rpt.bin \\\n"
    "    --auth NAME --weight W --prop p1.bin [--auth NAME --weight W --prop p2.bin ...]\n"
  );
}

int main(int argc, char** argv){
  const char* base=NULL;
  const char* outp=NULL;
  const char* rpt=NULL;

  cand_t* cands=NULL; size_t n=0, cap=0;
  uint32_t arrival=0;

  const char* cur_auth=NULL;
  uint32_t cur_w=1;
  const char* cur_prop=NULL;

  for (int i=1;i<argc;i++){
    if (strcmp(argv[i],"--base")==0 && i+1<argc) base=argv[++i];
    else if (strcmp(argv[i],"--out")==0 && i+1<argc) outp=argv[++i];
    else if (strcmp(argv[i],"--report")==0 && i+1<argc) rpt=argv[++i];
    else if (strcmp(argv[i],"--auth")==0 && i+1<argc) cur_auth=argv[++i];
    else if (strcmp(argv[i],"--weight")==0 && i+1<argc) cur_w=(uint32_t)strtoul(argv[++i],NULL,0);
    else if (strcmp(argv[i],"--prop")==0 && i+1<argc) {
      cur_prop=argv[++i];
      if (!cur_auth){ fprintf(stderr,"need --auth before --prop\n"); return 2; }
      if (read_records(cur_prop, &cands, &n, &cap, cur_auth, cur_w, &arrival) != 0){
        fprintf(stderr,"failed reading %s\n", cur_prop); return 1;
      }
      cur_prop=NULL;
    } else {
      usage(); return 2;
    }
  }

  if (!base || !outp || !rpt){ usage(); return 2; }

  // sort candidates by authority weight
  qsort(cands, n, sizeof(cand_t), cmp_cand);

  // replay base into mk
  sp_mk_t mk;
  sp_mk_init(&mk);

  // simplest base replay: apply records directly
  {
    FILE* f = fopen(base, "rb");
    if (!f){ fprintf(stderr,"cannot open base\n"); return 1; }
    for(;;){
      sp_event_hdr_t h;
      if (fread(&h,1,sizeof(h),f)!=sizeof(h)){
        if (feof(f)) break;
        return 1;
      }
      void* pl=NULL;
      if (h.payload_len){
        pl = malloc(h.payload_len);
        fread(pl,1,h.payload_len,f);
      }
      sp_mk_apply(&mk, &h, pl);
      free(pl);
    }
    fclose(f);
  }

  sp_eid_t next_eid = scan_max_eid(base) + 1;
  sp_tick_t tick = 0;

  arb_rpt_entry_t* entries = (arb_rpt_entry_t*)calloc(n, sizeof(*entries));
  if (!entries) return 1;

  FILE* out = fopen(outp, "wb");
  if (!out){ fprintf(stderr,"cannot open out\n"); return 1; }
  if (copy_file(base, out) != 0){ fprintf(stderr,"copy base failed\n"); return 1; }

  // apply candidates to shadow, commit accepted
  for (size_t i=0;i<n;i++){
    cand_t* c = &cands[i];

    uint32_t before_objs = mk.objs_len;
    uint32_t before_rels = mk.rels_len;

    sp_event_hdr_t h = c->hdr;
    h.eid = next_eid++;
    h.tick = ++tick;

    // deterministic POP handle assignment: if NEW_OBJ=0, assign from local counter
    if (h.op == SP_OP_POP && c->payload && h.payload_len >= sizeof(sp_evt_pop_t)){
      sp_evt_pop_t* e = (sp_evt_pop_t*)c->payload;
      if (e->p.a == 0){
        // naive: next handle is max+1 each time (O(n)); acceptable for prototype
        sp_handle_t max=0;
        for (uint32_t k=0;k<mk.objs_len;k++) if (mk.objs[k].id > max) max = mk.objs[k].id;
        e->p.a = (max < 0x100 ? 0x100 : (max+1));
      }
    }

    sp_status_t s = sp_mk_apply(&mk, &h, c->payload);

    arb_rpt_entry_t* r = &entries[i];
    memset(r,0,sizeof(*r));
    r->accepted = (s >= 0);
    r->op = h.op;
    r->err = (s >= 0) ? 0 : (int32_t)s;
    r->weight = c->weight;
    r->authority_hash = c->auth_hash;
    r->eid_assigned = h.eid;
    r->payload_len = h.payload_len;
    extract_abc(h.op, c->payload, &r->a, &r->b, &r->c);

    if (s >= 0){
      r->d_objs = (int32_t)mk.objs_len - (int32_t)before_objs;
      r->d_rels = (int32_t)mk.rels_len - (int32_t)before_rels;

      // commit to output
      fwrite(&h, 1, sizeof(h), out);
      if (h.payload_len) fwrite(c->payload, 1, h.payload_len, out);
    } else {
      // rejected: do NOT write to output (but do keep report)
    }
  }

  fclose(out);

  FILE* rf = fopen(rpt, "wb");
  if (!rf){ fprintf(stderr,"cannot open report\n"); return 1; }
  arb_rpt_hdr_t rh = { ARB_RPT_MAGIC, 1, (uint32_t)n, 0 };
  arb_rpt_write(rf, &rh, entries);
  fclose(rf);

  // cleanup
  for (size_t i=0;i<n;i++) free(cands[i].payload);
  free(cands);
  free(entries);
  sp_mk_free(&mk);
  return 0;
}