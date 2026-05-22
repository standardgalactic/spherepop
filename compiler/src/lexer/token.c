/* token.c — Token utilities. */
#include "token.h"
#include <stdio.h>

const char *token_type_name(TokenType t) {
    switch (t) {
        case TOK_INT: return "INT"; case TOK_FLOAT: return "FLOAT";
        case TOK_STRING: return "STRING"; case TOK_BOOL: return "BOOL";
        case TOK_NULL: return "NULL"; case TOK_IDENT: return "IDENT";
        case TOK_LET: return "let"; case TOK_MUT: return "mut";
        case TOK_BUBBLE: return "bubble"; case TOK_POP: return "pop";
        case TOK_REFUSE: return "refuse"; case TOK_COLLAPSE: return "collapse";
        case TOK_BIND: return "bind"; case TOK_IF: return "if";
        case TOK_ELSE: return "else"; case TOK_WHILE: return "while";
        case TOK_FOR: return "for"; case TOK_RETURN: return "return";
        case TOK_FN: return "fn"; case TOK_PRINT: return "print";
        case TOK_OBSERVE: return "observe"; case TOK_HISTORY: return "history";
        case TOK_PLUS: return "+"; case TOK_MINUS: return "-";
        case TOK_STAR: return "*"; case TOK_SLASH: return "/";
        case TOK_PERCENT: return "%"; case TOK_CARET: return "^";
        case TOK_EQ: return "=="; case TOK_NEQ: return "!=";
        case TOK_LT: return "<"; case TOK_GT: return ">";
        case TOK_LEQ: return "<="; case TOK_GEQ: return ">=";
        case TOK_AND: return "&&"; case TOK_OR: return "||";
        case TOK_NOT: return "!"; case TOK_ASSIGN: return "=";
        case TOK_ARROW: return "->"; case TOK_DOT: return ".";
        case TOK_LPAREN: return "("; case TOK_RPAREN: return ")";
        case TOK_LBRACE: return "{"; case TOK_RBRACE: return "}";
        case TOK_LBRACKET: return "["; case TOK_RBRACKET: return "]";
        case TOK_SEMI: return ";"; case TOK_COMMA: return ",";
        case TOK_COLON: return ":"; case TOK_EOF: return "EOF";
        case TOK_ERROR: return "ERROR"; default: return "?";
    }
}

void token_print(const Token *t) {
    if (!t) { printf("<null token>\n"); return; }
    printf("Token(%s, '%s', line=%d, col=%d)\n",
           token_type_name(t->type), t->lexeme ? t->lexeme : "",
           t->line, t->col);
}
