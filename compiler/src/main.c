#define _POSIX_C_SOURCE 200809L
/* main.c — Spherepop interpreter entry point.
 *
 * Usage:
 *   spherepop              — interactive REPL
 *   spherepop file.sp      — execute file
 *   spherepop -e "expr"    — evaluate expression string
 *   spherepop --trace ...  — enable event trace output
 *   spherepop --ast ...    — print AST and exit
 *   spherepop --history    — print global history after execution
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/ast.h"
#include "runtime/evaluator.h"
#include "runtime/bubble.h"
#include "runtime/scope.h"
#include "runtime/value.h"
#include "runtime/history.h"

static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open: %s\n", path); return NULL; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(sz + 1);
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    return buf;
}

static Value *run_source(const char *src, EvalContext *ctx,
                         bool print_ast, bool print_history) {
    Lexer  *lexer  = lexer_create(src);
    Parser *parser = parser_create(lexer);
    Node   *ast    = parser_parse(parser);

    if (parser_had_error(parser)) {
        fprintf(stderr, "Parse errors; aborting.\n");
        node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        return val_error("parse error");
    }

    if (print_ast) {
        node_print(ast, 0);
        node_free(ast);
        parser_free(parser);
        lexer_free(lexer);
        return val_null();
    }

    /* Build root bubble */
    Scope  *root_scope = scope_create(NULL);
    Bubble *root       = bubble_alloc(NULL, root_scope);
    root->expression   = ast;

    Value *result = eval_node(ast, root, ctx);

    if (print_history) {
        printf("\n--- Computation History ---\n");
        history_print(ctx->global_history);
        printf("---\n");
    }

    node_free(ast);
    parser_free(parser);
    lexer_free(lexer);
    scope_free(root_scope);
    bubble_free(root);
    return result;
}

static void repl(EvalContext *ctx) {
    printf("Spherepop REPL — type 'exit' or Ctrl-D to quit.\n");
    printf("Computation is history. History is primary.\n\n");
    char line[4096];
    while (1) {
        printf("sp> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) { printf("\n"); break; }
        /* Trim newline */
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) break;
        if (strlen(line) == 0) continue;
        Value *result = run_source(line, ctx, false, false);
        if (result && result->kind != VAL_NULL) {
            printf("=> ");
            val_print(result);
            printf("\n");
        }
        if (result) val_release(result);
    }
}

int main(int argc, char **argv) {
    bool trace       = false;
    bool print_ast   = false;
    bool print_hist  = false;
    bool do_expr     = false;
    const char *expr_str = NULL;
    const char *file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--trace")   == 0) trace      = true;
        else if (strcmp(argv[i], "--ast")== 0) print_ast  = true;
        else if (strcmp(argv[i], "--history") == 0) print_hist = true;
        else if (strcmp(argv[i], "-e")   == 0 && i+1 < argc) {
            do_expr  = true;
            expr_str = argv[++i];
        } else if (argv[i][0] != '-') {
            file = argv[i];
        }
    }

    EvalContext *ctx = eval_context_create(trace);

    if (do_expr) {
        Value *result = run_source(expr_str, ctx, print_ast, print_hist);
        if (result) { val_print(result); printf("\n"); val_release(result); }
    } else if (file) {
        char *src = read_file(file);
        if (!src) { eval_context_free(ctx); return 1; }
        Value *result = run_source(src, ctx, print_ast, print_hist);
        free(src);
        if (result && result->kind == VAL_ERROR) {
            fprintf(stderr, "Error: %s\n", result->u.error_msg);
            val_release(result);
            eval_context_free(ctx);
            return 1;
        }
        if (result) val_release(result);
    } else {
        repl(ctx);
    }

    eval_context_free(ctx);
    return 0;
}
