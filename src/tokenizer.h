/* tokenizer.h - BASIC Tokenizer */
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "tokens.h"

/* Token structure */
typedef struct {
    unsigned char type;      /* Token type/ID */
    char *text;              /* Original text (for identifiers/strings) */
    double value;            /* Numeric value (for numbers) */
    int line;                /* Source line number */
    int column;              /* Source column number */
} Token;

/* Tokenizer state */
typedef struct {
    const char *input_start;  /* Start of input (for error reporting) */
    const char *input;        /* Current input position */
    const char *line_start;   /* Start of current line */
    int line_num;             /* Current line number */
    Token current;            /* Current token */
    Token lookahead;          /* Next token (for 1-token lookahead) */
} Tokenizer;

/* Tokenizer functions */
void tokenizer_init(Tokenizer *tok, const char *input);
void tokenizer_free(Tokenizer *tok);
void tokenizer_refresh(Tokenizer *tok);  /* Refresh tokens after state restore */
Token* tokenizer_next(Tokenizer *tok);
Token* tokenizer_peek(Tokenizer *tok);
const char* token_name(unsigned char type);

#endif /* TOKENIZER_H */
