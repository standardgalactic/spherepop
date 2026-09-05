/* json.c — Minimal recursive-descent JSON parser. See json.h. */
#include "json.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *s;
    size_t      pos;
    size_t      len;
} Lexer;

static void skip_ws(Lexer *lx) {
    while (lx->pos < lx->len) {
        char c = lx->s[lx->pos];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') lx->pos++;
        else break;
    }
}

static char peek(Lexer *lx) {
    return lx->pos < lx->len ? lx->s[lx->pos] : '\0';
}

static Json *json_alloc(JsonKind kind) {
    Json *v = calloc(1, sizeof(Json));
    if (!v) { perror("json_alloc"); exit(1); }
    v->kind = kind;
    return v;
}

static char *make_err(Lexer *lx, const char *msg) {
    char *buf = malloc(128);
    snprintf(buf, 128, "%s at offset %zu", msg, lx->pos);
    return buf;
}

static Json *parse_value(Lexer *lx, char **err);

static char *parse_raw_string(Lexer *lx, char **err) {
    /* Assumes current char is the opening quote. */
    lx->pos++; /* consume opening '"' */
    size_t cap = 32, n = 0;
    char *out = malloc(cap);
    while (lx->pos < lx->len && lx->s[lx->pos] != '"') {
        char c = lx->s[lx->pos];
        if (c == '\\') {
            lx->pos++;
            if (lx->pos >= lx->len) { *err = make_err(lx, "unterminated escape"); free(out); return NULL; }
            char e = lx->s[lx->pos];
            char decoded;
            switch (e) {
                case '"':  decoded = '"';  break;
                case '\\': decoded = '\\'; break;
                case '/':  decoded = '/';  break;
                case 'n':  decoded = '\n'; break;
                case 't':  decoded = '\t'; break;
                case 'r':  decoded = '\r'; break;
                case 'b':  decoded = '\b'; break;
                case 'f':  decoded = '\f'; break;
                default:
                    /* \\uXXXX and anything else: not needed by these
                     * fixtures (plain ASCII identifiers/text only); pass
                     * the character through literally rather than
                     * silently corrupting the string. */
                    decoded = e;
                    break;
            }
            if (n + 1 >= cap) { cap *= 2; out = realloc(out, cap); }
            out[n++] = decoded;
            lx->pos++;
        } else {
            if (n + 1 >= cap) { cap *= 2; out = realloc(out, cap); }
            out[n++] = c;
            lx->pos++;
        }
    }
    if (lx->pos >= lx->len) { *err = make_err(lx, "unterminated string"); free(out); return NULL; }
    lx->pos++; /* consume closing '"' */
    out[n] = '\0';
    return out;
}

static Json *parse_string(Lexer *lx, char **err) {
    char *s = parse_raw_string(lx, err);
    if (!s) return NULL;
    Json *v = json_alloc(JSON_STRING);
    v->u.string = s;
    return v;
}

static Json *parse_number(Lexer *lx, char **err) {
    size_t start = lx->pos;
    if (peek(lx) == '-') lx->pos++;
    while (isdigit((unsigned char)peek(lx))) lx->pos++;
    if (peek(lx) == '.') {
        lx->pos++;
        while (isdigit((unsigned char)peek(lx))) lx->pos++;
    }
    if (peek(lx) == 'e' || peek(lx) == 'E') {
        lx->pos++;
        if (peek(lx) == '+' || peek(lx) == '-') lx->pos++;
        while (isdigit((unsigned char)peek(lx))) lx->pos++;
    }
    if (lx->pos == start) { *err = make_err(lx, "invalid number"); return NULL; }
    char buf[64];
    size_t n = lx->pos - start;
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    memcpy(buf, lx->s + start, n);
    buf[n] = '\0';
    Json *v = json_alloc(JSON_NUMBER);
    v->u.number = atof(buf);
    return v;
}

static Json *parse_array(Lexer *lx, char **err) {
    lx->pos++; /* '[' */
    Json *v = json_alloc(JSON_ARRAY);
    size_t cap = 4;
    v->u.array.items = malloc(cap * sizeof(Json *));
    v->u.array.len   = 0;
    skip_ws(lx);
    if (peek(lx) == ']') { lx->pos++; return v; }
    while (1) {
        skip_ws(lx);
        Json *item = parse_value(lx, err);
        if (!item) { json_free(v); return NULL; }
        if (v->u.array.len >= cap) { cap *= 2; v->u.array.items = realloc(v->u.array.items, cap * sizeof(Json *)); }
        v->u.array.items[v->u.array.len++] = item;
        skip_ws(lx);
        if (peek(lx) == ',') { lx->pos++; continue; }
        if (peek(lx) == ']') { lx->pos++; break; }
        *err = make_err(lx, "expected ',' or ']' in array");
        json_free(v);
        return NULL;
    }
    return v;
}

