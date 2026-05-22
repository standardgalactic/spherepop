/* ast.h — Abstract syntax tree for Spherepop.
 *
 * AST nodes represent the unevaluated expression structure before
 * bubble allocation. The evaluator converts them into live Bubble trees.
 * Parentheses in source become Bubble nodes (not syntactic markers).
 */
#ifndef SP_AST_H
#define SP_AST_H

#include <stdint.h>

typedef enum {
    NODE_INT_LIT,
    NODE_FLOAT_LIT,
    NODE_STRING_LIT,
    NODE_BOOL_LIT,
    NODE_NULL_LIT,
    NODE_IDENT,
    NODE_BINOP,
    NODE_UNOP,
    NODE_BUBBLE,      /* bubble { ... } — explicit nested admissibility region */
    NODE_POP,         /* pop(expr) */
    NODE_REFUSE,      /* refuse(expr) */
    NODE_COLLAPSE,    /* collapse(expr, level) */
    NODE_BIND,        /* bind(bubble, constraints) */
    NODE_LET,         /* let x = expr */
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_BLOCK,
    NODE_FN_DEF,
    NODE_FN_CALL,
    NODE_FIELD_ACCESS, /* b.name */
    NODE_RETURN,
    NODE_OBSERVE,     /* observe(bubble) — first-class observer */
    NODE_HISTORY,     /* history(bubble) — retrieve history object */
    NODE_TRAJECTORY,  /* trajectory(bubble) — retrieve trajectory */
    NODE_PRINT,
} NodeKind;

typedef struct Node {
    NodeKind kind;
    int      line;
    int      col;
    union {
        long long  ival;
        double     fval;
        char      *sval;
        int        bval;
        struct {
            char     *op;
            struct Node *left;
            struct Node *right;
        } binop;
        struct {
            char     *op;
            struct Node *operand;
        } unop;
        struct {
            struct Node **stmts;
            int          n;
        } block;
        struct {
            char       *name;
            struct Node *init;
            int         mutable;
        } let;
        struct {
            struct Node *cond;
            struct Node *then_branch;
            struct Node *else_branch;
        } ifnode;
        struct {
            struct Node *cond;
            struct Node *body;
        } whilenode;
        struct {
            char        *name;
            char       **params;
            int          n_params;
            struct Node *body;
        } fn_def;
        struct {
            char        *name;
            struct Node **args;
            int          n_args;
        } fn_call;
        struct {
            struct Node *obj;
            char        *field;
        } field_access;
        struct {
            char        *name;
            struct Node *value;
        } assign;
        struct Node *child;  /* pop, refuse, return, observe, etc. */
        struct {
            char        *level;  /* collapse level string */
            struct Node *target;
        } collapse;
        struct {
            struct Node *target;
            struct Node *constraints;
        } bind;
    } u;
} Node;

Node *node_make_int(long long val, int line, int col);
Node *node_make_float(double val, int line, int col);
Node *node_make_string(const char *s, int line, int col);
Node *node_make_bool(int val, int line, int col);
Node *node_make_ident(const char *name, int line, int col);
Node *node_make_binop(const char *op, Node *l, Node *r, int line, int col);
Node *node_make_unop(const char *op, Node *operand, int line, int col);
Node *node_make_bubble(Node **stmts, int n, int line, int col);
Node *node_make_pop(Node *child, int line, int col);
Node *node_make_refuse(Node *child, int line, int col);
Node *node_make_let(const char *name, Node *init, int mutable, int line, int col);
Node *node_make_block(Node **stmts, int n, int line, int col);
void  node_free(Node *n);
void  node_print(const Node *n, int indent);

Node *node_alloc(NodeKind kind, int line, int col);

#endif /* SP_AST_H */

/* Internal: allocate a node of given kind. Used by parser. */
