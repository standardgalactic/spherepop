#include <stdio.h>
#include <stdlib.h>

#include "sp_cli_common.h"

static void usage(void){
  fprintf(stderr,
    "Usage:\n"
    "  spls events.bin\n"
    "\n"
    "Lists objects, representatives, and relations from replay.\n"
  );
}

int main(int argc, char** argv){
  if (argc != 2 || strcmp(argv[1], "--help") == 0){
    usage();
    return (argc==2)?0:2;
  }

  sp_cli_replay_result_t rr;
  sp_status_t s = sp_cli_replay_file(argv[1], &rr);
  if (s < 0){
    fprintf(stderr, "spls: replay failed: %d\n", (int)s);
    return 1;
  }

  sp_mk_t* mk = &rr.mk;

  printf("Objects:\n");
  for (uint32_t i = 0; i < mk->objs_len; i++){
    sp_handle_t id = mk->objs[i].id;
    sp_handle_t rep = sp_mk_find(mk, id);
    printf("  0x%llx  rep=0x%llx\n",
      (unsigned long long)id,
      (unsigned long long)rep);
  }

  printf("\nRelations:\n");
  for (uint32_t i = 0; i < mk->rels_len; i++){
    const sp_mk_rel_t* r = &mk->rels[i];
    printf("  (0x%llx -> 0x%llx) type=%u\n",
      (unsigned long long)r->a,
      (unsigned long long)r->b,
      (unsigned)r->reltype);
  }

  sp_cli_replay_free(&rr);
  return 0;
}