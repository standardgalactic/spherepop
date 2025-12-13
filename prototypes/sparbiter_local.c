#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "spherepop_abi.h"
#include "spherepop_event_layout.h"
#include "sp_mock_kernel.h"
#include "sp_replay.h"

/* --------------------------------------------------
 * File IO glue (same pattern as before)
 * -------------------------------------------------- */

typedef struct file_io { FILE* f; } file_io_t;

static sp_status_t io_read(void* u, void* buf, uint32_t cap, uint32_t* out_n){
  file_io_t* io = (file_io_t*)u;
  size_t n = fread(buf, 1, cap, io->f);
  if (n == 0){
    if (feof(io->f)){ *out_n = 0; return SP_OK; }
    return SP_EIO;
  }
  *out_n = (uint32_t)n;
  return SP_OK;
}

static sp_status_t io_seek(void* u, uint64_t off){
  return fseek(((file_io_t*)u)->f, (long)off, SEEK_SET)==0 ? SP_OK : SP_EIO;
}

static sp_status_t io_tell(void* u, uint64_t* out){
  long p = ftell(((file_io_t*)u)->f);
  if (p < 0) return SP_EIO;
  *out = (uint64_t)p;
  return SP_OK;
}

static sp_status_t mk_dispatch(void* u, const sp_event_hdr_t* h, const void* p){
  return sp_mk_apply((sp_mk_t*)u, h, p);
}

/* --------------------------------------------------
 * Utility helpers
 * -------------------------------------------------- */

static int copy_file(const char* path, FILE* out){
  FILE* f = fopen(path, "rb");
  if (!f) return 0;
  uint8_t buf[1<<15];
  for (;;){
    size_t n = fread(buf, 1, sizeof(buf), f);
    if (!n) break;
    if (fwrite(buf, 1, n, out) != n){
      fclose(f);
      return 0;
    }
  }
  fclose(f);
  return 1;
}

static sp_handle_t next_handle(const sp_mk_t* mk, sp_handle_t floor){
  sp_handle_t max = 0;
  for (uint32_t i=0;i<mk->objs_len;i++)
    if (mk->objs[i].id > max) max = mk->objs[i].id;
  sp_handle_t h = max + 1;
  if (h < floor) h = floor;
  return h;
}

/* --------------------------------------------------
 * CLI
 * -------------------------------------------------- */

static void usage(void){
  fprintf(stderr,
    "Usage:\n"
    "  sparbiter-local --base committed.bin --out new.bin proposal1.bin [proposal2.bin ...]\n"
    "\n"
    "Deterministically orders and commits proposal logs.\n"
  );
}

int main(int argc, char** argv){
  if (argc < 6 || strcmp(argv[1],"--base")!=0 || strcmp(argv[3],"--out")!=0){
    usage();
    return 2;
  }

  const char* base_path = argv[2];
  const char* out_path  = argv[4];

  /* Replay base into kernel */
  sp_mk_t mk;
  sp_mk_init(&mk);

  sp_eid_t max_eid = 0;
  {
    FILE* f = fopen(base_path, "rb");
    if (!f){ fprintf(stderr,"cannot open base\n"); return 1; }
    file_io_t io = { .f = f };

    sp_replay_ctx_t ctx = {0};
    ctx.io.user = &io;
    ctx.io.read = io_read;
    ctx.io.seek = io_seek;
    ctx.io.tell = io_tell;

    sp_replay_handlers_t h = {0};
    h.user = &mk;
    h.on_event = mk_dispatch;

    for (;;){
      sp_status_t s = sp_replay_next(&ctx, &h);
      if (s == SP_ENOENT) break;
      if (s < 0){ fprintf(stderr,"base replay failed\n"); return 1; }
    }
    fclose(f);
  }

  /* Scan base for max eid */
  {
    FILE* f = fopen(base_path, "rb");
    while (f){
      sp_event_hdr_t hdr;
      if (fread(&hdr,1,sizeof(hdr),f)!=sizeof(hdr)) break;
      if (hdr.eid > max_eid) max_eid = hdr.eid;
      fseek(f, hdr.payload_len, SEEK_CUR);
    }
    if (f) fclose(f);
  }

  sp_handle_t next_h = next_handle(&mk, 0x100);
  sp_eid_t next_eid = max_eid + 1;
  sp_tick_t tick = 0;

  FILE* out = fopen(out_path, "wb");
  if (!out){ fprintf(stderr,"cannot open out\n"); return 1; }

  if (!copy_file(base_path, out)){
    fprintf(stderr,"copy base failed\n");
    return 1;
  }

  /* Commit proposals in argv order */
  for (int pi = 5; pi < argc; pi++){
    FILE* in = fopen(argv[pi], "rb");
    if (!in){ fprintf(stderr,"cannot open proposal %s\n", argv[pi]); return 1; }

    for (;;){
      sp_event_hdr_t hdr;
      if (fread(&hdr,1,sizeof(hdr),in)!=sizeof(hdr)){
        if (feof(in)) break;
        fprintf(stderr,"bad proposal hdr\n");
        return 1;
      }

      void* payload = NULL;
      if (hdr.payload_len){
        payload = malloc(hdr.payload_len);
        fread(payload,1,hdr.payload_len,in);
      }

      hdr.eid = next_eid++;
      hdr.tick = ++tick;

      if (hdr.op == SP_OP_POP && payload){
        sp_evt_pop_t* e = (sp_evt_pop_t*)payload;
        if (e->p.a == 0)
          e->p.a = next_h++;
      }

      fwrite(&hdr,1,sizeof(hdr),out);
      if (payload){
        fwrite(payload,1,hdr.payload_len,out);
        free(payload);
      }
    }
    fclose(in);
  }

  fclose(out);
  sp_mk_free(&mk);

  fprintf(stderr,
    "sparbiter-local: committed %llu events, next_handle=0x%llx\n",
    (unsigned long long)(next_eid - max_eid - 1),
    (unsigned long long)next_h
  );

  return 0;
}