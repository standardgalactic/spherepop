#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sp_cli_common.h"

static void usage(void){
  fprintf(stderr,
    "Usage:\n"
    "  spgrep events.bin [--src H] [--dst H] [--type T]\n"
  );
}

int main(int argc, char** argv){
  if (argc < 2){
    usage();
    return 2;
  }

  const char* path = argv[1];
  sp_handle_t src = 0, dst = 0;
  uint32_t type = 0;
  int have_src=0, have_dst=0, have_type=0;

  for (int i = 2; i < argc; i++){
    if (strcmp(argv[i],"--src")==0 && i+1<argc){
      sp_cli_parse_handle(argv[++i], &src);
      have_src=1;
    } else if (strcmp(argv[i],"--dst")==0 && i+1<argc){
      sp_cli_parse_handle(argv[++i], &dst);
      have_dst=1;
    } else if (strcmp(argv[i],"--type")==0 && i+1<argc){
      type = (uint32_t)strtoul(argv[++i], NULL, 0);
      have_type=1;
    } else {
      usage();
      return 2;
    }
  }

  sp_cli_replay_result_t rr;
  if (sp_cli_replay_file(path, &rr) < 0){
    fprintf(stderr, "spgrep: replay failed\n");
    return 1;
  }

  sp_mk_t* mk = &rr.mk;
  for (uint32_t i = 0; i < mk->rels_len; i++){
    const sp_mk_rel_t* r = &mk->rels[i];
    if (have_src && r->a != src) continue;
    if (have_dst && r->b != dst) continue;
    if (have_type && r->reltype != type) continue;

    printf("0x%llx -> 0x%llx  type=%u\n",
      (unsigned long long)r->a,
      (unsigned long long)r->b,
      (unsigned)r->reltype);
  }

  sp_cli_replay_free(&rr);
  return 0;
}