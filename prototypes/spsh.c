// spsh.c — Spherepop TUI Shell (v0.3)
// Build:
//   cc -O2 -std=c11 -Wall -Wextra -I. spsh.c -lncurses -o spsh
//
// Requires:
//   sppop spmerge splink sparbiter-local-w spcat spls spdiff spgrep spconflicts
//
// New features:
//   - authority weighting (auth NAME WEIGHT)
//   - conflict reporting (conflicts)
//   - preview pane (accepted/rejected summary)
//
// State files:
//   branches/<branch>.bin
//   proposal.tmp.bin
//   committed.next.bin
//   arbiter_report.bin

#define _POSIX_C_SOURCE 200809L

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>

#define BRANCH_DIR "branches"
#define PROP_TMP  "proposal.tmp.bin"
#define NEXT_TMP  "committed.next.bin"
#define REPORT_TMP "arbiter_report.bin"

typedef struct {
  WINDOW *w_hdr, *w_log, *w_state, *w_out, *w_in;
  int rows, cols;
} panes_t;

/* ---------------- authority context ---------------- */

static char current_auth[64] = "local";
static uint32_t current_weight = 1;

/* ---------------- filesystem helpers ---------------- */

static void ensure_dir(const char* d){
  struct stat st;
  if (stat(d, &st) == 0 && S_ISDIR(st.st_mode)) return;
  mkdir(d, 0755);
}

static void branch_path(char* out, size_t cap, const char* name){
  snprintf(out, cap, "%s/%s.bin", BRANCH_DIR, name);
}

static void ensure_branch(const char* name){
  ensure_dir(BRANCH_DIR);
  char p[256];
  branch_path(p, sizeof(p), name);
  if (access(p, F_OK) != 0){
    FILE* f = fopen(p, "wb");
    if (f) fclose(f);
  }
}

/* ---------------- UI helpers ---------------- */

static void panes_create(panes_t* p){
  getmaxyx(stdscr, p->rows, p->cols);
  int header = 1, input = 3;
  int body = p->rows - header - input;
  int left = p->cols / 2;
  int right = p->cols - left;
  int log_h = body / 2;

  p->w_hdr   = newwin(header, p->cols, 0, 0);
  p->w_log   = newwin(log_h, left, header, 0);
  p->w_state = newwin(body - log_h, left, header + log_h, 0);
  p->w_out   = newwin(body, right, header, left);
  p->w_in    = newwin(input, p->cols, header + body, 0);

  scrollok(p->w_log, TRUE);
  scrollok(p->w_state, TRUE);
  scrollok(p->w_out, TRUE);
  keypad(p->w_in, TRUE);
}

static void panes_draw(panes_t* p, const char* branch){
  werase(stdscr);
  mvprintw(0, 0,
    "Spherepop spsh | branch=%s | auth=%s(%u)",
    branch, current_auth, current_weight);

  box(p->w_log, 0, 0);
  box(p->w_state, 0, 0);
  box(p->w_out, 0, 0);
  box(p->w_in, 0, 0);

  mvwprintw(p->w_log, 0, 2, " Log ");
  mvwprintw(p->w_state, 0, 2, " State ");
  mvwprintw(p->w_out, 0, 2, " Output ");
  mvwprintw(p->w_in, 0, 2, " Command ");

  wrefresh(stdscr);
  wrefresh(p->w_log);
  wrefresh(p->w_state);
  wrefresh(p->w_out);
  wrefresh(p->w_in);
}

static void pane_clear(WINDOW* w){
  int y,x; getmaxyx(w,y,x);
  for (int r=1;r<y-1;r++)
    mvwhline(w, r, 1, ' ', x-2);
  wmove(w,1,1);
  wrefresh(w);
}

static void pane_print(WINDOW* w, const char* s){
  waddstr(w, s);
  waddstr(w, "\n");
  wrefresh(w);
}

static int run_capture(const char* cmd, WINDOW* w){
  FILE* p = popen(cmd, "r");
  if (!p) return -1;
  char line[512];
  while (fgets(line, sizeof(line), p)){
    pane_print(w, line);
  }
  return pclose(p);
}

/* ---------------- refresh panes ---------------- */

static void refresh_log_state(panes_t* p, const char* branch_file){
  pane_clear(p->w_log);
  pane_clear(p->w_state);

  char cmd[512];
  snprintf(cmd, sizeof(cmd), "./spcat %s 2>/dev/null", branch_file);
  run_capture(cmd, p->w_log);

  snprintf(cmd, sizeof(cmd), "./spls %s 2>/dev/null", branch_file);
  run_capture(cmd, p->w_state);
}

/* ---------------- command execution ---------------- */

