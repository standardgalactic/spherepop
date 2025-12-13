#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define CMD_MAX 256
#define LOG_BASE "committed.bin"
#define LOG_NEXT "committed.next.bin"
#define PROP_TMP "proposal.tmp.bin"

static WINDOW *w_cmd, *w_out, *w_input;

static void draw_layout(void){
  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  w_cmd   = newwin(rows-3, cols/3, 1, 0);
  w_out   = newwin(rows-3, cols-cols/3, 1, cols/3);
  w_input = newwin(2, cols, rows-2, 0);

  box(w_cmd, 0, 0);
  box(w_out, 0, 0);
  box(w_input, 0, 0);

  mvprintw(0, 2, " Spherepop OS — spsh ");
  mvwprintw(w_cmd, 0, 2, " Commands ");
  mvwprintw(w_out, 0, 2, " Output ");

  wrefresh(stdscr);
  wrefresh(w_cmd);
  wrefresh(w_out);
  wrefresh(w_input);
}

static void out_print(const char* s){
  waddstr(w_out, s);
  waddstr(w_out, "\n");
  wrefresh(w_out);
}

static void run_cmd(const char* cmd){
  char buf[512];

  /* clean temp */
  unlink(PROP_TMP);

  if (strncmp(cmd, "pop ", 4) == 0){
    snprintf(buf, sizeof(buf),
      "./sppop --out %s %s && "
      "./sparbiter-local --base %s --out %s %s",
      PROP_TMP, cmd+4, LOG_BASE, LOG_NEXT, PROP_TMP);
  }
  else if (strncmp(cmd, "merge ", 6) == 0){
    snprintf(buf, sizeof(buf),
      "./spmerge --in %s --out %s %s && "
      "./sparbiter-local --base %s --out %s %s",
      LOG_BASE, PROP_TMP, cmd+6, LOG_BASE, LOG_NEXT, PROP_TMP);
  }
  else if (strncmp(cmd, "link ", 5) == 0){
    snprintf(buf, sizeof(buf),
      "./splink --in %s --out %s %s && "
      "./sparbiter-local --base %s --out %s %s",
      LOG_BASE, PROP_TMP, cmd+5, LOG_BASE, LOG_NEXT, PROP_TMP);
  }
  else if (strcmp(cmd, "ls") == 0){
    snprintf(buf, sizeof(buf), "./spls %s", LOG_BASE);
  }
  else if (strcmp(cmd, "cat") == 0){
    snprintf(buf, sizeof(buf), "./spcat %s", LOG_BASE);
  }
  else if (strcmp(cmd, "diff") == 0){
    snprintf(buf, sizeof(buf), "./spdiff %s", LOG_BASE);
  }
  else if (strcmp(cmd, "help") == 0){
    out_print(
      "Commands:\n"
      "  pop N\n"
      "  merge A B [C...]\n"
      "  link SRC DST TYPE\n"
      "  ls\n"
      "  cat\n"
      "  diff\n"
      "  quit"
    );
    return;
  }
  else {
    out_print("Unknown command. Type 'help'.");
    return;
  }

  /* Run command and capture output */
  FILE* p = popen(buf, "r");
  if (!p){
    out_print("Failed to run command.");
    return;
  }

  char line[256];
  while (fgets(line, sizeof(line), p)){
    out_print(line);
  }
  pclose(p);

  /* Commit successful? */
  if (access(LOG_NEXT, F_OK) == 0){
    rename(LOG_NEXT, LOG_BASE);
    out_print("[committed]");
  }
}

int main(void){
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  draw_layout();

  char cmd[CMD_MAX];

  for (;;){
    werase(w_input);
    box(w_input, 0, 0);
    mvwprintw(w_input, 1, 2, "> ");
    wrefresh(w_input);

    wgetnstr(w_input, cmd, CMD_MAX-1);

    if (strcmp(cmd, "quit") == 0) break;

    out_print(cmd);
    run_cmd(cmd);
  }

  endwin();
  return 0;
}