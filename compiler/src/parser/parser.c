#define _POSIX_C_SOURCE 200809L
/* parser.c — Recursive descent parser for Spherepop.
 *
 * Operator precedence (low to high):
 *   || && == != < > <= >= + - * / % ^ (unary -)
 */
#include "parser.h"
#include "ast.h"
#include "../lexer/lexer.h"
#include "../lexer/token.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Parser *parser_create(Lexer *lexer) {
    Parser *p = calloc(1, sizeof(Parser));
    p->lexer   = lexer;
    p->current = lexer_next(lexer);
    p->peek    = lexer_next(lexer);
    return p;
}

void parser_free(Parser *p) {
    free(p->current);
    free(p->peek);
    free(p->last_error);
    free(p);
}

static Token *advance(Parser *p) {
    Token *old = p->current;
    p->current = p->peek;
    p->peek    = lexer_next(p->lexer);
    return old;
}

static int check(Parser *p, TokenType t) {
    return p->current->type == t;
}

static int match(Parser *p, TokenType t) {
    if (check(p, t)) { Token *tok = advance(p); free(tok); return 1; }
    return 0;
}

static Token *expect(Parser *p, TokenType t) {
    if (!check(p, t)) {
        char msg[128];
        snprintf(msg, sizeof(msg), "line %d: expected '%s', got '%s'",
                 p->current->line, token_type_name(t),
                 p->current->lexeme ? p->current->lexeme : "?");
        free(p->last_error);
        p->last_error = strdup(msg);
        p->error_count++;
        fprintf(stderr, "Parse error: %s\n", msg);
        return NULL;
    }
    return advance(p);
}

static Node *parse_expr(Parser *p);
static Node *parse_stmt(Parser *p);

static Node *node_alloc_null(int line, int col) {
    Node *n = calloc(1, sizeof(Node));
    n->kind = NODE_NULL_LIT;
    n->line = line; n->col = col;
    return n;
}

static Node *parse_primary(Parser *p) {
    Token *t = p->current;
    int line = t->line, col = t->col;

    if (t->type == TOK_INT) {
        long long v = t->literal.ival;
        advance(p);
        return node_make_int(v, line, col);
    }
    if (t->type == TOK_FLOAT) {
        double v = t->literal.fval;
        advance(p);
        return node_make_float(v, line, col);
    }
    if (t->type == TOK_STRING) {
        char *s = strdup(t->lexeme);
        advance(p);
        Node *n = node_make_string(s, line, col);
        free(s);
        return n;
    }
    if (t->type == TOK_BOOL) {
        int v = t->literal.ival;
        advance(p);
        return node_make_bool(v, line, col);
    }
    if (t->type == TOK_NULL) {
        advance(p);
        return node_alloc_null(line, col);
    }
    if (t->type == TOK_LPAREN) {
        advance(p);
        Node *expr = parse_expr(p);
        expect(p, TOK_RPAREN);
        return expr;
    }
    if (t->type == TOK_BUBBLE) {
        advance(p);
        expect(p, TOK_LBRACE);
        Node *stmts[256]; int n = 0;
        while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF))
            stmts[n++] = parse_stmt(p);
        expect(p, TOK_RBRACE);
        return node_make_bubble(stmts, n, line, col);
    }
    if (t->type == TOK_POP) {
        advance(p);
        expect(p, TOK_LPAREN);
        Node *child = parse_expr(p);
        expect(p, TOK_RPAREN);
        return node_make_pop(child, line, col);
    }
    if (t->type == TOK_REFUSE) {
        advance(p);
        if (check(p, TOK_LPAREN)) {
            advance(p);
            if (check(p, TOK_RPAREN)) {
                /* refuse() with no argument */
                advance(p);
                return node_make_refuse(NULL, line, col);
            }
            Node *child = parse_expr(p);
            expect(p, TOK_RPAREN);
            return node_make_refuse(child, line, col);
        }
        return node_make_refuse(NULL, line, col);
    }
    if (t->type == TOK_OBSERVE) {
        advance(p);
        expect(p, TOK_LPAREN);
        Node *child = parse_expr(p);
        expect(p, TOK_RPAREN);
        Node *n2 = node_alloc(NODE_OBSERVE, line, col);
        n2->u.child = child;
        return n2;
    }
    if (t->type == TOK_HISTORY) {
        advance(p);
        if (check(p, TOK_LPAREN)) {
            advance(p);
            expect(p, TOK_RPAREN);
        }
        Node *n2 = node_alloc(NODE_HISTORY, line, col);
        n2->u.child = NULL;
        return n2;
    }
    if (t->type == TOK_PRINT) {
        advance(p);
        expect(p, TOK_LPAREN);
        Node *child = parse_expr(p);
        expect(p, TOK_RPAREN);
        Node *n2 = node_alloc(NODE_PRINT, line, col);
        n2->u.child = child;
        return n2;
    }
    if (t->type == TOK_IDENT) {
        char *name = strdup(t->lexeme);
        advance(p);
        /* Function call */
        if (check(p, TOK_LPAREN)) {
            advance(p);
            Node *args[64]; int n_args = 0;
            while (!check(p, TOK_RPAREN) && !check(p, TOK_EOF)) {
                args[n_args++] = parse_expr(p);
                if (!match(p, TOK_COMMA)) break;
            }
            expect(p, TOK_RPAREN);
            Node *nd = node_alloc(NODE_FN_CALL, line, col);
            nd->u.fn_call.name   = name;
            nd->u.fn_call.args   = malloc(n_args * sizeof(Node *));
            nd->u.fn_call.n_args = n_args;
            memcpy(nd->u.fn_call.args, args, n_args * sizeof(Node *));
            return nd;
        }
        Node *nd = node_make_ident(name, line, col);
        free(name);
        /* Field access */
        while (check(p, TOK_DOT)) {
            advance(p);
            Token *field_tok = p->current;
            char *field = strdup(field_tok->lexeme);
            advance(p);
            Node *fa = node_alloc(NODE_FIELD_ACCESS, line, col);
            fa->u.field_access.obj   = nd;
            fa->u.field_access.field = field;
            nd = fa;
        }
        return nd;
    }

    fprintf(stderr, "line %d: unexpected token '%s'\n",
            line, t->lexeme ? t->lexeme : "?");
    p->error_count++;
    advance(p);
    return node_make_int(0, line, col);
}

