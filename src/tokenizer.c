/* tokenizer.c - Classic BASIC Tokenizer Implementation */
#include "tokenizer.h"
#include "syntax_tables.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Helper: skip whitespace (but not newlines in BASIC) */
static void skip_whitespace(Tokenizer *tok) {
    while (*tok->input == ' ' || *tok->input == '\t') {
        tok->input++;
    }
}

/* Helper: check if character can start an identifier */
static int is_ident_start(char c) {
    return isalpha(c);
}

/* Helper: check if character can continue an identifier */
static int is_ident_cont(char c) {
    return isalnum(c) || c == '_';
}

/* Helper: match keyword in table */
static int match_keyword(const char *text, int len, unsigned char *token) {
    int i;
    char upper[64];
    int j;
    
    /* Convert to uppercase for comparison */
    if (len >= 64) return 0;
    
    for (j = 0; j < len; j++) {
        upper[j] = toupper(text[j]);
    }
    upper[len] = '\0';
    
    /* Search keyword table */
    for (i = 0; i < keyword_table_size; i++) {
        if (strcmp(upper, keyword_table[i].keyword) == 0) {
            *token = keyword_table[i].token;
            return 1;
        }
    }
    
    return 0;
}

/* Read next token from input */
static void read_token(Tokenizer *tok, Token *t) {
    const char *start;
    int len;
    unsigned char tok_type;
    
    skip_whitespace(tok);
    
    /* Initialize token */
    t->line = tok->line_num;
    t->column = tok->input - tok->line_start;
    t->text = NULL;
    t->value = 0.0;
    
    /* End of input */
    if (*tok->input == '\0') {
        t->type = TOK_EOF;
        return;
    }
    
    /* Newline - treat as end of statement */
    if (*tok->input == '\n') {
        t->type = TOK_CCR;
        tok->input++;
        tok->line_num++;
        tok->line_start = tok->input;
        return;
    }
    
    /* Apostrophe - alias for REM statement (classic BASIC feature) */
    if (*tok->input == '\'') {
        t->type = TOK_REM;
        /* Store "REM" as text for consistency */
        t->text = malloc(4);
        strcpy(t->text, "REM");
        tok->input++; /* Skip the apostrophe */
        /* Skip everything until end of line (REM comment behavior) */
        while (*tok->input && *tok->input != '\n') {
            tok->input++;
        }
        return;
    }
    
    /* String literal - multi-line strings supported */
    if (*tok->input == '"') {
        start = ++tok->input;
        while (*tok->input && *tok->input != '"') {
            tok->input++;
        }
        len = tok->input - start;
        t->type = TOK_STRING;
        t->text = malloc(len + 1);
        memcpy(t->text, start, len);
        t->text[len] = '\0';
        if (*tok->input == '"') tok->input++;
        return;
    }
    
    /* Number */
    if (isdigit(*tok->input) || (*tok->input == '.' && isdigit(tok->input[1]))) {
        char *end;
        t->type = TOK_NUMBER;
        t->value = strtod(tok->input, &end);
        tok->input = end;
        return;
    }
    
    /* Multi-character operators */
    if (tok->input[0] == '<' && tok->input[1] == '=') {
        t->type = TOK_CLE;
        tok->input += 2;
        return;
    }
    if (tok->input[0] == '<' && tok->input[1] == '>') {
        t->type = TOK_CNE;
        tok->input += 2;
        return;
    }
    if (tok->input[0] == '>' && tok->input[1] == '=') {
        t->type = TOK_CGE;
        tok->input += 2;
        return;
    }
    
    /* Single-character operators and punctuation */
    switch (*tok->input) {
        case ',': t->type = TOK_CCOM; tok->input++; return;
        case ':': t->type = TOK_CEOS; tok->input++; return;
        case ';': t->type = TOK_CSC; tok->input++; return;
        case '#': t->type = TOK_CPND; tok->input++; return;
        case '<': t->type = TOK_CLT; tok->input++; return;
        case '>': t->type = TOK_CGT; tok->input++; return;
        case '=': t->type = TOK_CEQ; tok->input++; return;
        case '^': t->type = TOK_CEXP; tok->input++; return;
        case '*': t->type = TOK_CMUL; tok->input++; return;
        case '+': t->type = TOK_CPLUS; tok->input++; return;
        case '-': t->type = TOK_CMINUS; tok->input++; return;
        case '/': t->type = TOK_CDIV; tok->input++; return;
        case '(': t->type = TOK_CLPRN; tok->input++; return;
        case ')': t->type = TOK_CRPRN; tok->input++; return;
        case '$': t->type = TOK_CDOL; tok->input++; return;
        case '?': t->type = TOK_QUESTION; tok->input++; return;
    }
    
    /* Identifier or keyword */
    if (is_ident_start(*tok->input)) {
        start = tok->input;
        
        /* Try to match keywords of varying lengths (longest match wins) */
        /* Microsoft BASIC allows keywords directly followed by identifiers/numbers */
        /* e.g., CLEAR5400, DEFINTA-Z, GOTO100, IFFPRINT (IF F PRINT) */
        {
            int best_len = 0;
            unsigned char best_tok = 0;
            int max_try_len = 16;
            
            /* Calculate maximum reasonable length to try */
            for (len = 0; len < max_try_len && start[len] != '\0'; len++) {
                if (!is_ident_cont(start[len]) && start[len] != '$') {
                    max_try_len = len + 1;
                    break;
                }
            }
            
            for (len = 1; len < max_try_len; len++) {
                if (match_keyword(start, len, &tok_type)) {
                    /* Found a keyword match of length 'len' */
                    /* Check what follows */
                    char next_ch = start[len];
                    
                    if (next_ch == '\0' || !isalpha(next_ch)) {
                        /* End of input or followed by non-alpha (digit, symbol, whitespace, etc) */
                        /* This is a clean keyword boundary - accept immediately */
                        best_len = len;
                        best_tok = tok_type;
                        break;
                    }
                    
                    /* Followed by alpha character - might be start of identifier */
                    /* Remember this match and keep looking for longer keywords */
                    if (best_len == 0 || len > best_len) {
                        best_len = len;
                        best_tok = tok_type;
                    }
                }
            }
            
            if (best_len > 0) {
                /* Found a keyword */
                tok->input = start + best_len;
                t->type = best_tok;
                t->text = malloc(best_len + 1);
                if (t->text) {
                    memcpy(t->text, start, best_len);
                    t->text[best_len] = '\0';
                }
                
                /* Special handling for REM - skip rest of line */
                if (best_tok == TOK_REM) {
                    while (*tok->input && *tok->input != '\n') {
                        tok->input++;
                    }
                }
                
                return;
            }
        }
        
        /* No keyword matched - scan the full identifier */
        tok->input = start;
        
        /* Microsoft BASIC allows keywords to appear immediately after */
        /* variable names without spaces, e.g. IFFPRINT → IF F PRINT */
        /* SFANDCINT should parse as SF AND CINT */
        /* Strategy: Consume identifier characters one at a time, and after each character, */
        /* check if what follows could be a keyword. If so, stop the identifier there. */
        {
            int best_keyword_len;
            
            while (is_ident_cont(*tok->input)) {
                /* Consume this character as part of identifier */
                tok->input++;
                
                /* After consuming this character, check if a keyword starts at current position */
                /* We need to find if ANY keyword matches here */
                /* Try all possible lengths and remember if we found any keyword */
                best_keyword_len = 0;
            
            for (len = 1; len <= 16 && tok->input[len-1] != '\0'; len++) {
                unsigned char test_tok;
                if (match_keyword(tok->input, len, &test_tok)) {
                    /* Found a keyword match - remember the longest one */
                    best_keyword_len = len;
                }
            }
            
                if (best_keyword_len > 0) {
                    /* Found at least one keyword match at current position */
                    /* Check if the best match is followed by alpha */
                    char after = tok->input[best_keyword_len];
                    if (after == '\0' || !isalpha(after)) {
                        /* Clean boundary - definitely stop here */
                        break;
                    } else {
                        /* Followed by alpha, but we found a keyword, so stop anyway */
                        /* This handles cases like ANDCINT -> AND + CINT */
                        break;
                    }
                }
            }
        }
        
        /* Check for string variable suffix ($) */
        if (*tok->input == '$') {
            tok->input++;
        }
        
        len = tok->input - start;
        
        /* It's a variable identifier */
        t->type = TOK_IDENT;
        t->text = malloc(len + 1);
        memcpy(t->text, start, len);
        t->text[len] = '\0';
        return;
    }
    
    /* Unknown character - skip it */
    fprintf(stderr, "Unknown character: '%c' (0x%02X)\n", *tok->input, (unsigned char)*tok->input);
    tok->input++;
    t->type = TOK_EOF;
}

