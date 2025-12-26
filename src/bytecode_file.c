/* bytecode_file.c - Binary bytecode file I/O */
#include "bytecode_file.h"
#include "bytecode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper: Write a size_t count */
static int write_count(FILE *f, size_t count) {
    uint32_t count32 = (uint32_t)count;
    return fwrite(&count32, sizeof(uint32_t), 1, f) == 1;
}

/* Helper: Read a size_t count */
static int read_count(FILE *f, size_t *count) {
    uint32_t count32;
    if (fread(&count32, sizeof(uint32_t), 1, f) != 1) {
        return 0;
    }
    *count = (size_t)count32;
    return 1;
}

/* Helper: Write a string (length-prefixed) */
static int write_string(FILE *f, const char *str) {
    uint32_t len = str ? (uint32_t)strlen(str) : 0;
    if (fwrite(&len, sizeof(uint32_t), 1, f) != 1) {
        return 0;
    }
    if (len > 0) {
        if (fwrite(str, 1, len, f) != len) {
            return 0;
        }
    }
    return 1;
}

/* Helper: Read a string (length-prefixed) */
static char* read_string(FILE *f) {
    uint32_t len;
    char *str;
    
    if (fread(&len, sizeof(uint32_t), 1, f) != 1) {
        return NULL;
    }
    
    if (len == 0) {
        str = malloc(1);
        str[0] = '\0';
        return str;
    }
    
    str = malloc(len + 1);
    if (!str) {
        return NULL;
    }
    
    if (fread(str, 1, len, f) != len) {
        free(str);
        return NULL;
    }
    
    str[len] = '\0';
    return str;
}

/* Save compiled program to binary file */
int bytecode_file_save(const char *filename, CompiledProgram *prog) {
    FILE *f;
    ABCHeader header;
    size_t i;
    
    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Error: Cannot create file '%s'\n", filename);
        return 0;
    }
    
    /* Write header */
    memcpy(header.magic, ABC_MAGIC, 4);
    header.version = ABC_VERSION;
    header.reserved = 0;
    
    if (fwrite(&header, sizeof(ABCHeader), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to write header\n");
        fclose(f);
        return 0;
    }
    
    /* Section 1: Bytecode instructions */
    if (!write_count(f, prog->code_len)) goto error;
    if (prog->code_len > 0) {
        if (fwrite(prog->code, sizeof(Instruction), prog->code_len, f) != prog->code_len) {
            goto error;
        }
    }
    
    /* Section 2: Constant pool */
    if (!write_count(f, prog->const_count)) goto error;
    if (prog->const_count > 0) {
        if (fwrite(prog->const_pool, sizeof(double), prog->const_count, f) != prog->const_count) {
            goto error;
        }
    }
    
    /* Section 3: String pool */
    if (!write_count(f, prog->string_count)) goto error;
    for (i = 0; i < prog->string_count; i++) {
        if (!write_string(f, prog->string_pool[i])) goto error;
    }
    
    /* Section 4: Variable table */
    if (!write_count(f, prog->var_count)) goto error;
    for (i = 0; i < prog->var_count; i++) {
        VariableInfo *var = &prog->var_table[i];
        if (!write_string(f, var->name)) goto error;
        if (fwrite(&var->slot, sizeof(uint16_t), 1, f) != 1) goto error;
        if (fwrite(&var->type, sizeof(uint8_t), 1, f) != 1) goto error;
        if (fwrite(&var->array_dim1, sizeof(uint16_t), 1, f) != 1) goto error;
        if (fwrite(&var->array_dim2, sizeof(uint16_t), 1, f) != 1) goto error;
    }
    
    /* Section 5: Line mappings */
    if (!write_count(f, prog->line_count)) goto error;
    for (i = 0; i < prog->line_count; i++) {
        if (fwrite(&prog->line_map[i].line_number, sizeof(uint16_t), 1, f) != 1) goto error;
        if (fwrite(&prog->line_map[i].pc_offset, sizeof(uint32_t), 1, f) != 1) goto error;
    }
    
    /* Section 6: DATA numeric pool */
    if (!write_count(f, prog->data_numeric_count)) goto error;
    if (prog->data_numeric_count > 0) {
        if (fwrite(prog->data_numeric_pool, sizeof(double), prog->data_numeric_count, f) != prog->data_numeric_count) {
            goto error;
        }
    }
    
    /* Section 7: DATA string pool */
    if (!write_count(f, prog->data_string_count)) goto error;
    for (i = 0; i < prog->data_string_count; i++) {
        if (!write_string(f, prog->data_string_pool[i])) goto error;
    }
    
    /* Section 8: DATA entries */
    if (!write_count(f, prog->data_count)) goto error;
    for (i = 0; i < prog->data_count; i++) {
        DataEntry *entry = &prog->data_entries[i];
        if (fwrite(&entry->type, sizeof(uint8_t), 1, f) != 1) goto error;
        if (entry->type == DATA_NUMERIC) {
            uint32_t idx = (uint32_t)entry->value.numeric_idx;
            if (fwrite(&idx, sizeof(uint32_t), 1, f) != 1) goto error;
        } else {
            uint32_t idx = (uint32_t)entry->value.string_idx;
            if (fwrite(&idx, sizeof(uint32_t), 1, f) != 1) goto error;
        }
    }
    
    fclose(f);
    return 1;
    
error:
    fprintf(stderr, "Error: Failed to write bytecode file\n");
    fclose(f);
    return 0;
}

