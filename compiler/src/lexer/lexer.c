#define _POSIX_C_SOURCE 200809L
/* lexer.c — Spherepop lexer. */
#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

Lexer *lexer_create(const char *source) {
    Lexer *l = calloc(1, sizeof(Lexer));
    l->src     = source;
    l->src_len = strlen(source);
    l->line    = 1;
    l->col     = 1;
    return l;
}

void lexer_free(Lexer *l) { free(l); }

static char cur(Lexer *l) {
    return l->pos < l->src_len ? l->src[l->pos] : '\0';
}
static char peek1(Lexer *l) {
    return (l->pos + 1) < l->src_len ? l->src[l->pos + 1] : '\0';
}
static char advance(Lexer *l) {
    char c = l->src[l->pos++];
    if (c == '\n') { l->line++; l->col = 1; } else l->col++;
    return c;
}

static Token *make_token(TokenType type, const char *lex, int line, int col) {
    Token *t = calloc(1, sizeof(Token));
    t->type   = type;
    t->lexeme = lex ? strdup(lex) : NULL;
    t->line   = line;
    t->col    = col;
    return t;
}

static TokenType keyword_or_ident(const char *s) {
    if (strcmp(s, "let")      == 0) return TOK_LET;
    if (strcmp(s, "mut")      == 0) return TOK_MUT;
    if (strcmp(s, "bubble")   == 0) return TOK_BUBBLE;
    if (strcmp(s, "pop")      == 0) return TOK_POP;
    if (strcmp(s, "refuse")   == 0) return TOK_REFUSE;
    if (strcmp(s, "collapse") == 0) return TOK_COLLAPSE;
    if (strcmp(s, "bind")     == 0) return TOK_BIND;
    if (strcmp(s, "if")       == 0) return TOK_IF;
    if (strcmp(s, "else")     == 0) return TOK_ELSE;
    if (strcmp(s, "while")    == 0) return TOK_WHILE;
    if (strcmp(s, "for")      == 0) return TOK_FOR;
    if (strcmp(s, "return")   == 0) return TOK_RETURN;
    if (strcmp(s, "fn")       == 0) return TOK_FN;
    if (strcmp(s, "print")    == 0) return TOK_PRINT;
    if (strcmp(s, "observe")  == 0) return TOK_OBSERVE;
    if (strcmp(s, "history")  == 0) return TOK_HISTORY;
    if (strcmp(s, "trajectory")== 0)return TOK_TRAJECTORY;
    if (strcmp(s, "import")   == 0) return TOK_IMPORT;
    if (strcmp(s, "export")   == 0) return TOK_EXPORT;
    if (strcmp(s, "true")     == 0) return TOK_BOOL;
    if (strcmp(s, "false")    == 0) return TOK_BOOL;
    if (strcmp(s, "null")     == 0) return TOK_NULL;
    return TOK_IDENT;
}

