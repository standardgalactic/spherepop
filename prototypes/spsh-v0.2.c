// spsh.c — Spherepop TUI Shell (v0.2)
// Build: cc -O2 -std=c11 -Wall -Wextra -I. spsh.c -lncurses -o spsh
//
// Requires in cwd:
//   sppop spmerge splink sparbiter-local spls spcat spdiff spgrep
//
// Files created:
//   .spsh_history
//   .spsh_justifications.log
//   branches/<name>.bin
//
// Remote arbiter (optional):
//   spsh --remote 127.0.0.1:7777
// Protocol (very simple):
//   client sends: "SPARB1\n" then uint32 branchLen + branch bytes
//                then uint32 baseLen + base bytes
//                then uint32 propLen + prop bytes
//   server replies: uint32 committedLen + committed bytes
// If remote fails, falls back to local arbiter (unless --remote-required).

#define _POSIX_C_SOURCE 200809L

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <time.h>

// --------- config ---------
#define HISTORY_PATH ".spsh_history"
#define JUSTLOG_PATH ".spsh_justifications.log"
#define BRANCH_DIR   "branches"
#define DEFAULT_BRANCH "main"

// temp proposal / next log
#define PROP_TMP "proposal.tmp.bin"
#define NEXT_TMP "committed.next.bin"

// pane geometry
typedef struct panes {
  WINDOW* w_hdr;
  WINDOW* w_log;
  WINDOW* w_state;
  WINDOW* w_out;
  WINDOW* w_in;
  int rows, cols;
} panes_t;

// history
typedef struct hist {
  char** items;
  int n;
  int cap;
  int cursor; // for browsing (n means "new line")
} hist_t;

// remote options
typedef struct remote_cfg {
  int enabled;
  int required;
  char host[256];
  int port;
} remote_cfg_t;

// ---------- small utils ----------
static void die_endwin(const char* msg) {
  endwin();
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

static int mkdir_p(const char* path) {
  struct stat st;
  if (stat(path, &st) == 0) {
    if (S_ISDIR(st.st_mode)) return 0;
    return -1;
  }
  return mkdir(path, 0755);
}

static int file_exists(const char* path) {
  return access(path, F_OK) == 0;
}

static int file_copy(const char* src, const char* dst) {
  FILE* in = fopen(src, "rb");
  if (!in) return -1;
  FILE* out = fopen(dst, "wb");
  if (!out) { fclose(in); return -1; }
  unsigned char buf[1<<16];
  for (;;) {
    size_t n = fread(buf, 1, sizeof(buf), in);
    if (n == 0) break;
    if (fwrite(buf, 1, n, out) != n) { fclose(in); fclose(out); return -1; }
  }
  fclose(in);
  fclose(out);
  return 0;
}

static int file_read_all(const char* path, unsigned char** out_buf, size_t* out_len) {
  *out_buf = NULL; *out_len = 0;
  FILE* f = fopen(path, "rb");
  if (!f) return -1;
  if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
  long sz = ftell(f);
  if (sz < 0) { fclose(f); return -1; }
  if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return -1; }
  unsigned char* buf = (unsigned char*)malloc((size_t)sz ? (size_t)sz : 1);
  if (!buf) { fclose(f); return -1; }
  size_t rd = fread(buf, 1, (size_t)sz, f);
  fclose(f);
  if (rd != (size_t)sz) { free(buf); return -1; }
  *out_buf = buf;
  *out_len = (size_t)sz;
  return 0;
}

static int file_write_all(const char* path, const unsigned char* buf, size_t len) {
  FILE* f = fopen(path, "wb");
  if (!f) return -1;
  if (len && fwrite(buf, 1, len, f) != len) { fclose(f); return -1; }
  fclose(f);
  return 0;
}