/* Initialize tokenizer */
void tokenizer_init(Tokenizer *tok, const char *input) {
    tok->input_start = input;
    tok->input = input;
    tok->line_start = input;
    tok->line_num = 1;
    
    /* Initialize token text pointers to NULL before reading */
    tok->current.text = NULL;
    tok->lookahead.text = NULL;
    
    /* Read first two tokens for lookahead */
    read_token(tok, &tok->current);
    read_token(tok, &tok->lookahead);
}

/* Refresh tokens after state restore (for backtracking) */
void tokenizer_refresh(Tokenizer *tok) {
    /* Free existing token text to prevent leaks */
    if (tok->current.text) {
        free(tok->current.text);
        tok->current.text = NULL;
    }
    if (tok->lookahead.text) {
        free(tok->lookahead.text);
        tok->lookahead.text = NULL;
    }
    
    /* Re-read tokens from current position */
    read_token(tok, &tok->current);
    read_token(tok, &tok->lookahead);
}

/* Free tokenizer resources */
void tokenizer_free(Tokenizer *tok) {
    if (tok->current.text) {
        free(tok->current.text);
        tok->current.text = NULL;
    }
    if (tok->lookahead.text) {
        free(tok->lookahead.text);
        tok->lookahead.text = NULL;
    }
}

