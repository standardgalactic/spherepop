/* parser.h — Recursive descent parser for Spherepop. */
#ifndef SP_PARSER_H
#define SP_PARSER_H

#include "../lexer/lexer.h"
#include "ast.h"

typedef struct Parser {
    Lexer  *lexer;
    Token  *current;
    Token  *peek;
    int     error_count;
    char   *last_error;
} Parser;

Parser *parser_create(Lexer *lexer);
void    parser_free(Parser *p);
Node   *parser_parse(Parser *p);         /* Parse full program */
Node   *parser_parse_expr(Parser *p);    /* Parse expression */
Node   *parser_parse_stmt(Parser *p);    /* Parse statement */
int     parser_had_error(const Parser *p);

#endif /* SP_PARSER_H */