/* node_alloc exposed for parser internal use */
static Node *parse_unary(Parser *p) {
    int line = p->current->line, col = p->current->col;
    if (check(p, TOK_MINUS)) {
        advance(p);
        Node *operand = parse_unary(p);
        return node_make_unop("-", operand, line, col);
    }
    if (check(p, TOK_NOT)) {
        advance(p);
        Node *operand = parse_unary(p);
        return node_make_unop("!", operand, line, col);
    }
    return parse_primary(p);
}

static Node *parse_power(Parser *p) {
    Node *base = parse_unary(p);
    if (check(p, TOK_CARET)) {
        int line = p->current->line, col = p->current->col;
        advance(p);
        Node *exp = parse_power(p);
        return node_make_binop("^", base, exp, line, col);
    }
    return base;
}

static Node *parse_mul(Parser *p) {
    Node *left = parse_power(p);
    while (check(p, TOK_STAR) || check(p, TOK_SLASH) || check(p, TOK_PERCENT)) {
        int line = p->current->line, col = p->current->col;
        char op[3]; strncpy(op, p->current->lexeme, 2); op[2] = '\0';
        advance(p);
        Node *right = parse_power(p);
        left = node_make_binop(op, left, right, line, col);
    }
    return left;
}

static Node *parse_add(Parser *p) {
    Node *left = parse_mul(p);
    while (check(p, TOK_PLUS) || check(p, TOK_MINUS)) {
        int line = p->current->line, col = p->current->col;
        char op[2] = {p->current->lexeme[0], 0};
        advance(p);
        Node *right = parse_mul(p);
        left = node_make_binop(op, left, right, line, col);
    }
    return left;
}

static Node *parse_compare(Parser *p) {
    Node *left = parse_add(p);
    while (check(p, TOK_LT) || check(p, TOK_GT) ||
           check(p, TOK_LEQ) || check(p, TOK_GEQ)) {
        int line = p->current->line, col = p->current->col;
        char op[3]; strncpy(op, p->current->lexeme, 2); op[2] = '\0';
        advance(p);
        Node *right = parse_add(p);
        left = node_make_binop(op, left, right, line, col);
    }
    return left;
}

static Node *parse_equality(Parser *p) {
    Node *left = parse_compare(p);
    while (check(p, TOK_EQ) || check(p, TOK_NEQ)) {
        int line = p->current->line, col = p->current->col;
        char op[3]; strncpy(op, p->current->lexeme, 2); op[2] = '\0';
        advance(p);
        Node *right = parse_compare(p);
        left = node_make_binop(op, left, right, line, col);
    }
    return left;
}

static Node *parse_and(Parser *p) {
    Node *left = parse_equality(p);
    while (check(p, TOK_AND)) {
        int line = p->current->line, col = p->current->col;
        advance(p);
        Node *right = parse_equality(p);
        left = node_make_binop("&&", left, right, line, col);
    }
    return left;
}

static Node *parse_or(Parser *p) {
    Node *left = parse_and(p);
    while (check(p, TOK_OR)) {
        int line = p->current->line, col = p->current->col;
        advance(p);
        Node *right = parse_and(p);
        left = node_make_binop("||", left, right, line, col);
    }
    return left;
}

