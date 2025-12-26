/*
 * basset_tokenize.c - Tokenizer debugging utility
 * Reads a BASIC source file and displays all tokens produced by the tokenizer.
 * Useful for debugging lexer issues and verifying token recognition.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "tokens.h"

/* Read entire file into memory */
static char* read_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    char *buffer;
    long length;
    
    if (!f) {
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);
    
    return buffer;
}

int main(int argc, char *argv[]) {
    Tokenizer tokenizer;
    Token *token;
    int token_count = 0;
    char *source_code;
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.bas>\n", argv[0]);
        fprintf(stderr, "Displays tokenization of a BASIC source file.\n");
        return 1;
    }
    
    /* Read source file */
    source_code = read_file(argv[1]);
    if (!source_code) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
        return 1;
    }
    
    /* Initialize tokenizer */
    tokenizer_init(&tokenizer, source_code);
    
    printf("Tokenizing: %s\n", argv[1]);
    printf("════════════════════════════════════════════════════════════\n");
    printf("%-4s  %-5s  %-12s  %-20s  %s\n", "Line", "Col", "Type", "Name", "Value/Text");
    printf("════════════════════════════════════════════════════════════\n");
    
    /* Process all tokens */
    while (1) {
        token = tokenizer_peek(&tokenizer);
        
        /* Stop at end of file */
        if (token->type == TOK_EOF) {
            printf("%-4d  %-5d  %-12s  %-20s\n", 
                   token->line, token->column, "TOK_EOF", token_name(TOK_EOF));
            break;
        }
        
        /* Print token information */
        printf("%-4d  %-5d  0x%02X/%-6d  %-20s  ", 
               token->line, token->column, token->type, token->type, token_name(token->type));
        
        /* Print value or text depending on token type */
        if (token->type == TOK_NUMBER) {
            printf("%g", token->value);
        } else if (token->type == TOK_STRING) {
            printf("\"%s\"", token->text ? token->text : "");
        } else if (token->type == TOK_IDENT) {
            printf("%s", token->text ? token->text : "");
        } else if (token->type == TOK_REM || token->type == TOK_DATA) {
            printf("%s", token->text ? token->text : "");
        }
        
        printf("\n");
        
        /* Advance to next token */
        tokenizer_next(&tokenizer);
        token_count++;
        
        /* Safety limit to prevent infinite loops */
        if (token_count > 100000) {
            fprintf(stderr, "\nError: Token limit exceeded (possible infinite loop)\n");
            break;
        }
    }
    
    printf("════════════════════════════════════════════════════════════\n");
    printf("Total tokens: %d\n", token_count);
    
    tokenizer_free(&tokenizer);
    free(source_code);
    return 0;
}
