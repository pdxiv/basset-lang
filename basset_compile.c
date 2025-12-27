/* basset_compile.c - Compile BASIC source to bytecode file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "parser.h"
#include "compiler.h"
#include "bytecode_file.h"
#include "syntax_tables.h"
#include "keyword_hash.h"

/* Read entire file into memory */
static char* read_file(const char *filename) {
    FILE *f;
    long size;
    char *buffer;
    size_t bytes_read;
    
    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }
    
    /* Get file size */
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    /* Allocate buffer */
    buffer = malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(f);
        return NULL;
    }
    
    /* Read file */
    bytes_read = fread(buffer, 1, size, f);
    buffer[bytes_read] = '\0';
    
    fclose(f);
    return buffer;
}

int main(int argc, char **argv) {
    char *source;
    char *output_file;
    Tokenizer tokenizer;
    Parser *parser;
    ParseNode *program;
    CompiledProgram *compiled;
    
    /* Check arguments */
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <source.bas> [output.abc]\n", argv[0]);
        fprintf(stderr, "  Compiles BASIC source to binary bytecode\n");
        fprintf(stderr, "  Default output: source.abc\n");
        return 1;
    }
    
    /* Determine output file */
    if (argc == 3) {
        output_file = argv[2];
    } else {
        /* Generate output filename: replace .bas with .abc */
        size_t len = strlen(argv[1]);
        output_file = malloc(len + 5);
        strcpy(output_file, argv[1]);
        
        /* Replace extension */
        if (len > 4 && strcmp(output_file + len - 4, ".bas") == 0) {
            strcpy(output_file + len - 4, ".abc");
        } else {
            strcat(output_file, ".abc");
        }
    }
    
    /* Initialize syntax tables */
    init_syntax_tables();
    
    /* Initialize keyword hash table */
    keyword_hash_init();
    
    /* Read source file */
    source = read_file(argv[1]);
    if (!source) {
        if (argc == 2) free(output_file);
        return 1;
    }
    
    /* Tokenize */
    tokenizer_init(&tokenizer, source);
    
    /* Parse */
    parser = parser_create(&tokenizer);
    program = parser_parse_program(parser);
    
    /* Check for errors */
    if (parser->error_count > 0) {
        fprintf(stderr, "\nCompilation failed with %d error(s)\n", parser->error_count);
        parser_free(parser);
        tokenizer_free(&tokenizer);
        free(source);
        if (argc == 2) free(output_file);
        return 1;
    }
    
    /* Compile to bytecode */
    compiled = compiler_compile(program);
    if (!compiled) {
        fprintf(stderr, "Compilation failed\n");
        parser_free(parser);
        tokenizer_free(&tokenizer);
        free(source);
        if (argc == 2) free(output_file);
        return 1;
    }
    
    /* Save to file */
    printf("Compiling %s -> %s\n", argv[1], output_file);
    printf("  %lu instructions\n", (unsigned long)compiled->code_len);
    printf("  %lu constants\n", (unsigned long)compiled->const_count);
    printf("  %lu strings\n", (unsigned long)compiled->string_count);
    printf("  %lu variables\n", (unsigned long)compiled->var_count);
    printf("  %lu lines\n", (unsigned long)compiled->line_count);
    
    if (!bytecode_file_save(output_file, compiled)) {
        fprintf(stderr, "Failed to save bytecode file\n");
        compiled_program_free(compiled);
        parser_free(parser);
        tokenizer_free(&tokenizer);
        free(source);
        if (argc == 2) free(output_file);
        return 1;
    }
    
    printf("Success!\n");
    
    /* Cleanup */
    compiled_program_free(compiled);
    parser_free(parser);
    tokenizer_free(&tokenizer);
    free(source);
    if (argc == 2) free(output_file);
    
    return 0;
}
