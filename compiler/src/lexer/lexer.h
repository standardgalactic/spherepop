/* lexer.h — Spherepop lexer. */
#ifndef SP_LEXER_H
#define SP_LEXER_H

#include "token.h"

typedef struct Lexer {
    const char *src;
    int         pos;
    int         line;
    int         col;
    int         src_len;
} Lexer;

Lexer *lexer_create(const char *source);
void   lexer_free(Lexer *l);
Token *lexer_next(Lexer *l);
Token *lexer_peek(Lexer *l);
void   lexer_tokenize_all(Lexer *l, Token **out, int *count);

#endif /* SP_LEXER_H */
