/* test_programs.c — Integration tests: parse and evaluate programs. */
#include <stdio.h>
#include <string.h>
#include "../../src/lexer/lexer.h"
#include "../../src/parser/parser.h"
#include "../../src/parser/ast.h"
#include "../../src/runtime/bubble.h"
#include "../../src/runtime/scope.h"
#include "../../src/runtime/evaluator.h"
#include "../../src/runtime/value.h"

static int tests_run = 0, tests_passed = 0;

static Value *run(const char *src) {
    Lexer  *l   = lexer_create(src);
    Parser *p   = parser_create(l);
    Node   *ast = parser_parse(p);
    if (parser_had_error(p)) { parser_free(p); lexer_free(l); return val_error("parse error"); }
    EvalContext *ctx  = eval_context_create(false);
    Scope       *s    = scope_create(NULL);
    Bubble      *root = bubble_alloc(NULL, s);
    Value *result = eval_node(ast, root, ctx);
    bubble_free(root);
    scope_free(s);
    eval_context_free(ctx);
    node_free(ast);
    parser_free(p);
    lexer_free(l);
    return result;
}

#define TINT(name, src, expected) do { \
    tests_run++; \
    Value *v = run(src); \
    int ok = v && v->kind == VAL_INT && v->u.ival == (expected); \
    if (ok) { tests_passed++; printf("PASS: %s\n", name); } \
    else { printf("FAIL: %s (got kind=%d)\n", name, v ? v->kind : -1); } \
    if(v) val_release(v); \
} while(0)

int main(void) {
    printf("=== Integration Tests ===\n");

    TINT("arithmetic 1+3*4", "1 + 3 * 4", 13);
    TINT("power 2^10", "2 ^ 10", 1024);
    TINT("let binding", "let x = 7; x", 7);
    TINT("nested bubble", "bubble { bubble { 6 * 7 } }", 42);
    TINT("if true", "if 1 { 99 } else { 0 }", 99);
    TINT("if false", "if 0 { 99 } else { 42 }", 42);
    TINT("factorial 5", "fn f(n) { if n <= 1 { return 1; } return n * f(n-1); } f(5)", 120);

    printf("\n%d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
