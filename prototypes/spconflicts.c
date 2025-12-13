#include <stdio.h>
#include <stdlib.h>
#include "arbiter_report.h"

static const char* opn(uint32_t op){
  switch(op){
    default: return "OP?";
  }
}

int main(int argc, char** argv){
  if (argc != 2){
    fprintf(stderr, "Usage: spconflicts report.bin\n");
    return 2;
  }
  FILE* f = fopen(argv[1], "rb");
  if (!f){ perror("open"); return 1; }

  arb_rpt_hdr_t h;
  if (fread(&h, 1, sizeof(h), f) != sizeof(h)){ fprintf(stderr,"bad hdr\n"); return 1; }
  if (h.magic != ARB_RPT_MAGIC){ fprintf(stderr,"bad magic\n"); return 1; }

  arb_rpt_entry_t* e = (arb_rpt_entry_t*)calloc(h.n_entries, sizeof(*e));
  if (!e) return 1;
  if (h.n_entries && fread(e, sizeof(*e), h.n_entries, f) != h.n_entries){
    fprintf(stderr,"bad entries\n"); free(e); return 1;
  }
  fclose(f);

  printf("Conflicts (rejected entries):\n");
  for (uint32_t i=0;i<h.n_entries;i++){
    if (e[i].accepted) continue;
    printf("  eid=%llu op=%u(%s) err=%d w=%u a=0x%llx b=0x%llx c=0x%llx\n",
      (unsigned long long)e[i].eid_assigned,
      e[i].op, opn(e[i].op),
      (int)e[i].err,
      (unsigned)e[i].weight,
      (unsigned long long)e[i].a,
      (unsigned long long)e[i].b,
      (unsigned long long)e[i].c
    );
  }

  free(e);
  return 0;
}