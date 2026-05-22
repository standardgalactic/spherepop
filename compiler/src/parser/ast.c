#define _POSIX_C_SOURCE 200809L
/* ast.c — AST node construction and printing. */
#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Node *node_alloc(NodeKind kind, int line, int col) {
    Node *n = calloc(1, sizeof(Node));
    n->kind = kind;
    n->line = line;
    n->col  = col;
    return n;
}

Node *node_make_int(long long val, int line, int col) {
    Node *n = node_alloc(NODE_INT_LIT, line, col);
    n->u.ival = val;
    return n;
}
Node *node_make_float(double val, int line, int col) {
    Node *n = node_alloc(NODE_FLOAT_LIT, line, col);
    n->u.fval = val;
    return n;
}
Node *node_make_string(const char *s, int line, int col) {
    Node *n = node_alloc(NODE_STRING_LIT, line, col);
    n->u.sval = strdup(s);
    return n;
}
Node *node_make_bool(int val, int line, int col) {
    Node *n = node_alloc(NODE_BOOL_LIT, line, col);
    n->u.bval = val;
    return n;
}
Node *node_make_ident(const char *name, int line, int col) {
    Node *n = node_alloc(NODE_IDENT, line, col);
    n->u.sval = strdup(name);
    return n;
}
Node *node_make_binop(const char *op, Node *l, Node *r, int line, int col) {
    Node *n = node_alloc(NODE_BINOP, line, col);
    n->u.binop.op    = strdup(op);
    n->u.binop.left  = l;
    n->u.binop.right = r;
    return n;
}
Node *node_make_unop(const char *op, Node *operand, int line, int col) {
    Node *n = node_alloc(NODE_UNOP, line, col);
    n->u.unop.op      = strdup(op);
    n->u.unop.operand = operand;
    return n;
}
Node *node_make_bubble(Node **stmts, int cnt, int line, int col) {
    Node *n = node_alloc(NODE_BUBBLE, line, col);
    n->u.block.stmts = malloc(cnt * sizeof(Node *));
    memcpy(n->u.block.stmts, stmts, cnt * sizeof(Node *));
    n->u.block.n = cnt;
    return n;
}
Node *node_make_pop(Node *child, int line, int col) {
    Node *n = node_alloc(NODE_POP, line, col);
    n->u.child = child;
    return n;
}
Node *node_make_refuse(Node *child, int line, int col) {
    Node *n = node_alloc(NODE_REFUSE, line, col);
    n->u.child = child;
    return n;
}
Node *node_make_let(const char *name, Node *init, int mutable, int line, int col) {
    Node *n = node_alloc(NODE_LET, line, col);
    n->u.let.name    = strdup(name);
    n->u.let.init    = init;
    n->u.let.mutable = mutable;
    return n;
}
Node *node_make_block(Node **stmts, int cnt, int line, int col) {
    Node *n = node_alloc(NODE_BLOCK, line, col);
    n->u.block.stmts = malloc(cnt * sizeof(Node *));
    memcpy(n->u.block.stmts, stmts, cnt * sizeof(Node *));
    n->u.block.n = cnt;
    return n;
}

static const char *indent_str(int indent) {
    static char buf[128];
    int n = indent * 2;
    if (n >= 128) n = 126;
    memset(buf, ' ', n);
    buf[n] = '\0';
    return buf;
}