// ---------- history ----------
static void hist_init(hist_t* h) {
  memset(h, 0, sizeof(*h));
  h->cursor = 0;
}
static void hist_free(hist_t* h) {
  for (int i=0;i<h->n;i++) free(h->items[i]);
  free(h->items);
  memset(h, 0, sizeof(*h));
}
static void hist_push(hist_t* h, const char* s) {
  if (!s || !*s) return;
  // dedupe: if same as last, skip
  if (h->n > 0 && strcmp(h->items[h->n-1], s) == 0) {
    h->cursor = h->n;
    return;
  }
  if (h->n == h->cap) {
    int nc = h->cap ? h->cap*2 : 64;
    char** ni = (char**)realloc(h->items, (size_t)nc * sizeof(char*));
    if (!ni) return;
    h->items = ni;
    h->cap = nc;
  }
  h->items[h->n++] = strdup(s);
  h->cursor = h->n;
}
static void hist_load(hist_t* h) {
  FILE* f = fopen(HISTORY_PATH, "r");
  if (!f) { h->cursor = h->n; return; }
  char line[512];
  while (fgets(line, sizeof(line), f)) {
    size_t L = strlen(line);
    while (L && (line[L-1] == '\n' || line[L-1] == '\r')) line[--L] = 0;
    if (L) hist_push(h, line);
  }
  fclose(f);
  h->cursor = h->n;
}
static void hist_save(const hist_t* h) {
  FILE* f = fopen(HISTORY_PATH, "w");
  if (!f) return;
  for (int i=0;i<h->n;i++) fprintf(f, "%s\n", h->items[i]);
  fclose(f);
}
static const char* hist_prev(hist_t* h) {
  if (h->n == 0) return NULL;
  if (h->cursor > 0) h->cursor--;
  return h->items[h->cursor];
}
static const char* hist_next(hist_t* h) {
  if (h->n == 0) return NULL;
  if (h->cursor < h->n) h->cursor++;
  if (h->cursor == h->n) return "";
  return h->items[h->cursor];
}

// ---------- panes ----------
static void panes_destroy(panes_t* p) {
  if (p->w_hdr) delwin(p->w_hdr);
  if (p->w_log) delwin(p->w_log);
  if (p->w_state) delwin(p->w_state);
  if (p->w_out) delwin(p->w_out);
  if (p->w_in) delwin(p->w_in);
  memset(p, 0, sizeof(*p));
}

static void panes_create(panes_t* p) {
  getmaxyx(stdscr, p->rows, p->cols);

  int header_h = 1;
  int input_h  = 3;
  int body_h = p->rows - header_h - input_h;

  int left_w = p->cols / 2;
  int right_w = p->cols - left_w;

  // left column split: log (top half), state (bottom half)
  int log_h = body_h / 2;
  int state_h = body_h - log_h;

  p->w_hdr = newwin(header_h, p->cols, 0, 0);
  p->w_log = newwin(log_h, left_w, header_h, 0);
  p->w_state = newwin(state_h, left_w, header_h + log_h, 0);
  p->w_out = newwin(body_h, right_w, header_h, left_w);
  p->w_in = newwin(input_h, p->cols, header_h + body_h, 0);

  scrollok(p->w_log, TRUE);
  scrollok(p->w_state, TRUE);
  scrollok(p->w_out, TRUE);

  keypad(p->w_in, TRUE);
  keypad(stdscr, TRUE);
}

static void panes_draw_frame(panes_t* p, const char* branch, const remote_cfg_t* rcfg) {
  werase(p->w_hdr);
  werase(p->w_log);
  werase(p->w_state);
  werase(p->w_out);
  werase(p->w_in);

  mvwprintw(p->w_hdr, 0, 0, "Spherepop OS — spsh  | branch=%s  | arbiter=%s",
            branch,
            rcfg->enabled ? (rcfg->required ? "remote(required)" : "remote(fallback)") : "local");

  box(p->w_log, 0, 0);
  box(p->w_state, 0, 0);
  box(p->w_out, 0, 0);
  box(p->w_in, 0, 0);

  mvwprintw(p->w_log, 0, 2, " Log ");
  mvwprintw(p->w_state, 0, 2, " State ");
  mvwprintw(p->w_out, 0, 2, " Output ");
  mvwprintw(p->w_in, 0, 2, " Command ");

  wrefresh(p->w_hdr);
  wrefresh(p->w_log);
  wrefresh(p->w_state);
  wrefresh(p->w_out);
  wrefresh(p->w_in);
}

