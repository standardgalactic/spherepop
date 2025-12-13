/* sp_inject_server_ws.c
 *
 * Minimal RFC6455 WebSocket injection arbiter (no TLS).
 * - Accepts multiple WS clients
 * - Incoming WS text frames are NDJSON proposal lines
 * - Applies canonical events to mock kernel via sp_mk_apply_with_diff()
 * - Broadcasts enriched diffs to all clients
 * - Sends acks to the origin client
 *
 * Dependencies: POSIX sockets + poll. No external libs.
 *
 * Compile:
 *   cc -O2 -std=c11 -Wall -Wextra -o sp_inject_ws \
 *     sp_inject_server_ws.c sp_mock_kernel.c sp_mk_diff.c sp_replay.c
 *
 * Run:
 *   ./sp_inject_ws events.bin 8765
 *
 * Connect:
 *   websocat ws://127.0.0.1:8765
 */

#define _POSIX_C_SOURCE 200809L

#include "sp_mock_kernel.h"
#include "sp_mk_diff.h"
#include "sp_layout_hint.h"
#include "spherepop_event_layout.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/* =========================
 * Small utilities
 * ========================= */

static void die(const char* msg) {
  perror(msg);
  exit(2);
}

static int set_nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) return -1;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int write_all(int fd, const void* buf, size_t len) {
  const unsigned char* p = (const unsigned char*)buf;
  size_t off = 0;
  while (off < len) {
    ssize_t n = send(fd, p + off, len - off, 0);
    if (n < 0) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
      return -1;
    }
    off += (size_t)n;
  }
  return 1;
}

/* =========================
 * SHA1 + Base64 (for WS accept)
 * ========================= */

typedef struct {
  unsigned int h[5];
  unsigned long long len;
  unsigned char buf[64];
  unsigned int buf_len;
} sha1_t;

static unsigned int rol(unsigned int v, unsigned int n) { return (v << n) | (v >> (32 - n)); }

static void sha1_init(sha1_t* s) {
  s->h[0] = 0x67452301u;
  s->h[1] = 0xEFCDAB89u;
  s->h[2] = 0x98BADCFEu;
  s->h[3] = 0x10325476u;
  s->h[4] = 0xC3D2E1F0u;
  s->len = 0;
  s->buf_len = 0;
}

static void sha1_block(sha1_t* s, const unsigned char b[64]) {
  unsigned int w[80];
  for (int i = 0; i < 16; i++) {
    w[i] = ((unsigned int)b[i * 4 + 0] << 24) |
           ((unsigned int)b[i * 4 + 1] << 16) |
           ((unsigned int)b[i * 4 + 2] << 8)  |
           ((unsigned int)b[i * 4 + 3] << 0);
  }
  for (int i = 16; i < 80; i++) w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

  unsigned int a = s->h[0], c = s->h[2], b2 = s->h[1], d = s->h[3], e = s->h[4];
  for (int i = 0; i < 80; i++) {
    unsigned int f, k;
    if (i < 20)      { f = (b2 & c) | ((~b2) & d); k = 0x5A827999u; }
    else if (i < 40) { f = b2 ^ c ^ d;            k = 0x6ED9EBA1u; }
    else if (i < 60) { f = (b2 & c) | (b2 & d) | (c & d); k = 0x8F1BBCDCu; }
    else             { f = b2 ^ c ^ d;            k = 0xCA62C1D6u; }
    unsigned int temp = rol(a, 5) + f + e + k + w[i];
    e = d; d = c; c = rol(b2, 30); b2 = a; a = temp;
  }
  s->h[0] += a; s->h[1] += b2; s->h[2] += c; s->h[3] += d; s->h[4] += e;
}

static void sha1_update(sha1_t* s, const unsigned char* data, size_t n) {
  s->len += (unsigned long long)n * 8ull;
  while (n) {
    size_t take = 64 - s->buf_len;
    if (take > n) take = n;
    memcpy(s->buf + s->buf_len, data, take);
    s->buf_len += (unsigned int)take;
    data += take;
    n -= take;
    if (s->buf_len == 64) {
      sha1_block(s, s->buf);
      s->buf_len = 0;
    }
  }
}