void node_print(const Node *n, int indent) {
    if (!n) return;
    const char *pad = indent_str(indent);
    switch (n->kind) {
        case NODE_INT_LIT:   printf("%sInt(%lld)\n", pad, n->u.ival); break;
        case NODE_FLOAT_LIT: printf("%sFloat(%g)\n", pad, n->u.fval); break;
        case NODE_STRING_LIT:printf("%sString(%s)\n",pad, n->u.sval); break;
        case NODE_BOOL_LIT:  printf("%sBool(%s)\n",  pad, n->u.bval?"true":"false"); break;
        case NODE_NULL_LIT:  printf("%sNull\n",      pad); break;
        case NODE_IDENT:     printf("%sIdent(%s)\n", pad, n->u.sval); break;
        case NODE_BINOP:
            printf("%sBinop(%s)\n", pad, n->u.binop.op);
            node_print(n->u.binop.left,  indent+1);
            node_print(n->u.binop.right, indent+1);
            break;
        case NODE_UNOP:
            printf("%sUnop(%s)\n", pad, n->u.unop.op);
            node_print(n->u.unop.operand, indent+1);
            break;
        case NODE_LET:
            printf("%sLet(%s%s)\n", pad, n->u.let.mutable?"mut ":"", n->u.let.name);
            node_print(n->u.let.init, indent+1);
            break;
        case NODE_BUBBLE:
            printf("%sBubble(%d stmts)\n", pad, n->u.block.n);
            for (int i = 0; i < n->u.block.n; i++)
                node_print(n->u.block.stmts[i], indent+1);
            break;
        case NODE_BLOCK:
            printf("%sBlock(%d)\n", pad, n->u.block.n);
            for (int i = 0; i < n->u.block.n; i++)
                node_print(n->u.block.stmts[i], indent+1);
            break;
        case NODE_POP:
            printf("%sPop\n", pad);
            node_print(n->u.child, indent+1);
            break;
        case NODE_PRINT:
            printf("%sPrint\n", pad);
            node_print(n->u.child, indent+1);
            break;
        case NODE_FN_CALL:
            printf("%sCall(%s, %d args)\n", pad, n->u.fn_call.name, n->u.fn_call.n_args);
            for (int i = 0; i < n->u.fn_call.n_args; i++)
                node_print(n->u.fn_call.args[i], indent+1);
            break;
        case NODE_IF:
            printf("%sIf\n", pad);
            node_print(n->u.ifnode.cond, indent+1);
            node_print(n->u.ifnode.then_branch, indent+1);
            if (n->u.ifnode.else_branch) node_print(n->u.ifnode.else_branch, indent+1);
            break;
        default:
            printf("%sNode(kind=%d)\n", pad, n->kind);
    }
}

void node_free(Node *n) {
    if (!n) return;
    switch (n->kind) {
        case NODE_STRING_LIT:
        case NODE_IDENT:      free(n->u.sval); break;
        case NODE_BINOP:
            free(n->u.binop.op);
            node_free(n->u.binop.left);
            node_free(n->u.binop.right);
            break;
        case NODE_UNOP:
            free(n->u.unop.op);
            node_free(n->u.unop.operand);
            break;
        case NODE_LET:
            free(n->u.let.name);
            node_free(n->u.let.init);
            break;
        case NODE_BUBBLE:
        case NODE_BLOCK:
            for (int i = 0; i < n->u.block.n; i++)
                node_free(n->u.block.stmts[i]);
            free(n->u.block.stmts);
            break;
        case NODE_POP: case NODE_REFUSE: case NODE_PRINT:
        case NODE_RETURN: case NODE_OBSERVE: case NODE_HISTORY:
            node_free(n->u.child); break;
        case NODE_FN_CALL:
            for (int i = 0; i < n->u.fn_call.n_args; i++)
                node_free(n->u.fn_call.args[i]);
            free(n->u.fn_call.args);
            break;
        case NODE_IF:
            node_free(n->u.ifnode.cond);
            node_free(n->u.ifnode.then_branch);
            node_free(n->u.ifnode.else_branch);
            break;
        case NODE_WHILE:
            node_free(n->u.whilenode.cond);
            node_free(n->u.whilenode.body);
            break;
        case NODE_FN_DEF:
            for (int i = 0; i < n->u.fn_def.n_params; i++)
                free(n->u.fn_def.params[i]);
            free(n->u.fn_def.params);
            node_free(n->u.fn_def.body);
            break;
        default: break;
    }
    free(n);
}
