/* test_lexer.c — Lexer unit tests. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../src/lexer/lexer.h"
#include "../../src/lexer/token.h"

static int tests_run = 0, tests_passed = 0;
#define TEST(name, expr) do { \
    tests_run++; \
    if (expr) { tests_passed++; printf("PASS: %s\n", name); } \
    else { printf("FAIL: %s (line %d)\n", name, __LINE__); } \
} while(0)

static Token *nt(Lexer *l) { return lexer_next(l); }

int main(void) {
    printf("=== Lexer Tests ===\n");

    Lexer *l = lexer_create("let x = 42;");
    Token *t;
    t = nt(l); TEST("let",      t->type == TOK_LET);   free(t);
    t = nt(l); TEST("ident x",  t->type == TOK_IDENT && strcmp(t->lexeme,"x")==0); free(t);
    t = nt(l); TEST("assign",   t->type == TOK_ASSIGN); free(t);
    t = nt(l); TEST("int 42",   t->type == TOK_INT && t->literal.ival == 42); free(t);
    t = nt(l); TEST("semi",     t->type == TOK_SEMI);   free(t);
    t = nt(l); TEST("eof",      t->type == TOK_EOF);    free(t);
    lexer_free(l);

    l = lexer_create("bubble pop refuse collapse bind observe history");
    t = nt(l); TEST("bubble kw",   t->type == TOK_BUBBLE);   free(t);
    t = nt(l); TEST("pop kw",      t->type == TOK_POP);      free(t);
    t = nt(l); TEST("refuse kw",   t->type == TOK_REFUSE);   free(t);
    t = nt(l); TEST("collapse kw", t->type == TOK_COLLAPSE); free(t);
    t = nt(l); TEST("bind kw",     t->type == TOK_BIND);     free(t);
    t = nt(l); TEST("observe kw",  t->type == TOK_OBSERVE);  free(t);
    t = nt(l); TEST("history kw",  t->type == TOK_HISTORY);  free(t);
    lexer_free(l);

    l = lexer_create("3.14 \"hello\\n\" true false null");
    t = nt(l); TEST("float 3.14",  t->type == TOK_FLOAT);   free(t);
    t = nt(l); TEST("string",      t->type == TOK_STRING);  free(t);
    t = nt(l); TEST("true",        t->type == TOK_BOOL && t->literal.ival == 1); free(t);
    t = nt(l); TEST("false",       t->type == TOK_BOOL && t->literal.ival == 0); free(t);
    t = nt(l); TEST("null",        t->type == TOK_NULL);    free(t);
    lexer_free(l);

    l = lexer_create("== != <= >= && ||");
    t = nt(l); TEST("==", t->type == TOK_EQ);  free(t);
    t = nt(l); TEST("!=", t->type == TOK_NEQ); free(t);
    t = nt(l); TEST("<=", t->type == TOK_LEQ); free(t);
    t = nt(l); TEST(">=", t->type == TOK_GEQ); free(t);
    t = nt(l); TEST("&&", t->type == TOK_AND); free(t);
    t = nt(l); TEST("||", t->type == TOK_OR);  free(t);
    lexer_free(l);

    printf("\n%d/%d passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
