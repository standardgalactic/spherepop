/* token.h — Token types for the Spherepop lexer. */
#ifndef SP_TOKEN_H
#define SP_TOKEN_H

typedef enum {
    /* Literals */
    TOK_INT, TOK_FLOAT, TOK_STRING, TOK_BOOL, TOK_NULL,

    /* Identifiers and keywords */
    TOK_IDENT,
    TOK_LET, TOK_MUT,
    TOK_BUBBLE, TOK_POP, TOK_REFUSE, TOK_COLLAPSE, TOK_BIND,
    TOK_IF, TOK_ELSE, TOK_WHILE, TOK_FOR, TOK_RETURN,
    TOK_FN, TOK_IMPORT, TOK_EXPORT,
    TOK_PRINT, TOK_FIELD, TOK_MERGE, TOK_SHELL,
    TOK_OBSERVE, TOK_HISTORY, TOK_TRAJECTORY,

    /* Operators */
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT,
    TOK_CARET, TOK_AMP, TOK_PIPE, TOK_TILDE,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LEQ, TOK_GEQ,
    TOK_AND, TOK_OR, TOK_NOT,
    TOK_ASSIGN, TOK_ARROW, TOK_DOT,

    /* Delimiters */
    TOK_LPAREN, TOK_RPAREN,
    TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_SEMI, TOK_COMMA, TOK_COLON,

    TOK_EOF, TOK_ERROR
} TokenType;

typedef struct Token {
    TokenType   type;
    char       *lexeme;  /* Interned string; do not free individually */
    int         line;
    int         col;
    union {
        long long  ival;
        double     fval;
    } literal;
} Token;

const char *token_type_name(TokenType t);
void        token_print(const Token *t);

#endif /* SP_TOKEN_H */
