#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sp_cli_common.h"

static void usage(void) {
  fprintf(stderr,
    "Usage:\n"
    "  sppop --out proposal.bin N\n"
    "\n"
    "Emits N SP_OP_POP proposals (NEW_OBJ filled by kernel when committed).\n"
  );
}

int main(int argc, char** argv) {
  const char* out_path = NULL;
  int i = 1;
  for (; i < argc; i++) {
    if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) { out_path = argv[++i]; continue; }
    if (strcmp(argv[i], "--help") == 0) { usage(); return 0; }
    break;
  }
  if (!out_path || i >= argc) { usage(); return 2; }

  long n = strtol(argv[i], NULL, 10);
  if (n <= 0) { fprintf(stderr, "sppop: N must be > 0\n"); return 2; }

  FILE* out = fopen(out_path, "wb");
  if (!out) { fprintf(stderr, "sppop: cannot open %s\n", out_path); return 1; }

  for (long k = 0; k < n; k++) {
    sp_evt_pop_t ev = {0};
    ev.p.a = 0;          /* NEW_OBJ placeholder */
    ev.p.b = 0;          /* parent (optional) */
    ev.objtype = 0;
    ev.link_flags = 0;
    ev.reltype_override = 0;
    ev.has_meta = 0;

    sp_status_t s = sp_cli_write_record(out, SP_OP_POP, 0, 0, &ev, (uint32_t)sizeof(ev));
    if (s < 0) { fprintf(stderr, "sppop: write failed: %d\n", (int)s); fclose(out); return 1; }
  }

  fclose(out);
  fprintf(stderr, "sppop: wrote %ld POP proposals to %s\n", n, out_path);
  return 0;
}