static void pane_append(WINDOW* w, const char* s) {
  if (!s) return;
  waddstr(w, s);
  wrefresh(w);
}

static void pane_appendln(WINDOW* w, const char* s) {
  if (!s) return;
  waddstr(w, s);
  waddstr(w, "\n");
  wrefresh(w);
}

static void pane_clear_body(WINDOW* w) {
  int y,x; getmaxyx(w,y,x);
  for (int r=1;r<y-1;r++){
    mvwhline(w, r, 1, ' ', x-2);
  }
  wmove(w, 1, 1);
  wrefresh(w);
}

// ---------- command execution ----------
static int run_capture_lines(const char* shell_cmd, WINDOW* w_out) {
  FILE* p = popen(shell_cmd, "r");
  if (!p) return -1;
  char line[512];
  while (fgets(line, sizeof(line), p)) {
    // ncurses safe-ish: strip CR
    size_t L = strlen(line);
    while (L && (line[L-1] == '\n' || line[L-1] == '\r')) line[--L] = 0;
    pane_appendln(w_out, line);
  }
  int rc = pclose(p);
  return rc;
}

// ---------- branch management ----------
static void branch_path(char* out, size_t cap, const char* name) {
  snprintf(out, cap, "%s/%s.bin", BRANCH_DIR, name);
}

static void ensure_branch_exists(const char* name) {
  mkdir_p(BRANCH_DIR);
  char path[PATH_MAX];
  branch_path(path, sizeof(path), name);
  if (!file_exists(path)) {
    // create empty log
    FILE* f = fopen(path, "wb");
    if (f) fclose(f);
  }
}

// ---------- justification ----------
static void append_justification(const char* branch, const char* cmd, const char* note) {
  FILE* f = fopen(JUSTLOG_PATH, "a");
  if (!f) return;
  time_t t = time(NULL);
  fprintf(f, "%ld\tbranch=%s\tcmd=%s\tnote=%s\n",
          (long)t,
          branch ? branch : "?",
          cmd ? cmd : "?",
          note ? note : "");
  fclose(f);
}

// ---------- remote arbiter ----------
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

