/* bytecode_file.h - Binary bytecode file format */
#ifndef BYTECODE_FILE_H
#define BYTECODE_FILE_H

#include "compiler.h"
#include <stdio.h>

/* BASIC Compiled (.abc) file format
 * 
 * Header:
 *   Magic: "ABC\0" (4 bytes)
 *   Version: uint16_t (2 bytes)
 *   Reserved: uint16_t (2 bytes)
 * 
 * Sections (each with count + data):
 *   1. Bytecode instructions
 *   2. Constant pool (doubles)
 *   3. String pool
 *   4. Variable table
 *   5. Line mappings
 *   6. DATA numeric pool
 *   7. DATA string pool
 *   8. DATA entries
 */

#define ABC_MAGIC "ABC"
#define ABC_VERSION 1

/* File header */
typedef struct {
    char magic[4];        /* "ABC\0" */
    uint16_t version;     /* File format version */
    uint16_t reserved;    /* Reserved for future use */
} ABCHeader;

/* Save compiled program to binary file */
int bytecode_file_save(const char *filename, CompiledProgram *prog);

/* Load compiled program from binary file */
CompiledProgram* bytecode_file_load(const char *filename);

/* Disassemble bytecode to human-readable text */
int bytecode_disassemble(const char *input_file, const char *output_file);

/* Assemble text bytecode to binary file */
int bytecode_assemble(const char *input_file, const char *output_file);

#endif /* BYTECODE_FILE_H */
