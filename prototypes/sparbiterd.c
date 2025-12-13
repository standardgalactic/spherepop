// sparbiterd.c — Spherepop Remote Arbiter Daemon
// Build: cc -O2 -std=c11 -Wall -Wextra -I. sparbiterd.c -o sparbiterd
//
// Requires:
//   sparbiter-local in PATH
//
// Usage:
//   ./sparbiterd 7777
//
// This is a minimal, single-connection-at-a-time server.

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAGIC "SPARB1\n"
#define TMP_BASE "arb_base.bin"
#define TMP_PROP "arb_prop.bin"
#define TMP_OUT  "arb_out.bin"

static int read_exact(int fd, void* buf, size_t n) {
  size_t off = 0;
  while (off < n) {
    ssize_t r = recv(fd, (char*)buf + off, n - off, 0);
    if (r <= 0) return -1;
    off += (size_t)r;
  }
  return 0;
}

static int write_exact(int fd, const void* buf, size_t n) {
  size_t off = 0;
  while (off < n) {
    ssize_t r = send(fd, (const char*)buf + off, n - off, 0);
    if (r <= 0) return -1;
    off += (size_t)r;
  }
  return 0;
}

static int read_u32(int fd, uint32_t* out) {
  uint32_t v;
  if (read_exact(fd, &v, sizeof(v)) < 0) return -1;
  *out = ntohl(v);
  return 0;
}

static int write_u32(int fd, uint32_t v) {
  uint32_t n = htonl(v);
  return write_exact(fd, &n, sizeof(n));
}

static int write_file(const char* path, const uint8_t* buf, size_t len) {
  FILE* f = fopen(path, "wb");
  if (!f) return -1;
  if (len && fwrite(buf, 1, len, f) != len) { fclose(f); return -1; }
  fclose(f);
  return 0;
}

static int read_file(const char* path, uint8_t** out_buf, size_t* out_len) {
  *out_buf = NULL;
  *out_len = 0;

  FILE* f = fopen(path, "rb");
  if (!f) return -1;
  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  if (sz < 0) { fclose(f); return -1; }
  fseek(f, 0, SEEK_SET);

  uint8_t* buf = (uint8_t*)malloc((size_t)sz ? (size_t)sz : 1);
  if (!buf) { fclose(f); return -1; }

  if (sz && fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
    free(buf); fclose(f); return -1;
  }

  fclose(f);
  *out_buf = buf;
  *out_len = (size_t)sz;
  return 0;
}

static int handle_client(int fd) {
  char magic[sizeof(MAGIC)];
  if (read_exact(fd, magic, sizeof(MAGIC) - 1) < 0) return -1;
  magic[sizeof(MAGIC) - 1] = 0;
  if (strcmp(magic, MAGIC) != 0) return -1;

  uint32_t blen, base_len, prop_len;

  if (read_u32(fd, &blen) < 0) return -1;
  char* branch = (char*)malloc(blen + 1);
  if (!branch) return -1;
  if (read_exact(fd, branch, blen) < 0) { free(branch); return -1; }
  branch[blen] = 0;

  if (read_u32(fd, &base_len) < 0) { free(branch); return -1; }
  uint8_t* base = (uint8_t*)malloc(base_len ? base_len : 1);
  if (!base) { free(branch); return -1; }
  if (base_len && read_exact(fd, base, base_len) < 0) {
    free(branch); free(base); return -1;
  }

  if (read_u32(fd, &prop_len) < 0) {
    free(branch); free(base); return -1;
  }
  uint8_t* prop = (uint8_t*)malloc(prop_len ? prop_len : 1);
  if (!prop) { free(branch); free(base); return -1; }
  if (prop_len && read_exact(fd, prop, prop_len) < 0) {
    free(branch); free(base); free(prop); return -1;
  }

  // write temp files
  if (write_file(TMP_BASE, base, base_len) < 0 ||
      write_file(TMP_PROP, prop, prop_len) < 0) {
    free(branch); free(base); free(prop); return -1;
  }

  free(branch);
  free(base);
  free(prop);

  // run arbiter
  char cmd[256];
  snprintf(cmd, sizeof(cmd),
           "./sparbiter-local --base %s --out %s %s >/dev/null 2>&1",
           TMP_BASE, TMP_OUT, TMP_PROP);

  int rc = system(cmd);
  if (rc != 0) return -1;

  // read result
  uint8_t* out_buf = NULL;
  size_t out_len = 0;
  if (read_file(TMP_OUT, &out_buf, &out_len) < 0) return -1;

  if (write_u32(fd, (uint32_t)out_len) < 0 ||
      (out_len && write_exact(fd, out_buf, out_len) < 0)) {
    free(out_buf);
    return -1;
  }

  free(out_buf);
  return 0;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return 2;
  }

  int port = atoi(argv[1]);
  if (port <= 0) {
    fprintf(stderr, "Invalid port\n");
    return 2;
  }

  int srv = socket(AF_INET, SOCK_STREAM, 0);
  if (srv < 0) { perror("socket"); return 1; }

  int opt = 1;
  setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons((uint16_t)port);

  if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(srv);
    return 1;
  }

  if (listen(srv, 4) < 0) {
    perror("listen");
    close(srv);
    return 1;
  }

  fprintf(stderr, "sparbiterd listening on port %d\n", port);

  for (;;) {
    int fd = accept(srv, NULL, NULL);
    if (fd < 0) {
      perror("accept");
      continue;
    }
    handle_client(fd);
    close(fd);
  }
}