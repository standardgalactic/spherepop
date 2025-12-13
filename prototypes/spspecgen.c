#include <stdio.h>
#include <stdint.h>
#include "spherepop_abi.h"
#include "spherepop_event_layout.h"

static void tex_hdr(FILE* o){
  fprintf(o,
    "\\documentclass[11pt]{article}\n"
    "\\usepackage[T1]{fontenc}\n"
    "\\usepackage{lmodern}\n"
    "\\usepackage{geometry}\n"
    "\\usepackage{longtable}\n"
    "\\geometry{margin=1in}\n"
    "\\begin{document}\n"
    "\\section*{Spherepop ABI Spec (Generated)}\n"
  );
}

static void tex_footer(FILE* o){
  fprintf(o, "\\end{document}\n");
}

static void tex_op_table(FILE* o){
  fprintf(o, "\\section*{Opcodes}\n");
  fprintf(o, "\\begin{longtable}{ll}\n");
  fprintf(o, "Name & Value\\\\\\hline\n");
  fprintf(o, "POP & %u\\\\\n", (unsigned)SP_OP_POP);
  fprintf(o, "MERGE & %u\\\\\n", (unsigned)SP_OP_MERGE);
  fprintf(o, "LINK & %u\\\\\n", (unsigned)SP_OP_LINK);
  fprintf(o, "UNLINK & %u\\\\\n", (unsigned)SP_OP_UNLINK);
  fprintf(o, "COLLAPSE & %u\\\\\n", (unsigned)SP_OP_COLLAPSE);
  fprintf(o, "SETMETA & %u\\\\\n", (unsigned)SP_OP_SETMETA);
  fprintf(o, "\\end{longtable}\n");
}

static void tex_struct_sizes(FILE* o){
  fprintf(o, "\\section*{Struct Sizes}\n");
  fprintf(o, "\\begin{longtable}{ll}\n");
  fprintf(o, "Type & sizeof(bytes)\\\\\\hline\n");
  fprintf(o, "sp\\_event\\_hdr\\_t & %u\\\\\n", (unsigned)sizeof(sp_event_hdr_t));
  fprintf(o, "sp\\_evt\\_pop\\_t & %u\\\\\n", (unsigned)sizeof(sp_evt_pop_t));
  fprintf(o, "sp\\_evt\\_merge\\_t & %u\\\\\n", (unsigned)sizeof(sp_evt_merge_t));
  fprintf(o, "sp\\_evt\\_link\\_t & %u\\\\\n", (unsigned)sizeof(sp_evt_link_t));
  fprintf(o, "\\end{longtable}\n");
}

int main(int argc, char** argv){
  const char* out = (argc >= 2) ? argv[1] : "spherepop_spec.tex";
  FILE* o = fopen(out, "w");
  if (!o){ perror("open"); return 1; }
  tex_hdr(o);
  tex_op_table(o);
  tex_struct_sizes(o);
  fprintf(o, "\\section*{Invariants (Stub)}\n");
  fprintf(o, "\\begin{itemize}\n");
  fprintf(o, "\\item Deterministic replay: same log prefix yields identical state.\n");
  fprintf(o, "\\item Authoritative changes are event-induced; views are derived.\n");
  fprintf(o, "\\item ABI stability: event header/payload layouts are fixed.\n");
  fprintf(o, "\\end{itemize}\n");
  tex_footer(o);
  fclose(o);
  return 0;
}