Node *parse_expr(Parser *p) { return parse_or(p); }

Node *parse_stmt(Parser *p) {
    int line = p->current->line, col = p->current->col;

    if (check(p, TOK_LET)) {
        advance(p);
        int is_mut = match(p, TOK_MUT);
        Token *name_tok = expect(p, TOK_IDENT);
        char *name = name_tok ? strdup(name_tok->lexeme) : strdup("?");
        free(name_tok);
        Node *init = NULL;
        if (match(p, TOK_ASSIGN)) init = parse_expr(p);
        match(p, TOK_SEMI);
        Node *n = node_make_let(name, init, is_mut, line, col);
        free(name);
        return n;
    }

    if (check(p, TOK_IF)) {
        advance(p);
        expect(p, TOK_LPAREN);
        Node *cond = parse_expr(p);
        expect(p, TOK_RPAREN);
        Node *then_b = parse_stmt(p);
        Node *else_b = NULL;
        if (match(p, TOK_ELSE)) else_b = parse_stmt(p);
        Node *n = node_alloc(NODE_IF, line, col);
        n->u.ifnode.cond        = cond;
        n->u.ifnode.then_branch = then_b;
        n->u.ifnode.else_branch = else_b;
        return n;
    }

    if (check(p, TOK_WHILE)) {
        advance(p);
        expect(p, TOK_LPAREN);
        Node *cond = parse_expr(p);
        expect(p, TOK_RPAREN);
        Node *body = parse_stmt(p);
        Node *n = node_alloc(NODE_WHILE, line, col);
        n->u.whilenode.cond = cond;
        n->u.whilenode.body = body;
        return n;
    }

    if (check(p, TOK_FN)) {
        advance(p);
        Token *name_tok = expect(p, TOK_IDENT);
        char *fn_name = name_tok ? strdup(name_tok->lexeme) : strdup("?");
        free(name_tok);
        expect(p, TOK_LPAREN);
        char *params[32]; int n_params = 0;
        while (!check(p, TOK_RPAREN) && !check(p, TOK_EOF)) {
            Token *pt = expect(p, TOK_IDENT);
            if (pt) { params[n_params++] = strdup(pt->lexeme); free(pt); }
            if (!match(p, TOK_COMMA)) break;
        }
        expect(p, TOK_RPAREN);
        expect(p, TOK_LBRACE);
        Node *stmts[256]; int n_s = 0;
        while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF))
            stmts[n_s++] = parse_stmt(p);
        expect(p, TOK_RBRACE);
        Node *body = node_make_block(stmts, n_s, line, col);
        Node *n = node_alloc(NODE_FN_DEF, line, col);
        n->u.fn_def.name     = fn_name;
        n->u.fn_def.params   = malloc(n_params * sizeof(char *));
        n->u.fn_def.n_params = n_params;
        memcpy(n->u.fn_def.params, params, n_params * sizeof(char *));
        n->u.fn_def.body     = body;
        return n;
    }

    if (check(p, TOK_RETURN)) {
        advance(p);
        Node *val = !check(p, TOK_SEMI) ? parse_expr(p) : node_alloc_null(line, col);
        match(p, TOK_SEMI);
        Node *n = node_alloc(NODE_RETURN, line, col);
        n->u.child = val;
        return n;
    }

    if (check(p, TOK_LBRACE)) {
        advance(p);
        Node *stmts[256]; int n = 0;
        while (!check(p, TOK_RBRACE) && !check(p, TOK_EOF))
            stmts[n++] = parse_stmt(p);
        expect(p, TOK_RBRACE);
        return node_make_block(stmts, n, line, col);
    }

    /* Assignment: ident = expr */
    if (check(p, TOK_IDENT) && p->peek->type == TOK_ASSIGN) {
        char *name = strdup(p->current->lexeme);
        advance(p); advance(p);
        Node *val = parse_expr(p);
        match(p, TOK_SEMI);
        Node *n = node_alloc(NODE_ASSIGN, line, col);
        n->u.assign.name  = name;
        n->u.assign.value = val;
        return n;
    }

    Node *expr = parse_expr(p);
    match(p, TOK_SEMI);
    return expr;
}

Node *parser_parse(Parser *p) {
    Node *stmts[1024]; int n = 0;
    while (!check(p, TOK_EOF) && n < 1024)
        stmts[n++] = parse_stmt(p);
    return node_make_block(stmts, n, 1, 1);
}

Node *parser_parse_expr(Parser *p) { return parse_expr(p); }
Node *parser_parse_stmt(Parser *p) { return parse_stmt(p); }
int   parser_had_error(const Parser *p) { return p->error_count > 0; }