/* Load compiled program from binary file */
CompiledProgram* bytecode_file_load(const char *filename) {
    FILE *f;
    ABCHeader header;
    CompiledProgram *prog;
    size_t i;
    
    f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }
    
    /* Read header */
    if (fread(&header, sizeof(ABCHeader), 1, f) != 1) {
        fprintf(stderr, "Error: Failed to read header\n");
        fclose(f);
        return NULL;
    }
    
    /* Validate header */
    if (memcmp(header.magic, ABC_MAGIC, 3) != 0) {
        fprintf(stderr, "Error: Invalid file format (bad magic)\n");
        fclose(f);
        return NULL;
    }
    
    if (header.version != ABC_VERSION) {
        fprintf(stderr, "Error: Unsupported file version %d\n", header.version);
        fclose(f);
        return NULL;
    }
    
    /* Allocate program structure */
    prog = calloc(1, sizeof(CompiledProgram));
    if (!prog) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(f);
        return NULL;
    }
    
    /* Section 1: Bytecode instructions */
    if (!read_count(f, &prog->code_len)) goto error;
    prog->code_capacity = prog->code_len;
    if (prog->code_len > 0) {
        prog->code = malloc(sizeof(Instruction) * prog->code_len);
        if (!prog->code) goto error;
        if (fread(prog->code, sizeof(Instruction), prog->code_len, f) != prog->code_len) {
            goto error;
        }
    }
    
    /* Section 2: Constant pool */
    if (!read_count(f, &prog->const_count)) goto error;
    prog->const_capacity = prog->const_count;
    if (prog->const_count > 0) {
        prog->const_pool = malloc(sizeof(double) * prog->const_count);
        if (!prog->const_pool) goto error;
        if (fread(prog->const_pool, sizeof(double), prog->const_count, f) != prog->const_count) {
            goto error;
        }
    }
    
    /* Section 3: String pool */
    if (!read_count(f, &prog->string_count)) goto error;
    prog->string_capacity = prog->string_count;
    if (prog->string_count > 0) {
        prog->string_pool = malloc(sizeof(char*) * prog->string_count);
        if (!prog->string_pool) goto error;
        for (i = 0; i < prog->string_count; i++) {
            prog->string_pool[i] = read_string(f);
            if (!prog->string_pool[i]) goto error;
        }
    }
    
    /* Section 4: Variable table */
    if (!read_count(f, &prog->var_count)) goto error;
    prog->var_capacity = prog->var_count;
    if (prog->var_count > 0) {
        prog->var_table = malloc(sizeof(VariableInfo) * prog->var_count);
        if (!prog->var_table) goto error;
        for (i = 0; i < prog->var_count; i++) {
            VariableInfo *var = &prog->var_table[i];
            var->name = read_string(f);
            if (!var->name) goto error;
            if (fread(&var->slot, sizeof(uint16_t), 1, f) != 1) goto error;
            if (fread(&var->type, sizeof(uint8_t), 1, f) != 1) goto error;
            if (fread(&var->array_dim1, sizeof(uint16_t), 1, f) != 1) goto error;
            if (fread(&var->array_dim2, sizeof(uint16_t), 1, f) != 1) goto error;
        }
    }
    
    /* Section 5: Line mappings */
    if (!read_count(f, &prog->line_count)) goto error;
    prog->line_capacity = prog->line_count;
    if (prog->line_count > 0) {
        prog->line_map = malloc(sizeof(LineMapping) * prog->line_count);
        if (!prog->line_map) goto error;
        for (i = 0; i < prog->line_count; i++) {
            if (fread(&prog->line_map[i].line_number, sizeof(uint16_t), 1, f) != 1) goto error;
            if (fread(&prog->line_map[i].pc_offset, sizeof(uint32_t), 1, f) != 1) goto error;
        }
    }
    
    /* Section 6: DATA numeric pool */
    if (!read_count(f, &prog->data_numeric_count)) goto error;
    prog->data_numeric_capacity = prog->data_numeric_count;
    if (prog->data_numeric_count > 0) {
        prog->data_numeric_pool = malloc(sizeof(double) * prog->data_numeric_count);
        if (!prog->data_numeric_pool) goto error;
        if (fread(prog->data_numeric_pool, sizeof(double), prog->data_numeric_count, f) != prog->data_numeric_count) {
            goto error;
        }
    }
    
    /* Section 7: DATA string pool */
    if (!read_count(f, &prog->data_string_count)) goto error;
    prog->data_string_capacity = prog->data_string_count;
    if (prog->data_string_count > 0) {
        prog->data_string_pool = malloc(sizeof(char*) * prog->data_string_count);
        if (!prog->data_string_pool) goto error;
        for (i = 0; i < prog->data_string_count; i++) {
            prog->data_string_pool[i] = read_string(f);
            if (!prog->data_string_pool[i]) goto error;
        }
    }
    
    /* Section 8: DATA entries */
    if (!read_count(f, &prog->data_count)) goto error;
    prog->data_capacity = prog->data_count;
    if (prog->data_count > 0) {
        prog->data_entries = malloc(sizeof(DataEntry) * prog->data_count);
        if (!prog->data_entries) goto error;
        for (i = 0; i < prog->data_count; i++) {
            DataEntry *entry = &prog->data_entries[i];
            uint8_t type_byte;
            uint32_t idx;
            
            if (fread(&type_byte, sizeof(uint8_t), 1, f) != 1) goto error;
            entry->type = (DataType)type_byte;
            
            if (fread(&idx, sizeof(uint32_t), 1, f) != 1) goto error;
            
            if (entry->type == DATA_NUMERIC) {
                entry->value.numeric_idx = (size_t)idx;
            } else {
                entry->value.string_idx = (size_t)idx;
            }
        }
    }
    
    fclose(f);
    return prog;
    
error:
    fprintf(stderr, "Error: Failed to load bytecode file\n");
    if (prog) {
        compiled_program_free(prog);
    }
    fclose(f);
    return NULL;
}
