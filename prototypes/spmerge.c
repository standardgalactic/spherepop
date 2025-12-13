#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sp_cli_common.h"

static void usage(void) {
  fprintf(stderr,
    "Usage:\n"
    "  spmerge --in events.bin --out proposal.bin A B [C ...]\n"
    "\n"
    "Emits SP_OP_MERGE proposals to unify B.. into A (chain merge).\n"
    "Validates handles by replaying --in into the mock kernel.\n"
    "\n"
    "Notes:\n"
    "  - This does not commit anything; it only writes proposal events.\n"
    "  - If two handles are already equivalent under replay, spmerge errors.\n"
  );
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

  if (!in_path || !out_path) { usage(); return 2; }
  if (i >= argc) { usage(); return 2; }

  /* Parse handles */
  sp_handle_t base = 0;
  if (!sp_cli_parse_handle(argv[i], &base) || base == 0) {
    fprintf(stderr, "spmerge: invalid base handle: %s\n", argv[i]);
    return 2;
  }
  i++;

  if (i >= argc) {
    fprintf(stderr, "spmerge: need at least two handles\n");
    return 2;
  }

  sp_cli_replay_result_t rr;
  sp_status_t s = sp_cli_replay_file(in_path, &rr);
  if (s < 0) {
    fprintf(stderr, "spmerge: replay failed: %d\n", (int)s);
    return 1;
  }

  /* Validate base exists */
  if (!sp_mk_get_obj(&rr.mk, base)) {
    fprintf(stderr, "spmerge: base handle not found in replayed state: %llu\n",
            (unsigned long long)base);
    sp_cli_replay_free(&rr);
    return 2;
  }

  FILE* out = fopen(out_path, "wb");
  if (!out) {
    fprintf(stderr, "spmerge: cannot open output: %s\n", out_path);
    sp_cli_replay_free(&rr);
    return 1;
  }

  /* Deterministic chain merge: MERGE(base, h_i) for each i */
  for (; i < argc; i++) {
    sp_handle_t b = 0;
    if (!sp_cli_parse_handle(argv[i], &b) || b == 0) {
      fprintf(stderr, "spmerge: invalid handle: %s\n", argv[i]);
      fclose(out);
      sp_cli_replay_free(&rr);
      return 2;
    }
    if (!sp_mk_get_obj(&rr.mk, b)) {
      fprintf(stderr, "spmerge: handle not found in replayed state: %llu\n",
              (unsigned long long)b);
      fclose(out);
      sp_cli_replay_free(&rr);
      return 2;
    }

    sp_handle_t ra = sp_mk_find(&rr.mk, base);
    sp_handle_t rb = sp_mk_find(&rr.mk, b);
    if (ra == rb) {
      fprintf(stderr, "spmerge: redundant: %llu already equivalent to %llu (rep=%llu)\n",
              (unsigned long long)b, (unsigned long long)base, (unsigned long long)ra);
      fclose(out);
      sp_cli_replay_free(&rr);
      return 2;
    }

    sp_evt_merge_t ev = {0};
    ev.p.a = base;
    ev.p.b = b;
    ev.p.c = 0;         /* into = 0 means kernel default */
    ev.mode = 0;        /* SP_MG_* default (set later if needed) */
    ev.has_policy = 0;

    s = sp_cli_write_record(out, SP_OP_MERGE, 0, /*user_tag=*/0, &ev, (uint32_t)sizeof(ev));
    if (s < 0) {
      fprintf(stderr, "spmerge: write failed: %d\n", (int)s);
      fclose(out);
      sp_cli_replay_free(&rr);
      return 1;
    }
  }

  fclose(out);
  fprintf(stderr, "spmerge: wrote proposals to %s (replayed %llu events)\n",
          out_path, (unsigned long long)rr.events_replayed);

  sp_cli_replay_free(&rr);
  return 0;
}