static void sha1_final(sha1_t* s, unsigned char out[20]) {
  /* pad */
  s->buf[s->buf_len++] = 0x80;
  if (s->buf_len > 56) {
    while (s->buf_len < 64) s->buf[s->buf_len++] = 0;
    sha1_block(s, s->buf);
    s->buf_len = 0;
  }
  while (s->buf_len < 56) s->buf[s->buf_len++] = 0;

  /* length */
  unsigned long long L = s->len;
  for (int i = 7; i >= 0; i--) {
    s->buf[s->buf_len++] = (unsigned char)((L >> (i * 8)) & 0xFF);
  }
  sha1_block(s, s->buf);

  for (int i = 0; i < 5; i++) {
    out[i * 4 + 0] = (unsigned char)((s->h[i] >> 24) & 0xFF);
    out[i * 4 + 1] = (unsigned char)((s->h[i] >> 16) & 0xFF);
    out[i * 4 + 2] = (unsigned char)((s->h[i] >> 8) & 0xFF);
    out[i * 4 + 3] = (unsigned char)((s->h[i] >> 0) & 0xFF);
  }
}

static const char b64tab[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void base64_encode(const unsigned char* in, size_t inlen, char* out, size_t outcap) {
  size_t o = 0;
  for (size_t i = 0; i < inlen; i += 3) {
    unsigned int v = (unsigned int)in[i] << 16;
    if (i + 1 < inlen) v |= (unsigned int)in[i + 1] << 8;
    if (i + 2 < inlen) v |= (unsigned int)in[i + 2];

    char c0 = b64tab[(v >> 18) & 63];
    char c1 = b64tab[(v >> 12) & 63];
    char c2 = (i + 1 < inlen) ? b64tab[(v >> 6) & 63] : '=';
    char c3 = (i + 2 < inlen) ? b64tab[v & 63] : '=';

    if (o + 4 >= outcap) break;
    out[o++] = c0; out[o++] = c1; out[o++] = c2; out[o++] = c3;
  }
  if (o < outcap) out[o] = 0;
}

/* =========================
 * WebSocket frame IO
 * ========================= */

typedef struct {
  int fd;
  int is_ws;
  char name[64]; /* origin client label */
  unsigned char rx[8192];
  size_t rx_len;
} peer_t;

static int ws_send_text(int fd, const char* s, size_t n) {
  /* server -> client frames are unmasked */
  unsigned char hdr[10];
  size_t hlen = 0;

  hdr[0] = 0x81; /* FIN + text */
  if (n <= 125) {
    hdr[1] = (unsigned char)n;
    hlen = 2;
  } else if (n <= 65535) {
    hdr[1] = 126;
    hdr[2] = (unsigned char)((n >> 8) & 0xFF);
    hdr[3] = (unsigned char)(n & 0xFF);
    hlen = 4;
  } else {
    hdr[1] = 127;
    for (int i = 0; i < 8; i++) hdr[2 + i] = (unsigned char)((n >> (56 - 8*i)) & 0xFF);
    hlen = 10;
  }

  if (write_all(fd, hdr, hlen) < 0) return -1;
  if (write_all(fd, s, n) < 0) return -1;
  return 0;
}

static int ws_close(int fd, unsigned short code) {
  unsigned char frame[4];
  frame[0] = 0x88; /* FIN + close */
  frame[1] = 2;
  frame[2] = (unsigned char)((code >> 8) & 0xFF);
  frame[3] = (unsigned char)(code & 0xFF);
  return write_all(fd, frame, sizeof(frame)) < 0 ? -1 : 0;
}

/* Parse exactly one WS text frame from peer buffer if available.
 * Returns:
 *  1 -> got a text payload in out (null-terminated)
 *  0 -> need more data
 * -1 -> protocol error / close
 */
static int ws_try_read_text(peer_t* p, char* out, size_t outcap) {
  if (p->rx_len < 2) return 0;

  unsigned char b0 = p->rx[0];
  unsigned char b1 = p->rx[1];

  int fin = (b0 & 0x80) != 0;
  unsigned char opcode = (unsigned char)(b0 & 0x0F);
  int masked = (b1 & 0x80) != 0;
  unsigned long long len = (unsigned long long)(b1 & 0x7F);

  if (!fin) return -1; /* MVP: no fragmentation */
  if (!masked) return -1; /* client->server must be masked */

  size_t off = 2;
  if (len == 126) {
    if (p->rx_len < off + 2) return 0;
    len = ((unsigned long long)p->rx[off] << 8) | (unsigned long long)p->rx[off+1];
    off += 2;
  } else if (len == 127) {
    if (p->rx_len < off + 8) return 0;
    len = 0;
    for (int i = 0; i < 8; i++) len = (len << 8) | (unsigned long long)p->rx[off + i];
    off += 8;
  }

  if (p->rx_len < off + 4) return 0;
  unsigned char mask[4];
  memcpy(mask, p->rx + off, 4);
  off += 4;

  if (p->rx_len < off + (size_t)len) return 0;

  if (opcode == 0x8) { /* close */
    return -1;
  }
  if (opcode != 0x1) { /* MVP: only text */
    /* consume frame */
    memmove(p->rx, p->rx + off + (size_t)len, p->rx_len - (off + (size_t)len));
    p->rx_len -= (off + (size_t)len);
    return 0;
  }

  if ((size_t)len + 1 > outcap) return -1;

  for (size_t i = 0; i < (size_t)len; i++) {
    out[i] = (char)(p->rx[off + i] ^ mask[i & 3]);
  }
  out[len] = 0;

  /* consume frame */
  memmove(p->rx, p->rx + off + (size_t)len, p->rx_len - (off + (size_t)len));
  p->rx_len -= (off + (size_t)len);
  return 1;
}

/* =========================
 * Proposal parsing (MVP crude)
 * ========================= */

static int find_u64(const char* s, const char* key, unsigned long long* out) {
  const char* p = strstr(s, key);
  if (!p) return 0;
  p = strchr(p, ':'); if (!p) return 0;
  p++;
  while (*p == ' ' || *p == '\"') p++;
  *out = strtoull(p, NULL, 10);
  return 1;
}

static int find_str(const char* s, const char* key, char* out, size_t cap) {
  const char* p = strstr(s, key);
  if (!p) return 0;
  p = strchr(p, ':'); if (!p) return 0;
  p++;
  while (*p == ' ' || *p == '\"') p++;
  const char* q = strchr(p, '\"');
  if (!q) return 0;
  size_t n = (size_t)(q - p);
  if (n + 1 > cap) return 0;
  memcpy(out, p, n);
  out[n] = 0;
  return 1;
}

/* =========================
 * Event log + apply + diff broadcast
 * ========================= */

static sp_eid_t g_seq = 1;

typedef struct {
  peer_t* peers;
  int peer_count;
  int mirror_stdout;
  sp_eid_t seq;
  char origin_client[64];
  unsigned long long origin_req;
} diff_bus_t;

/* Broadcast helper */
static void bus_broadcast(diff_bus_t* bus, const char* line) {
  size_t n = strlen(line);
  for (int i = 0; i < bus->peer_count; i++) {
    if (!bus->peers[i].is_ws) continue;
    ws_send_text(bus->peers[i].fd, line, n);
  }
  if (bus->mirror_stdout) fputs(line, stdout), fflush(stdout);
}

static void d_begin(void* ctx, sp_eid_t seq) { ((diff_bus_t*)ctx)->seq = seq; }

static void d_add_obj(void* ctx, sp_handle_t id, sp_objtype_t type, sp_handle_t rep) {
  diff_bus_t* b = (diff_bus_t*)ctx;
  char line[512];
  snprintf(line, sizeof(line),
    "{\"seq\":%llu,\"origin\":{\"client\":\"%s\",\"req\":%llu},"
    "\"add_obj\":{\"id\":%llu,\"type\":%u,\"rep\":%llu}}\n",
    (unsigned long long)b->seq, b->origin_client, (unsigned long long)b->origin_req,
    (unsigned long long)id, (unsigned)type, (unsigned long long)rep);
  bus_broadcast(b, line);
}

static void d_rem_obj(void* ctx, sp_handle_t id) {
  diff_bus_t* b = (diff_bus_t*)ctx;
  char line[256];
  snprintf(line, sizeof(line),
    "{\"seq\":%llu,\"origin\":{\"client\":\"%s\",\"req\":%llu},\"rem_obj\":%llu}\n",
    (unsigned long long)b->seq, b->origin_client, (unsigned long long)b->origin_req,
    (unsigned long long)id);
  bus_broadcast(b, line);
}

static void d_add_edge(void* ctx, sp_handle_t src, sp_handle_t dst, sp_reltype_t rel, uint32_t flags) {
  diff_bus_t* b = (diff_bus_t*)ctx;
  char line[512];
  snprintf(line, sizeof(line),
    "{\"seq\":%llu,\"origin\":{\"client\":\"%s\",\"req\":%llu},"
    "\"add_edge\":{\"src\":%llu,\"dst\":%llu,\"rel\":%u,\"flags\":%u}}\n",
    (unsigned long long)b->seq, b->origin_client, (unsigned long long)b->origin_req,
    (unsigned long long)src, (unsigned long long)dst, (unsigned)rel, flags);
  bus_broadcast(b, line);
}

static void d_rem_edge(void* ctx, sp_handle_t src, sp_handle_t dst, sp_reltype_t rel) {
  diff_bus_t* b = (diff_bus_t*)ctx;
  char line[512];
  snprintf(line, sizeof(line),
    "{\"seq\":%llu,\"origin\":{\"client\":\"%s\",\"req\":%llu},"
    "\"rem_edge\":{\"src\":%llu,\"dst\":%llu,\"rel\":%u}}\n",
    (unsigned long long)b->seq, b->origin_client, (unsigned long long)b->origin_req,
    (unsigned long long)src, (unsigned long long)dst, (unsigned)rel);
  bus_broadcast(b, line);
}

static void d_end(void* ctx) { (void)ctx; }

static sp_mk_diff_sink_t make_sink(diff_bus_t* b) {
  return (sp_mk_diff_sink_t){
    .ctx = b,
    .begin = d_begin,
    .add_obj = d_add_obj,
    .rem_obj = d_rem_obj,
    .add_edge = d_add_edge,
    .rem_edge = d_rem_edge,
    .end = d_end
  };
}

static int write_event(FILE* f, const sp_event_hdr_t* hdr, const void* payload) {
  if (fwrite(hdr, 1, sizeof(*hdr), f) != sizeof(*hdr)) return 0;
  if (hdr->payload_len) {
    if (fwrite(payload, 1, hdr->payload_len, f) != hdr->payload_len) return 0;
  }
  fflush(f);
  return 1;
}

static void send_ack(peer_t* origin, unsigned long long req, const char* status, unsigned long long seq, const char* code) {
  char line[256];
  if (strcmp(status, "ok") == 0) {
    snprintf(line, sizeof(line), "{\"t\":\"ack\",\"req\":%llu,\"status\":\"ok\",\"seq\":%llu}\n",
             req, seq);
  } else {
    snprintf(line, sizeof(line), "{\"t\":\"ack\",\"req\":%llu,\"status\":\"err\",\"code\":\"%s\"}\n",
             req, code ? code : "E");
  }
  ws_send_text(origin->fd, line, strlen(line));
}

/* =========================
 * WebSocket handshake
 * ========================= */

static int http_read_request(int fd, char* buf, size_t cap) {
  size_t n = 0;
  while (n + 1 < cap) {
    ssize_t r = recv(fd, buf + n, cap - 1 - n, 0);
    if (r < 0) {
      if (errno == EINTR) continue;
      if (errno == EAGAIN || errno == EWOULDBLOCK) break;
      return -1;
    }
    if (r == 0) break;
    n += (size_t)r;
    buf[n] = 0;
    if (strstr(buf, "\r\n\r\n")) return (int)n;
  }
  return (int)n;
}

static int header_value(const char* req, const char* key, char* out, size_t outcap) {
  const char* p = strcasestr(req, key);
  if (!p) return 0;
  p = strchr(p, ':');
  if (!p) return 0;
  p++;
  while (*p == ' ') p++;
  const char* q = strstr(p, "\r\n");
  if (!q) return 0;
  size_t n = (size_t)(q - p);
  if (n + 1 > outcap) return 0;
  memcpy(out, p, n);
  out[n] = 0;
  return 1;
}

static int ws_handshake(int fd) {
  char req[4096];
  int n = http_read_request(fd, req, sizeof(req));
  if (n <= 0) return -1;

  char key[256];
  if (!header_value(req, "Sec-WebSocket-Key", key, sizeof(key))) return -1;

  /* accept = base64( sha1(key + GUID) ) */
  const char* guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  char concat[512];
  snprintf(concat, sizeof(concat), "%s%s", key, guid);

  sha1_t s;
  sha1_init(&s);
  sha1_update(&s, (const unsigned char*)concat, strlen(concat));
  unsigned char digest[20];
  sha1_final(&s, digest);

  char accept[64];
  base64_encode(digest, sizeof(digest), accept, sizeof(accept));

  char resp[512];
  int rn = snprintf(resp, sizeof(resp),
    "HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: %s\r\n"
    "\r\n", accept);

  return write_all(fd, resp, (size_t)rn) < 0 ? -1 : 0;
}

/* =========================
 * Main: listen + poll loop
 * ========================= */

int main(int argc, char** argv) {
  const char* log_path = (argc >= 2) ? argv[1] : "events.bin";
  int port = (argc >= 3) ? atoi(argv[2]) : 8765;

  FILE* flog = fopen(log_path, "ab+");
  if (!flog) die("events.bin");

  /* TODO: replay existing log into mk to reconstruct state before serving.
     Your integrated build already does this; keep it there. */

  sp_mk_t mk;
  if (sp_mk_init(&mk, 256, 512) != SP_OK) die("mk_init");

  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  if (lfd < 0) die("socket");

  int one = 1;
  setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons((unsigned short)port);

  if (bind(lfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) die("bind");
  if (listen(lfd, 32) < 0) die("listen");
  set_nonblock(lfd);

  peer_t peers[64];
  int peer_count = 0;

  fprintf(stderr, "sp_inject_ws listening on :%d (log=%s)\n", port, log_path);

  while (1) {
    struct pollfd pfds[1 + 64];
    int nfds = 0;

    pfds[nfds++] = (struct pollfd){ .fd = lfd, .events = POLLIN };

    for (int i = 0; i < peer_count; i++) {
      pfds[nfds++] = (struct pollfd){ .fd = peers[i].fd, .events = POLLIN };
    }

    int pr = poll(pfds, nfds, 50);
    if (pr < 0) {
      if (errno == EINTR) continue;
      die("poll");
    }

    /* accept */
    if (pfds[0].revents & POLLIN) {
      for (;;) {
        int cfd = accept(lfd, NULL, NULL);
        if (cfd < 0) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) break;
          break;
        }
        set_nonblock(cfd);

        if (ws_handshake(cfd) == 0) {
          if (peer_count < 64) {
            peer_t* p = &peers[peer_count++];
            memset(p, 0, sizeof(*p));
            p->fd = cfd;
            p->is_ws = 1;
            snprintf(p->name, sizeof(p->name), "conn-%d", cfd);

            /* optional handshake/capabilities */
            const char* hello =
              "{\"t\":\"hello\",\"cap\":{\"ops\":[\"MERGE\",\"LINK\",\"UNLINK\",\"COLLAPSE\",\"SET_LAYOUT\"],"
              "\"transport\":\"ws\",\"ordering\":\"server_seq\"}}\n";
            ws_send_text(cfd, hello, strlen(hello));
          } else {
            ws_close(cfd, 1001);
            close(cfd);
          }
        } else {
          close(cfd);
        }
      }
    }

    /* handle clients */
    for (int i = 0; i < peer_count; ) {
      peer_t* p = &peers[i];
      int pidx = 1 + i;
      int dead = 0;

      if (pfds[pidx].revents & (POLLHUP | POLLERR)) dead = 1;

      if (!dead && (pfds[pidx].revents & POLLIN)) {
        unsigned char buf[4096];
        ssize_t n = recv(p->fd, buf, sizeof(buf), 0);
        if (n <= 0) {
          if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            /* nothing */
          } else {
            dead = 1;
          }
        } else {
          if (p->rx_len + (size_t)n > sizeof(p->rx)) {
            dead = 1;
          } else {
            memcpy(p->rx + p->rx_len, buf, (size_t)n);
            p->rx_len += (size_t)n;

            /* drain frames */
            char line[8192];
            for (;;) {
              int got = ws_try_read_text(p, line, sizeof(line));
              if (got == 0) break;
              if (got < 0) { dead = 1; break; }

              /* Process proposal line -> canonical event -> apply -> diffs */
              char op[32] = {0};
              unsigned long long req = 0;
              char client[64] = {0};

              /* origin: prefer explicit client field; else use connection name */
              if (!find_str(line, "\"client\"", client, sizeof(client))) {
                strncpy(client, p->name, sizeof(client)-1);
              }
              find_u64(line, "\"req\"", &req);
              if (!find_str(line, "\"op\"", op, sizeof(op))) {
                send_ack(p, req, "err", 0, "EINVAL");
                continue;
              }

              diff_bus_t bus;
              memset(&bus, 0, sizeof(bus));
              bus.peers = peers;
              bus.peer_count = peer_count;
              bus.mirror_stdout = 0; /* set to 1 if you want stdout mirror */
              strncpy(bus.origin_client, client, sizeof(bus.origin_client)-1);
              bus.origin_req = req;

              sp_mk_diff_sink_t sink = make_sink(&bus);

              sp_event_hdr_t hdr = {0};
              hdr.eid = g_seq++;
              hdr.tick = 0;
              hdr.flags = SP_F_DETERMINISTIC | SP_F_RECORD_EVENT;
              hdr.user_tag = (uint64_t)req;

              unsigned char payload[1024];
              memset(payload, 0, sizeof(payload));

              if (strcmp(op, "MERGE") == 0) {
                unsigned long long a=0,b=0,mode=SP_MG_DEFAULT;
                if (!find_u64(line, "\"a\"", &a) || !find_u64(line, "\"b\"", &b)) { send_ack(p, req, "err", 0, "EINVAL"); continue; }
                find_u64(line, "\"mode\"", &mode);

                sp_evt_merge_t* e = (sp_evt_merge_t*)payload;
                e->p.a = (sp_handle_t)a;
                e->p.b = (sp_handle_t)b;
                e->mode = (uint32_t)mode;
                e->has_policy = 0;

                hdr.op = SP_OP_MERGE;
                hdr.payload_len = sizeof(sp_evt_merge_t);

              } else if (strcmp(op, "LINK") == 0) {
                unsigned long long src=0,dst=0,rel=0,flags=0;
                if (!find_u64(line, "\"src\"", &src) || !find_u64(line, "\"dst\"", &dst) || !find_u64(line, "\"rel\"", &rel)) { send_ack(p, req, "err", 0, "EINVAL"); continue; }
                find_u64(line, "\"flags\"", &flags);

                sp_evt_link_t* e = (sp_evt_link_t*)payload;
                e->p.a = (sp_handle_t)src;
                e->p.b = (sp_handle_t)dst;
                e->reltype = (uint32_t)rel;
                e->link_flags = (uint32_t)flags;
                e->has_meta = 0;

                hdr.op = SP_OP_LINK;
                hdr.payload_len = sizeof(sp_evt_link_t);

              } else if (strcmp(op, "UNLINK") == 0) {
                unsigned long long src=0,dst=0,rel=0;
                if (!find_u64(line, "\"src\"", &src) || !find_u64(line, "\"dst\"", &dst) || !find_u64(line, "\"rel\"", &rel)) { send_ack(p, req, "err", 0, "EINVAL"); continue; }

                sp_evt_unlink_t* e = (sp_evt_unlink_t*)payload;
                e->p.a = (sp_handle_t)src;
                e->p.b = (sp_handle_t)dst;
                e->reltype = (uint32_t)rel;

                hdr.op = SP_OP_UNLINK;
                hdr.payload_len = sizeof(sp_evt_unlink_t);

              } else if (strcmp(op, "COLLAPSE") == 0) {
                unsigned long long region=0,cflags=0;
                if (!find_u64(line, "\"region\"", &region)) { send_ack(p, req, "err", 0, "EINVAL"); continue; }
                find_u64(line, "\"collapse_flags\"", &cflags);

                sp_evt_collapse_t* e = (sp_evt_collapse_t*)payload;
                e->p.a = (sp_handle_t)region;
                e->p.b = 0;
                e->collapse_flags = (uint32_t)cflags;
                e->has_policy = 0;

                hdr.op = SP_OP_COLLAPSE;
                hdr.payload_len = sizeof(sp_evt_collapse_t);

              } else if (strcmp(op, "SET_LAYOUT") == 0) {
                unsigned long long id=0;
                if (!find_u64(line, "\"id\"", &id)) { send_ack(p, req, "err", 0, "EINVAL"); continue; }

                /* MVP float parsing trick: send x/y/z/radius scaled by 1000 as integers
                   (your client can do this trivially). */
                unsigned long long xi=0, yi=0, zi=0, ri=500, lhflags=0;
                find_u64(line, "\"x\"", &xi);
                find_u64(line, "\"y\"", &yi);
                find_u64(line, "\"z\"", &zi);
                find_u64(line, "\"radius\"", &ri);
                find_u64(line, "\"lhflags\"", &lhflags);

                struct __attribute__((packed)) {
                  sp_evt_set_meta_t e;
                  sp_layout_hint_t lh;
                } p2;
                memset(&p2, 0, sizeof(p2));
                p2.e.p.a = (sp_handle_t)id;
                p2.e.has_meta = 1;
                p2.e.meta.type = SP_MT_LAYOUT_HINT;
                p2.e.meta.len  = sizeof(sp_layout_hint_t);
                p2.e.meta.off  = (uint32_t)offsetof(typeof(p2), lh);

                p2.lh.version = SP_LAYOUT_VERSION;
                p2.lh.flags   = (uint32_t)lhflags;
                p2.lh.x = (float)((long long)xi) / 1000.0f;
                p2.lh.y = (float)((long long)yi) / 1000.0f;
                p2.lh.z = (float)((long long)zi) / 1000.0f;
                p2.lh.radius = (float)ri / 1000.0f;

                hdr.op = SP_OP_SET_META;
                hdr.payload_len = (uint32_t)sizeof(p2);
                memcpy(payload, &p2, sizeof(p2));

              } else {
                send_ack(p, req, "err", 0, "EOP");
                continue;
              }

              if (!write_event(flog, &hdr, payload)) {
                send_ack(p, req, "err", 0, "EIO");
                continue;
              }

              sp_status_t as = sp_mk_apply_with_diff(&mk, &hdr, payload, &sink);
              if (as != SP_OK) {
                send_ack(p, req, "err", 0, "EAPPLY");
                continue;
              }

              send_ack(p, req, "ok", (unsigned long long)hdr.eid, NULL);
            }
          }
        }
      }

      if (dead) {
        ws_close(p->fd, 1001);
        close(p->fd);
        peers[i] = peers[peer_count - 1];
        peer_count--;
        continue;
      }
      i++;
    }
  }

  sp_mk_free(&mk);
  fclose(flog);
  close(lfd);
  return 0;
}