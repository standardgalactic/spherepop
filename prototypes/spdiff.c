#include <stdio.h>
#include <stdlib.h>

#include "sp_cli_common.h"

static void usage(void){
  fprintf(stderr,
    "Usage:\n"
    "  spdiff events.bin\n"
    "\n"
    "Prints semantic diffs per event during replay.\n"
  );
}

static void print_diff(sp_mk_t* before, sp_mk_t* after){
  if (after->objs_len != before->objs_len){
    printf("  objects: %u -> %u\n",
      before->objs_len, after->objs_len);
  }
  if (after->rels_len != before->rels_len){
    printf("  relations: %u -> %u\n",
      before->rels_len, after->rels_len);
  }
}

int main(int argc, char** argv){
  if (argc != 2 || strcmp(argv[1], "--help") == 0){
    usage();
    return (argc==2)?0:2;
  }

  FILE* f = fopen(argv[1], "rb");
  if (!f){
    fprintf(stderr, "spdiff: cannot open %s\n", argv[1]);
    return 1;
  }

  sp_mk_t mk;
  sp_mk_init(&mk);

  for (;;){
    sp_event_hdr_t hdr;
    if (fread(&hdr, 1, sizeof(hdr), f) != sizeof(hdr)){
      break;
    }

    void* payload = NULL;
    if (hdr.payload_len){
      payload = malloc(hdr.payload_len);
      fread(payload, 1, hdr.payload_len, f);
    }

    sp_mk_t before = mk; /* shallow copy is fine for counts */
    sp_mk_apply(&mk, &hdr, payload);

    printf("EID=%llu op=%u\n",
      (unsigned long long)hdr.eid,
      hdr.op);

    print_diff(&before, &mk);

    if (payload) free(payload);
  }

  fclose(f);
  sp_mk_free(&mk);
  return 0;
}