static void run_command(panes_t* p,
                        const char* branch,
                        const char* branch_file,
                        const char* line){

  pane_print(p->w_out, line);

  /* ---------- non-mutating ---------- */

  if (strcmp(line, "help") == 0){
    pane_print(p->w_out,
      "Commands:\n"
      "  auth NAME WEIGHT\n"
      "  pop N\n"
      "  merge A B [C...]\n"
      "  link SRC DST TYPE\n"
      "  ls | state\n"
      "  cat | log\n"
      "  diff\n"
      "  conflicts\n"
      "  branch NAME\n"
      "  quit"
    );
    return;
  }

  if (strcmp(line, "ls") == 0 || strcmp(line, "state") == 0){
    char cmd[256];
    snprintf(cmd,sizeof(cmd),"./spls %s", branch_file);
    run_capture(cmd, p->w_out);
    return;
  }

  if (strcmp(line, "cat") == 0 || strcmp(line, "log") == 0){
    char cmd[256];
    snprintf(cmd,sizeof(cmd),"./spcat %s", branch_file);
    run_capture(cmd, p->w_out);
    return;
  }

  if (strcmp(line, "diff") == 0){
    char cmd[256];
    snprintf(cmd,sizeof(cmd),"./spdiff %s", branch_file);
    run_capture(cmd, p->w_out);
    return;
  }

  if (strcmp(line, "conflicts") == 0){
    run_capture("./spconflicts " REPORT_TMP, p->w_out);
    return;
  }

  /* ---------- authority ---------- */

  if (strncmp(line, "auth ", 5) == 0){
    const char* a = line + 5;
    char name[64]; unsigned w;
    if (sscanf(a, "%63s %u", name, &w) == 2){
      strncpy(current_auth, name, sizeof(current_auth)-1);
      current_weight = w;
      pane_print(p->w_out, "[authority updated]");
    } else {
      pane_print(p->w_out, "Usage: auth NAME WEIGHT");
    }
    return;
  }

  /* ---------- branch ---------- */

  if (strncmp(line, "branch ", 7) == 0){
    pane_print(p->w_out, "Restart shell to switch branch.");
    return;
  }

  /* ---------- mutating ---------- */

  const char* verb = NULL;
  const char* args = NULL;

  if (strncmp(line,"pop ",4)==0){ verb="sppop"; args=line+4; }
  else if (strncmp(line,"merge ",6)==0){ verb="spmerge"; args=line+6; }
  else if (strncmp(line,"link ",5)==0){ verb="splink"; args=line+5; }

  if (!verb){
    pane_print(p->w_out, "Unknown command. Type 'help'.");
    return;
  }

  unlink(PROP_TMP);
  unlink(NEXT_TMP);
  unlink(REPORT_TMP);

  char cmd[1024];

  if (strcmp(verb,"sppop")==0){
    snprintf(cmd,sizeof(cmd),
      "./sppop --out %s %s", PROP_TMP, args);
  } else {
    snprintf(cmd,sizeof(cmd),
      "./%s --in %s --out %s %s",
      verb, branch_file, PROP_TMP, args);
  }

  if (run_capture(cmd, p->w_out) != 0 || access(PROP_TMP,F_OK)!=0){
    pane_print(p->w_out, "[proposal failed]");
    return;
  }

  snprintf(cmd,sizeof(cmd),
    "./sparbiter-local-w "
    "--base %s "
    "--out %s "
    "--report %s "
    "--auth %s --weight %u --prop %s",
    branch_file, NEXT_TMP, REPORT_TMP,
    current_auth, current_weight, PROP_TMP
  );

  run_capture(cmd, p->w_out);

  if (access(NEXT_TMP, F_OK)==0){
    rename(NEXT_TMP, branch_file);
    pane_print(p->w_out, "[committed]");
    refresh_log_state(p, branch_file);
  } else {
    pane_print(p->w_out, "[commit failed]");
  }
}

/* ---------------- main ---------------- */

int main(int argc, char** argv){
  const char* branch = "main";
  if (argc >= 3 && strcmp(argv[1],"--branch")==0)
    branch = argv[2];

  ensure_branch(branch);

  char branch_file[256];
  branch_path(branch_file, sizeof(branch_file), branch);

  initscr();
  cbreak();
  noecho();

  panes_t p;
  panes_create(&p);
  panes_draw(&p, branch);
  refresh_log_state(&p, branch_file);

  char line[256];

  for (;;){
    werase(p.w_in);
    box(p.w_in,0,0);
    mvwprintw(p.w_in,1,2,"> ");
    wrefresh(p.w_in);

    wgetnstr(p.w_in, line, sizeof(line)-1);

    if (strcmp(line,"quit")==0) break;
    run_command(&p, branch, branch_file, line);
  }

  endwin();
  return 0;
}