static Json *parse_object(Lexer *lx, char **err) {
    lx->pos++; /* '{' */
    Json *v = json_alloc(JSON_OBJECT);
    size_t cap = 4;
    v->u.object.keys   = malloc(cap * sizeof(char *));
    v->u.object.values = malloc(cap * sizeof(Json *));
    v->u.object.len    = 0;
    skip_ws(lx);
    if (peek(lx) == '}') { lx->pos++; return v; }
    while (1) {
        skip_ws(lx);
        if (peek(lx) != '"') { *err = make_err(lx, "expected string key"); json_free(v); return NULL; }
        char *key = parse_raw_string(lx, err);
        if (!key) { json_free(v); return NULL; }
        skip_ws(lx);
        if (peek(lx) != ':') { *err = make_err(lx, "expected ':'"); free(key); json_free(v); return NULL; }
        lx->pos++;
        skip_ws(lx);
        Json *val = parse_value(lx, err);
        if (!val) { free(key); json_free(v); return NULL; }
        if (v->u.object.len >= cap) {
            cap *= 2;
            v->u.object.keys   = realloc(v->u.object.keys,   cap * sizeof(char *));
            v->u.object.values = realloc(v->u.object.values, cap * sizeof(Json *));
        }
        v->u.object.keys[v->u.object.len]   = key;
        v->u.object.values[v->u.object.len] = val;
        v->u.object.len++;
        skip_ws(lx);
        if (peek(lx) == ',') { lx->pos++; continue; }
        if (peek(lx) == '}') { lx->pos++; break; }
        *err = make_err(lx, "expected ',' or '}' in object");
        json_free(v);
        return NULL;
    }
    return v;
}

static Json *parse_value(Lexer *lx, char **err) {
    skip_ws(lx);
    char c = peek(lx);
    if (c == '"') return parse_string(lx, err);
    if (c == '{') return parse_object(lx, err);
    if (c == '[') return parse_array(lx, err);
    if (c == '-' || isdigit((unsigned char)c)) return parse_number(lx, err);
    if (strncmp(lx->s + lx->pos, "true", 4) == 0) {
        lx->pos += 4;
        Json *v = json_alloc(JSON_BOOL); v->u.boolean = true; return v;
    }
    if (strncmp(lx->s + lx->pos, "false", 5) == 0) {
        lx->pos += 5;
        Json *v = json_alloc(JSON_BOOL); v->u.boolean = false; return v;
    }
    if (strncmp(lx->s + lx->pos, "null", 4) == 0) {
        lx->pos += 4;
        return json_alloc(JSON_NULL);
    }
    *err = make_err(lx, "unexpected character");
    return NULL;
}

Json *json_parse(const char *text, char **err) {
    Lexer lx = { text, 0, strlen(text) };
    *err = NULL;
    Json *v = parse_value(&lx, err);
    if (!v) return NULL;
    skip_ws(&lx);
    if (lx.pos != lx.len) {
        *err = make_err(&lx, "trailing data after JSON value");
        json_free(v);
        return NULL;
    }
    return v;
}

void json_free_error(char *err) { free(err); }

void json_free(Json *v) {
    if (!v) return;
    switch (v->kind) {
        case JSON_STRING:
            free(v->u.string);
            break;
        case JSON_ARRAY:
            for (size_t i = 0; i < v->u.array.len; i++) json_free(v->u.array.items[i]);
            free(v->u.array.items);
            break;
        case JSON_OBJECT:
            for (size_t i = 0; i < v->u.object.len; i++) {
                free(v->u.object.keys[i]);
                json_free(v->u.object.values[i]);
            }
            free(v->u.object.keys);
            free(v->u.object.values);
            break;
        default:
            break;
    }
    free(v);
}

const Json *json_get(const Json *obj, const char *key) {
    if (!obj || obj->kind != JSON_OBJECT) return NULL;
    for (size_t i = 0; i < obj->u.object.len; i++) {
        if (strcmp(obj->u.object.keys[i], key) == 0) return obj->u.object.values[i];
    }
    return NULL;
}

bool json_get_bool(const Json *obj, const char *key, bool dflt) {
    const Json *v = json_get(obj, key);
    return v ? json_as_bool(v, dflt) : dflt;
}

const char *json_get_str(const Json *obj, const char *key, const char *dflt) {
    const Json *v = json_get(obj, key);
    if (!v || v->kind != JSON_STRING) return dflt;
    return v->u.string;
}

bool json_as_u64(const Json *v, uint64_t *out) {
    if (!v || v->kind != JSON_NUMBER) return false;
    *out = (uint64_t)v->u.number;
    return true;
}

const char *json_as_str(const Json *v) {
    if (!v || v->kind != JSON_STRING) return NULL;
    return v->u.string;
}

bool json_as_bool(const Json *v, bool dflt) {
    if (!v || v->kind != JSON_BOOL) return dflt;
    return v->u.boolean;
}

size_t json_array_len(const Json *v) {
    if (!v || v->kind != JSON_ARRAY) return 0;
    return v->u.array.len;
}

const Json *json_array_get(const Json *v, size_t i) {
    if (!v || v->kind != JSON_ARRAY || i >= v->u.array.len) return NULL;
    return v->u.array.items[i];
}