Token *lexer_next(Lexer *l) {
    /* Skip whitespace and comments */
    while (l->pos < l->src_len) {
        char c = cur(l);
        if (isspace(c)) { advance(l); continue; }
        if (c == '/' && peek1(l) == '/') {
            while (l->pos < l->src_len && cur(l) != '\n') advance(l);
            continue;
        }
        if (c == '/' && peek1(l) == '*') {
            advance(l); advance(l);
            while (l->pos < l->src_len - 1) {
                if (cur(l) == '*' && peek1(l) == '/') { advance(l); advance(l); break; }
                advance(l);
            }
            continue;
        }
        break;
    }

    if (l->pos >= l->src_len) return make_token(TOK_EOF, "", l->line, l->col);

    int line = l->line, col = l->col;
    char c = cur(l);

    /* Numbers */
    if (isdigit(c) || (c == '.' && isdigit(peek1(l)))) {
        char buf[64]; int i = 0;
        int is_float = 0;
        while (l->pos < l->src_len && (isdigit(cur(l)) || cur(l) == '.')) {
            if (cur(l) == '.') is_float = 1;
            if (i < 63) buf[i++] = advance(l); else advance(l);
        }
        buf[i] = '\0';
        Token *t = make_token(is_float ? TOK_FLOAT : TOK_INT, buf, line, col);
        if (is_float) t->literal.fval = atof(buf);
        else          t->literal.ival = atoll(buf);
        return t;
    }

    /* Strings */
    if (c == '"') {
        advance(l);
        char buf[1024]; int i = 0;
        while (l->pos < l->src_len && cur(l) != '"') {
            char ch = advance(l);
            if (ch == '\\') {
                char esc = advance(l);
                switch (esc) {
                    case 'n': ch = '\n'; break;
                    case 't': ch = '\t'; break;
                    case '"': ch = '"';  break;
                    case '\\': ch = '\\'; break;
                    default:  ch = esc; break;
                }
            }
            if (i < 1023) buf[i++] = ch;
        }
        if (cur(l) == '"') advance(l);
        buf[i] = '\0';
        return make_token(TOK_STRING, buf, line, col);
    }

    /* Identifiers / keywords */
    if (isalpha(c) || c == '_') {
        char buf[128]; int i = 0;
        while (l->pos < l->src_len && (isalnum(cur(l)) || cur(l) == '_')) {
            if (i < 127) buf[i++] = advance(l); else advance(l);
        }
        buf[i] = '\0';
        TokenType kw = keyword_or_ident(buf);
        Token *t = make_token(kw, buf, line, col);
        if (kw == TOK_BOOL) t->literal.ival = strcmp(buf, "true") == 0 ? 1 : 0;
        return t;
    }

    advance(l);
    /* Two-character operators */
    char n = cur(l);
    if (c == '=' && n == '=') { advance(l); return make_token(TOK_EQ,  "==", line, col); }
    if (c == '!' && n == '=') { advance(l); return make_token(TOK_NEQ, "!=", line, col); }
    if (c == '<' && n == '=') { advance(l); return make_token(TOK_LEQ, "<=", line, col); }
    if (c == '>' && n == '=') { advance(l); return make_token(TOK_GEQ, ">=", line, col); }
    if (c == '&' && n == '&') { advance(l); return make_token(TOK_AND, "&&", line, col); }
    if (c == '|' && n == '|') { advance(l); return make_token(TOK_OR,  "||", line, col); }
    if (c == '-' && n == '>') { advance(l); return make_token(TOK_ARROW,"->",line, col); }

    /* Single-character tokens */
    char s[2] = {c, 0};
    switch (c) {
        case '+': return make_token(TOK_PLUS,    s, line, col);
        case '-': return make_token(TOK_MINUS,   s, line, col);
        case '*': return make_token(TOK_STAR,    s, line, col);
        case '/': return make_token(TOK_SLASH,   s, line, col);
        case '%': return make_token(TOK_PERCENT, s, line, col);
        case '^': return make_token(TOK_CARET,   s, line, col);
        case '<': return make_token(TOK_LT,      s, line, col);
        case '>': return make_token(TOK_GT,      s, line, col);
        case '=': return make_token(TOK_ASSIGN,  s, line, col);
        case '!': return make_token(TOK_NOT,     s, line, col);
        case '.': return make_token(TOK_DOT,     s, line, col);
        case '(': return make_token(TOK_LPAREN,  s, line, col);
        case ')': return make_token(TOK_RPAREN,  s, line, col);
        case '{': return make_token(TOK_LBRACE,  s, line, col);
        case '}': return make_token(TOK_RBRACE,  s, line, col);
        case '[': return make_token(TOK_LBRACKET,s, line, col);
        case ']': return make_token(TOK_RBRACKET,s, line, col);
        case ';': return make_token(TOK_SEMI,    s, line, col);
        case ',': return make_token(TOK_COMMA,   s, line, col);
        case ':': return make_token(TOK_COLON,   s, line, col);
        default: {
            char msg[32]; snprintf(msg, sizeof(msg), "unexpected '%c'", c);
            return make_token(TOK_ERROR, msg, line, col);
        }
    }
}

Token *lexer_peek(Lexer *l) {
    int saved_pos  = l->pos;
    int saved_line = l->line;
    int saved_col  = l->col;
    Token *t = lexer_next(l);
    l->pos  = saved_pos;
    l->line = saved_line;
    l->col  = saved_col;
    return t;
}

void lexer_tokenize_all(Lexer *l, Token **out, int *count) {
    int cap = 64, n = 0;
    *out = malloc(cap * sizeof(Token));
    Token *t;
    do {
        t = lexer_next(l);
        if (n == cap) { cap *= 2; *out = realloc(*out, cap * sizeof(Token)); }
        (*out)[n++] = *t;
        free(t);
    } while ((*out)[n-1].type != TOK_EOF);
    *count = n;
}
