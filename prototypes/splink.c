#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sp_cli_common.h"

static void usage(void) {
  fprintf(stderr,
    "Usage:\n"
    "  splink --in events.bin --out proposal.bin SRC DST RELTYPE\n"
    "\n"
    "RELTYPE is uint32 (decimal or 0xHEX).\n"
  );
}

static int parse_u32(const char* s, uint32_t* out) {
  if (!s || !out) return 0;
  char* end = NULL;
  unsigned long v = 0;
  if (s[0]=='0' && (s[1]=='x'||s[1]=='X')) v = strtoul(s+2, &end, 16);
  else v = strtoul(s, &end, 10);
  if (!end || *end!='\0') return 0;
  *out = (uint32_t)v;
  return 1;
}

int main(int argc, char** argv) {
  const char* in_path = NULL;
  const char* out_path = NULL;

  int i = 1;
  for (; i < argc; i++) {
    if (strcmp(argv[i], "--in") == 0 && i + 1 < argc) { in_path = argv[++i]; continue; }
    if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) { out_path = argv[++i]; continue; }
    if (strcmp(argv[i], "--help") == 0) { usage(); return 0; }
    break;
  }

  if (!in_path || !out_path || (argc - i) != 3) { usage(); return 2; }

  sp_handle_t src=0, dst=0;
  uint32_t reltype=0;

  if (!sp_cli_parse_handle(argv[i+0], &src) || !sp_cli_parse_handle(argv[i+1], &dst)) {
    fprintf(stderr, "splink: invalid SRC/DST\n"); return 2;
  }
  if (!parse_u32(argv[i+2], &reltype)) {
    fprintf(stderr, "splink: invalid RELTYPE\n"); return 2;
  }

  sp_cli_replay_result_t rr;
  sp_status_t s = sp_cli_replay_file(in_path, &rr);
  if (s < 0) { fprintf(stderr, "splink: replay failed: %d\n", (int)s); return 1; }

  if (!sp_mk_get_obj(&rr.mk, src) || !sp_mk_get_obj(&rr.mk, dst)) {
    fprintf(stderr, "splink: SRC/DST not found in replayed state\n");
    sp_cli_replay_free(&rr);
    return 2;
  }

  FILE* out = fopen(out_path, "wb");
  if (!out) { fprintf(stderr, "splink: cannot open %s\n", out_path); sp_cli_replay_free(&rr); return 1; }

  sp_evt_link_t ev = {0};
  ev.p.a = src;
  ev.p.b = dst;
  ev.p.c = 0;            /* order */
  ev.reltype = reltype;
  ev.link_flags = 0;
  ev.has_meta = 0;

  s = sp_cli_write_record(out, SP_OP_LINK, 0, 0, &ev, (uint32_t)sizeof(ev));
  if (s < 0) { fprintf(stderr, "splink: write failed: %d\n", (int)s); fclose(out); sp_cli_replay_free(&rr); return 1; }

  fclose(out);
  sp_cli_replay_free(&rr);
  fprintf(stderr, "splink: wrote LINK proposal to %s\n", out_path);
  return 0;
}