static int tcp_connect(const char* host, int port) {
  struct addrinfo hints; memset(&hints,0,sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;

  char portbuf[32];
  snprintf(portbuf, sizeof(portbuf), "%d", port);

  struct addrinfo* res = NULL;
  if (getaddrinfo(host, portbuf, &hints, &res) != 0) return -1;

  int fd = -1;
  for (struct addrinfo* p=res; p; p=p->ai_next) {
    fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (fd < 0) continue;
    if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
    close(fd); fd = -1;
  }
  freeaddrinfo(res);
  return fd;
}

static int write_u32(int fd, uint32_t v) {
  uint32_t n = htonl(v);
  return (send(fd, &n, sizeof(n), 0) == sizeof(n)) ? 0 : -1;
}
static int read_u32(int fd, uint32_t* out) {
  uint32_t n=0;
  ssize_t r = recv(fd, &n, sizeof(n), MSG_WAITALL);
  if (r != (ssize_t)sizeof(n)) return -1;
  *out = ntohl(n);
  return 0;
}

static int remote_commit(const remote_cfg_t* rcfg,
                         const char* branch_name,
                         const char* base_path,
                         const char* prop_path,
                         const char* out_path) {
  unsigned char *base=NULL, *prop=NULL;
  size_t base_len=0, prop_len=0;

  if (file_read_all(base_path, &base, &base_len) != 0) return -1;
  if (file_read_all(prop_path, &prop, &prop_len) != 0) { free(base); return -1; }

  int fd = tcp_connect(rcfg->host, rcfg->port);
  if (fd < 0) { free(base); free(prop); return -1; }

  const char* magic = "SPARB1\n";
  if (send(fd, magic, strlen(magic), 0) != (ssize_t)strlen(magic)) { close(fd); free(base); free(prop); return -1; }

  uint32_t blen = (uint32_t)strlen(branch_name);
  if (write_u32(fd, blen) != 0) { close(fd); free(base); free(prop); return -1; }
  if (send(fd, branch_name, blen, 0) != (ssize_t)blen) { close(fd); free(base); free(prop); return -1; }

  if (write_u32(fd, (uint32_t)base_len) != 0) { close(fd); free(base); free(prop); return -1; }
  if (base_len && send(fd, base, base_len, 0) != (ssize_t)base_len) { close(fd); free(base); free(prop); return -1; }

  if (write_u32(fd, (uint32_t)prop_len) != 0) { close(fd); free(base); free(prop); return -1; }
  if (prop_len && send(fd, prop, prop_len, 0) != (ssize_t)prop_len) { close(fd); free(base); free(prop); return -1; }

  uint32_t out_len = 0;
  if (read_u32(fd, &out_len) != 0) { close(fd); free(base); free(prop); return -1; }

  unsigned char* out_buf = (unsigned char*)malloc(out_len ? out_len : 1);
  if (!out_buf) { close(fd); free(base); free(prop); return -1; }

  if (out_len) {
    ssize_t r = recv(fd, out_buf, out_len, MSG_WAITALL);
    if (r != (ssize_t)out_len) { free(out_buf); close(fd); free(base); free(prop); return -1; }
  }

  close(fd);
  free(base); free(prop);

  int rc = file_write_all(out_path, out_buf, out_len);
  free(out_buf);
  return rc;
}

// ---------- spsh command mapping ----------
static void refresh_log_and_state(WINDOW* w_log, WINDOW* w_state, const char* branch_path_file) {
  pane_clear_body(w_log);
  pane_clear_body(w_state);

  // show last ~N events: for simplicity just run spcat and let the pane scroll
  {
    char cmd[PATH_MAX + 64];
    snprintf(cmd, sizeof(cmd), "./spcat %s 2>/dev/null", branch_path_file);
    run_capture_lines(cmd, w_log);
  }
  {
    char cmd[PATH_MAX + 64];
    snprintf(cmd, sizeof(cmd), "./spls %s 2>/dev/null", branch_path_file);
    run_capture_lines(cmd, w_state);
  }
}

static void cmd_help(WINDOW* w_out) {
  pane_appendln(w_out,
    "Commands:\n"
    "  pop N\n"
    "  merge A B [C...]\n"
    "  link SRC DST TYPE\n"
    "  ls              (alias: state)\n"
    "  cat             (alias: log)\n"
    "  diff\n"
    "  grep [--src H] [--dst H] [--type T]\n"
    "  branch                  (list branches)\n"
    "  branch NAME             (switch/create)\n"
    "  note TEXT               (append a justification note)\n"
    "  justify on|off          (prompt for notes on mutating cmds)\n"
    "  remote on|off           (enable/disable remote if configured)\n"
    "  quit"
  );
}

// prompt in input pane for a note (non-semantic)
static void prompt_note(WINDOW* w_in, char* out, size_t cap, const char* label) {
  werase(w_in);
  box(w_in, 0, 0);
  mvwprintw(w_in, 1, 2, "%s: ", label);
  wrefresh(w_in);
  echo();
  wgetnstr(w_in, out, (int)cap - 1);
  noecho();
}

static void list_branches(WINDOW* w_out) {
  mkdir_p(BRANCH_DIR);
  pane_appendln(w_out, "Branches:");
  // minimal portable listing: use shell
  run_capture_lines("ls -1 branches 2>/dev/null | sed 's/\\.bin$//'", w_out);
}

static int run_local_commit(WINDOW* w_out,
                            const char* base_path,
                            const char* prop_path,
                            const char* out_path) {
  char cmd[PATH_MAX*2 + 128];
  snprintf(cmd, sizeof(cmd),
           "./sparbiter-local --base %s --out %s %s 2>&1",
           base_path, out_path, prop_path);
  return run_capture_lines(cmd, w_out);
}

static int run_make_proposal(WINDOW* w_out,
                             const char* verb,
                             const char* base_path,
                             const char* args) {
  // verb is "sppop" or "spmerge" or "splink"
  char cmd[PATH_MAX*2 + 256];
  if (strcmp(verb, "sppop") == 0) {
    snprintf(cmd, sizeof(cmd), "./sppop --out %s %s 2>&1", PROP_TMP, args);
  } else if (strcmp(verb, "spmerge") == 0) {
    snprintf(cmd, sizeof(cmd), "./spmerge --in %s --out %s %s 2>&1", base_path, PROP_TMP, args);
  } else if (strcmp(verb, "splink") == 0) {
    snprintf(cmd, sizeof(cmd), "./splink --in %s --out %s %s 2>&1", base_path, PROP_TMP, args);
  } else {
    pane_appendln(w_out, "internal: unknown proposal verb");
    return -1;
  }
  unlink(PROP_TMP);
  return run_capture_lines(cmd, w_out);
}

static void cmd_run(panes_t* p,
                    remote_cfg_t* rcfg,
                    const char* branch,
                    const char* branch_file,
                    const char* input,
                    hist_t* hist,
                    int* justify_on) {

  // record history
  hist_push(hist, input);

  // output echo
  pane_appendln(p->w_out, input);

  // commands
  if (strcmp(input, "quit") == 0) return;

  if (strcmp(input, "help") == 0) { cmd_help(p->w_out); return; }

  if (strcmp(input, "ls") == 0 || strcmp(input, "state") == 0) {
    char cmd[PATH_MAX + 64];
    snprintf(cmd, sizeof(cmd), "./spls %s 2>&1", branch_file);
    run_capture_lines(cmd, p->w_out);
    refresh_log_and_state(p->w_log, p->w_state, branch_file);
    return;
  }

  if (strcmp(input, "cat") == 0 || strcmp(input, "log") == 0) {
    char cmd[PATH_MAX + 64];
    snprintf(cmd, sizeof(cmd), "./spcat %s 2>&1", branch_file);
    run_capture_lines(cmd, p->w_out);
    refresh_log_and_state(p->w_log, p->w_state, branch_file);
    return;
  }

  if (strcmp(input, "diff") == 0) {
    char cmd[PATH_MAX + 64];
    snprintf(cmd, sizeof(cmd), "./spdiff %s 2>&1", branch_file);
    run_capture_lines(cmd, p->w_out);
    refresh_log_and_state(p->w_log, p->w_state, branch_file);
    return;
  }

  if (strncmp(input, "grep", 4) == 0) {
    char cmd[PATH_MAX + 256];
    const char* args = input + 4;
    while (*args == ' ') args++;
    snprintf(cmd, sizeof(cmd), "./spgrep %s %s 2>&1", branch_file, args);
    run_capture_lines(cmd, p->w_out);
    return;
  }

  if (strncmp(input, "note ", 5) == 0) {
    append_justification(branch, input, input + 5);
    pane_appendln(p->w_out, "[note saved]");
    return;
  }

  if (strncmp(input, "justify ", 8) == 0) {
    const char* a = input + 8;
    while (*a == ' ') a++;
    if (strcmp(a, "on") == 0) { *justify_on = 1; pane_appendln(p->w_out, "[justify=on]"); }
    else if (strcmp(a, "off") == 0) { *justify_on = 0; pane_appendln(p->w_out, "[justify=off]"); }
    else pane_appendln(p->w_out, "Usage: justify on|off");
    return;
  }

  if (strncmp(input, "remote ", 7) == 0) {
    const char* a = input + 7;
    while (*a == ' ') a++;
    if (strcmp(a, "on") == 0) { rcfg->enabled = 1; pane_appendln(p->w_out, "[remote enabled]"); }
    else if (strcmp(a, "off") == 0) { rcfg->enabled = 0; pane_appendln(p->w_out, "[remote disabled]"); }
    else pane_appendln(p->w_out, "Usage: remote on|off");
    return;
  }

  if (strcmp(input, "branch") == 0) {
    list_branches(p->w_out);
    return;
  }

  if (strncmp(input, "branch ", 7) == 0) {
    // switching branches is handled in main loop (we return a marker)
    pane_appendln(p->w_out, "[use: branch NAME to switch]");
    return;
  }

  // mutating commands: pop / merge / link
  const char* verb = NULL;
  const char* args = NULL;

  if (strncmp(input, "pop ", 4) == 0) { verb = "sppop"; args = input + 4; }
  else if (strncmp(input, "merge ", 6) == 0) { verb = "spmerge"; args = input + 6; }
  else if (strncmp(input, "link ", 5) == 0) { verb = "splink"; args = input + 5; }

  if (!verb) {
    pane_appendln(p->w_out, "Unknown command. Type 'help'.");
    return;
  }

  // optional justification prompt
  if (*justify_on) {
    char note[256] = {0};
    prompt_note(p->w_in, note, sizeof(note), "Justification (optional)");
    if (note[0]) {
      append_justification(branch, input, note);
      pane_appendln(p->w_out, "[justification saved]");
    }
  }

  // create proposal file
  if (run_make_proposal(p->w_out, verb, branch_file, args) != 0) {
    pane_appendln(p->w_out, "[proposal generation returned nonzero]");
    return;
  }

  if (!file_exists(PROP_TMP)) {
    pane_appendln(p->w_out, "[no proposal file produced]");
    return;
  }

  // commit via remote or local
  unlink(NEXT_TMP);

  int committed_ok = 0;

  if (rcfg->enabled) {
    if (remote_commit(rcfg, branch, branch_file, PROP_TMP, NEXT_TMP) == 0) {
      pane_appendln(p->w_out, "[remote committed]");
      committed_ok = 1;
    } else {
      pane_appendln(p->w_out, "[remote commit failed]");
      if (rcfg->required) {
        pane_appendln(p->w_out, "[remote-required: abort]");
        return;
      }
      pane_appendln(p->w_out, "[falling back to local arbiter]");
    }
  }

  if (!committed_ok) {
    run_local_commit(p->w_out, branch_file, PROP_TMP, NEXT_TMP);
    if (file_exists(NEXT_TMP)) {
      pane_appendln(p->w_out, "[local committed]");
      committed_ok = 1;
    }
  }

  if (!committed_ok) {
    pane_appendln(p->w_out, "[commit failed]");
    return;
  }

  // move next into branch file
  if (rename(NEXT_TMP, branch_file) != 0) {
    pane_appendln(p->w_out, "[failed to finalize branch log]");
    return;
  }

  refresh_log_and_state(p->w_log, p->w_state, branch_file);
}

// ---------- input line editor with history ----------
static void input_getline(panes_t* p, hist_t* hist, char* out, size_t cap) {
  memset(out, 0, cap);
  int pos = 0;

  werase(p->w_in);
  box(p->w_in, 0, 0);
  mvwprintw(p->w_in, 1, 2, "> ");
  wmove(p->w_in, 1, 4);
  wrefresh(p->w_in);

  hist->cursor = hist->n;

  for (;;) {
    int ch = wgetch(p->w_in);
    if (ch == '\n' || ch == '\r') {
      out[pos] = 0;
      return;
    }
    if (ch == KEY_UP) {
      const char* prev = hist_prev(hist);
      if (prev) {
        strncpy(out, prev, cap-1);
        pos = (int)strlen(out);
      }
    } else if (ch == KEY_DOWN) {
      const char* nxt = hist_next(hist);
      if (nxt) {
        strncpy(out, nxt, cap-1);
        pos = (int)strlen(out);
      }
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
      if (pos > 0) out[--pos] = 0;
    } else if (ch >= 32 && ch <= 126) {
      if ((size_t)pos < cap-1) {
        out[pos++] = (char)ch;
        out[pos] = 0;
      }
    }

    // redraw line
    int y,x; getmaxyx(p->w_in, y, x);
    mvwhline(p->w_in, 1, 4, ' ', x-6);
    mvwprintw(p->w_in, 1, 4, "%s", out);
    wmove(p->w_in, 1, 4 + pos);
    wrefresh(p->w_in);
  }
}

// ---------- main ----------
int main(int argc, char** argv) {
  remote_cfg_t rcfg;
  memset(&rcfg, 0, sizeof(rcfg));

  // parse args
  const char* branch = DEFAULT_BRANCH;
  for (int i=1;i<argc;i++){
    if (strcmp(argv[i], "--branch") == 0 && i+1 < argc) {
      branch = argv[++i];
    } else if (strcmp(argv[i], "--remote") == 0 && i+1 < argc) {
      rcfg.enabled = 1;
      rcfg.required = 0;
      const char* hp = argv[++i];
      const char* colon = strchr(hp, ':');
      if (!colon) {
        fprintf(stderr, "bad --remote host:port\n");
        return 2;
      }
      size_t hl = (size_t)(colon - hp);
      if (hl >= sizeof(rcfg.host)) hl = sizeof(rcfg.host)-1;
      memcpy(rcfg.host, hp, hl);
      rcfg.host[hl] = 0;
      rcfg.port = atoi(colon+1);
      if (rcfg.port <= 0) { fprintf(stderr, "bad remote port\n"); return 2; }
    } else if (strcmp(argv[i], "--remote-required") == 0) {
      rcfg.required = 1;
      rcfg.enabled = 1;
    } else if (strcmp(argv[i], "--help") == 0) {
      fprintf(stderr,
        "Usage: spsh [--branch NAME] [--remote host:port] [--remote-required]\n");
      return 0;
    } else {
      fprintf(stderr, "Unknown arg: %s\n", argv[i]);
      return 2;
    }
  }

  // init filesystem layout
  mkdir_p(BRANCH_DIR);
  ensure_branch_exists(branch);

  char branch_file[PATH_MAX];
  branch_path(branch_file, sizeof(branch_file), branch);

  // ncurses init
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  panes_t p; memset(&p, 0, sizeof(p));
  panes_create(&p);

  hist_t hist; hist_init(&hist);
  hist_load(&hist);

  int justify_on = 1;

  panes_draw_frame(&p, branch, &rcfg);
  refresh_log_and_state(p.w_log, p.w_state, branch_file);
  pane_appendln(p.w_out, "Type 'help' for commands.");

  char line[256];

  for (;;) {
    input_getline(&p, &hist, line, sizeof(line));
    if (strcmp(line, "quit") == 0) break;

    // branch switching handled here so header updates immediately
    if (strncmp(line, "branch ", 7) == 0) {
      const char* bn = line + 7;
      while (*bn == ' ') bn++;
      if (!*bn) {
        pane_appendln(p.w_out, "Usage: branch NAME");
        continue;
      }
      branch = bn;
      ensure_branch_exists(branch);
      branch_path(branch_file, sizeof(branch_file), branch);
      panes_destroy(&p);
      panes_create(&p);
      panes_draw_frame(&p, branch, &rcfg);
      refresh_log_and_state(p.w_log, p.w_state, branch_file);
      pane_appendln(p.w_out, "[switched branch]");
      continue;
    }

    cmd_run(&p, &rcfg, branch, branch_file, line, &hist, &justify_on);
  }

  hist_save(&hist);
  hist_free(&hist);

  panes_destroy(&p);
  endwin();
  return 0;
}