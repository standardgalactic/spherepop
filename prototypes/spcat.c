#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "spherepop_abi.h"
#include "spherepop_event_layout.h"

static const char* op_name(uint32_t op) {
  switch (op) {
    case SP_OP_POP:      return "POP";
    case SP_OP_MERGE:    return "MERGE";
    case SP_OP_LINK:     return "LINK";
    case SP_OP_UNLINK:   return "UNLINK";
    case SP_OP_COLLAPSE: return "COLLAPSE";
    case SP_OP_SETMETA:  return "SETMETA";
    default:             return "UNKNOWN";
  }
}

static void print_pop(const sp_evt_pop_t* e) {
  printf("  new_obj=0x%llx parent=0x%llx type=%u\n",
         (unsigned long long)e->p.a,
         (unsigned long long)e->p.b,
         (unsigned)e->objtype);
}

static void print_merge(const sp_evt_merge_t* e) {
  printf("  a=0x%llx b=0x%llx into=0x%llx\n",
         (unsigned long long)e->p.a,
         (unsigned long long)e->p.b,
         (unsigned long long)e->p.c);
}

static void print_link(const sp_evt_link_t* e) {
  printf("  src=0x%llx dst=0x%llx reltype=%u\n",
         (unsigned long long)e->p.a,
         (unsigned long long)e->p.b,
         (unsigned)e->reltype);
}

static void usage(void) {
  fprintf(stderr,
    "Usage:\n"
    "  spcat events.bin\n"
    "\n"
    "Prints Spherepop event records in a readable form.\n"
  );
}

int main(int argc, char** argv) {
  if (argc != 2 || strcmp(argv[1], "--help") == 0) {
    usage();
    return (argc == 2) ? 0 : 2;
  }

  const char* path = argv[1];
  FILE* f = fopen(path, "rb");
  if (!f) {
    fprintf(stderr, "spcat: cannot open %s\n", path);
    return 1;
  }

  for (;;) {
    sp_event_hdr_t hdr;
    size_t r = fread(&hdr, 1, sizeof(hdr), f);
    if (r == 0 && feof(f)) break;
    if (r != sizeof(hdr)) {
      fprintf(stderr, "spcat: truncated header\n");
      fclose(f);
      return 1;
    }

    uint8_t* payload = NULL;
    if (hdr.payload_len) {
      payload = (uint8_t*)malloc(hdr.payload_len);
      if (!payload) {
        fprintf(stderr, "spcat: out of memory\n");
        fclose(f);
        return 1;
      }
      if (fread(payload, 1, hdr.payload_len, f) != hdr.payload_len) {
        fprintf(stderr, "spcat: truncated payload\n");
        free(payload);
        fclose(f);
        return 1;
      }
    }

    printf("EID=%llu tick=%llu op=%s(%u) payload=%u\n",
           (unsigned long long)hdr.eid,
           (unsigned long long)hdr.tick,
           op_name(hdr.op),
           hdr.op,
           hdr.payload_len);

    switch (hdr.op) {
      case SP_OP_POP:
        if (hdr.payload_len >= sizeof(sp_evt_pop_t))
          print_pop((const sp_evt_pop_t*)payload);
        break;

      case SP_OP_MERGE:
        if (hdr.payload_len >= sizeof(sp_evt_merge_t))
          print_merge((const sp_evt_merge_t*)payload);
        break;

      case SP_OP_LINK:
        if (hdr.payload_len >= sizeof(sp_evt_link_t))
          print_link((const sp_evt_link_t*)payload);
        break;

      default:
        /* Unknown ops are printed header-only */
        break;
    }

    if (payload) free(payload);
  }

  fclose(f);
  return 0;
}