/* Get next token and advance */
Token* tokenizer_next(Tokenizer *tok) {
    /* Don't free old current text - will be freed in tokenizer_free() */
    /* This prevents issues with backtracking that saves/restores token pointers */
    
    /* Transfer lookahead to current (ownership transfer of text pointer) */
    tok->current = tok->lookahead;
    
    /* Clear lookahead text pointer since ownership transferred to current */
    tok->lookahead.text = NULL;
    
    /* Read new lookahead */
    read_token(tok, &tok->lookahead);
    
    return &tok->current;
}

/* Peek at current token without advancing */
Token* tokenizer_peek(Tokenizer *tok) {
    return &tok->current;
}

/* Get printable name for token type */
const char* token_name(unsigned char type) {
    static char buf[32];
    int i;
    
    /* Check keyword table */
    for (i = 0; i < keyword_table_size; i++) {
        if (keyword_table[i].token == type) {
            return keyword_table[i].keyword;
        }
    }
    
    /* Check special types */
    switch (type) {
        case TOK_NUMBER: return "NUMBER";
        case TOK_STRING: return "STRING";
        case TOK_IDENT: return "IDENTIFIER";
        case TOK_EOF: return "EOF";
        case TOK_CCOM: return ",";
        case TOK_CEOS: return ":";
        case TOK_CSC: return ";";
        case TOK_CCR: return "CR";
        case TOK_CPND: return "#";
        case TOK_CLE: return "<=";
        case TOK_CNE: return "<>";
        case TOK_CGE: return ">=";
        case TOK_CLT: return "<";
        case TOK_CGT: return ">";
        case TOK_CEQ: return "=";
        case TOK_CEXP: return "^";
        case TOK_CMUL: return "*";
        case TOK_CPLUS: return "+";
        case TOK_CMINUS: return "-";
        case TOK_CDIV: return "/";
        case TOK_CLPRN: return "(";
        case TOK_CRPRN: return ")";
        default:
            sprintf(buf, "TOK_%02X", type);
            return buf;
    }
}
