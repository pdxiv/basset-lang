/* basset_disasm.c - Disassemble bytecode to human-readable text */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bytecode.h"
#include "bytecode_file.h"
#include "compiler.h"

/* Opcode names table */
static const char* opcode_names[] = {
    /* 0x00 */ "PUSH_CONST", "PUSH_VAR", "POP_VAR", "DUP", "POP", "STR_POP_VAR", "STR_PUSH_VAR", NULL,
    /* 0x08 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x10 */ "ADD", "SUB", "MUL", "DIV", "MOD", "POW", "NEG", NULL,
    /* 0x18 */ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x20 */ "EQ", "NE", "LT", "LE", "GT", "GE", "AND", "OR",
    /* 0x28 */ "NOT", NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x30 */ "STR_PUSH", "STR_CONCAT", "STR_LEN", "STR_VAL", "STR_CHR", "STR_STR", "STR_ASC", "STR_LEFT",
    /* 0x38 */ "STR_RIGHT", "STR_MID", "STR_MID_2", NULL, NULL, NULL, NULL, NULL,
    /* 0x40 */ "ARRAY_GET_1D", "ARRAY_SET_1D", "ARRAY_GET_2D", "ARRAY_SET_2D", "DIM_1D", "DIM_2D", "STR_ARRAY_GET_1D", "STR_ARRAY_SET_1D",
    /* 0x48 */ "STR_ARRAY_GET_2D", "STR_ARRAY_SET_2D", NULL, NULL, NULL, NULL, NULL, NULL,
    /* 0x50 */ "JUMP", "JUMP_IF_FALSE", "JUMP_IF_TRUE", "JUMP_LINE", "GOSUB", "GOSUB_LINE", "RETURN", "ON_GOTO",
    /* 0x58 */ "ON_GOSUB", "FOR_INIT", "FOR_NEXT", NULL, NULL, NULL, NULL, NULL,
    /* 0x60 */ "PRINT_NUM", "PRINT_STR", "PRINT_NEWLINE", "PRINT_SPACE", "PRINT_TAB", "PRINT_NOSEP", "INPUT_NUM", "INPUT_STR",
    /* 0x68 */ "INPUT_PROMPT", "OPEN", "CLOSE", "GET", "PUT", "NOTE", "POINT", "STATUS",
    /* 0x70 */ "XIO", "DATA_READ_NUM", "DATA_READ_STR", "SET_PRINT_CHANNEL", "FUNC_SIN", "FUNC_COS", "FUNC_TAN", "FUNC_ATN",
    /* 0x78 */ "FUNC_EXP", "FUNC_LOG", "FUNC_CLOG", "FUNC_SQR", "FUNC_ABS", "FUNC_INT", "FUNC_RND", "FUNC_SGN",
    /* 0x80 */ NULL, "TRAP", "TRAP_DISABLE", "END", "STOP", "RESTORE", "RESTORE_LINE", "DEG",
    /* 0x88 */ "RAD", "RANDOMIZE", "CLR", "POP_GOSUB", "NOP", "HALT", "FUNC_PEEK", NULL,
};

/* Get opcode name */
static const char* get_opcode_name(uint8_t opcode) {
    const char *name;
    if (opcode >= sizeof(opcode_names) / sizeof(opcode_names[0])) {
        return "UNKNOWN";
    }
    name = opcode_names[opcode];
    return name ? name : "UNKNOWN";
}

/* Validate opcode table for duplicates and completeness */
static void validate_opcodes(FILE *out) {
    int i;
    int unknown_count = 0;
    
    fprintf(out, "; Opcode Validation Report\n");
    
    /* Check for duplicate opcodes by scanning bytecode.h definitions */
    /* For now, just report unknown opcodes in the disassembly */
    for (i = 0; i < 256; i++) {
        const char *name = get_opcode_name((uint8_t)i);
        if (strcmp(name, "UNKNOWN") == 0 && i >= 0x00 && i <= 0x8F) {
            /* Only report unknown in likely range */
            if (unknown_count == 0) {
                fprintf(out, "; Unused opcode slots: ");
            }
            fprintf(out, "0x%02X ", i);
            unknown_count++;
            if (unknown_count % 16 == 0) fprintf(out, "\n;                       ");
        }
    }
    
    if (unknown_count > 0) {
        fprintf(out, "\n; Total unused slots: %d\n", unknown_count);
    }
    fprintf(out, ";\n");
}

/* Check if opcode has operand */
static int has_operand(uint8_t opcode) {
    switch (opcode) {
        case OP_PUSH_CONST:
        case OP_PUSH_VAR:
        case OP_POP_VAR:
        case OP_STR_POP_VAR:
        case OP_STR_PUSH_VAR:
        case OP_STR_PUSH:
        case OP_ARRAY_GET_1D:
        case OP_ARRAY_SET_1D:
        case OP_ARRAY_GET_2D:
        case OP_ARRAY_SET_2D:
        case OP_DIM_1D:
        case OP_DIM_2D:
        case OP_STR_ARRAY_GET_1D:
        case OP_STR_ARRAY_SET_1D:
        case OP_STR_ARRAY_GET_2D:
        case OP_STR_ARRAY_SET_2D:
        case OP_JUMP:
        case OP_JUMP_IF_FALSE:
        case OP_JUMP_IF_TRUE:
        case OP_JUMP_LINE:
        case OP_GOSUB:
        case OP_GOSUB_LINE:
        case OP_ON_GOTO:
        case OP_ON_GOSUB:
        case OP_FOR_INIT:
        case OP_FOR_NEXT:
        case OP_PRINT_TAB:
        case OP_INPUT_NUM:
        case OP_INPUT_STR:
        case OP_INPUT_PROMPT:
        case OP_GET:
        case OP_PUT:
        case OP_XIO:
        case OP_DATA_READ_NUM:
        case OP_DATA_READ_STR:
        case OP_FUNC_PEEK:
        case OP_TRAP:
        case OP_RESTORE_LINE:
            return 1;
        default:
            return 0;
    }
}

int main(int argc, char **argv) {
    CompiledProgram *prog;
    FILE *out;
    size_t i;
    
    /* Check arguments */
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <input.abc> [output.txt]\n", argv[0]);
        fprintf(stderr, "  Disassembles bytecode to human-readable text\n");
        fprintf(stderr, "  Default output: stdout\n");
        return 1;
    }
    
    /* Load bytecode */
    prog = bytecode_file_load(argv[1]);
    if (!prog) {
        return 1;
    }
    
    /* Open output */
    if (argc == 3) {
        out = fopen(argv[2], "w");
        if (!out) {
            fprintf(stderr, "Error: Cannot create output file\n");
            compiled_program_free(prog);
            return 1;
        }
    } else {
        out = stdout;
    }
    
    /* Print header */
    fprintf(out, "; BASIC Bytecode Disassembly\n");
    fprintf(out, "; Instructions: %lu\n", (unsigned long)prog->code_len);
    fprintf(out, "; Constants: %lu\n", (unsigned long)prog->const_count);
    fprintf(out, "; Strings: %lu\n", (unsigned long)prog->string_count);
    fprintf(out, "; Variables: %lu\n", (unsigned long)prog->var_count);
    fprintf(out, "; Lines: %lu\n", (unsigned long)prog->line_count);
    fprintf(out, ";\n");
    
    /* Validate opcode table */
    validate_opcodes(out);
    fprintf(out, "\n");
    
    /* Print constant pool */
    if (prog->const_count > 0) {
        fprintf(out, ".CONST_POOL\n");
        for (i = 0; i < prog->const_count; i++) {
            fprintf(out, "  [%lu] = %.15g\n", (unsigned long)i, prog->const_pool[i]);
        }
        fprintf(out, "\n");
    }
    
    /* Print string pool */
    if (prog->string_count > 0) {
        fprintf(out, ".STRING_POOL\n");
        for (i = 0; i < prog->string_count; i++) {
            fprintf(out, "  [%lu] = \"%s\"\n", (unsigned long)i, prog->string_pool[i]);
        }
        fprintf(out, "\n");
    }
    
    /* Print variable table */
    if (prog->var_count > 0) {
        fprintf(out, ".VARIABLES\n");
        for (i = 0; i < prog->var_count; i++) {
            VariableInfo *var = &prog->var_table[i];
            const char *type_str = "NUMERIC";
            if (var->type == VAR_STRING) type_str = "STRING";
            else if (var->type == VAR_ARRAY_1D) type_str = "ARRAY_1D";
            else if (var->type == VAR_ARRAY_2D) type_str = "ARRAY_2D";
            
            fprintf(out, "  [%d] %s : %s", var->slot, var->name, type_str);
            if (var->type == VAR_ARRAY_1D) {
                fprintf(out, " (%d)", var->array_dim1);
            } else if (var->type == VAR_ARRAY_2D) {
                fprintf(out, " (%d,%d)", var->array_dim1, var->array_dim2);
            }
            fprintf(out, "\n");
        }
        fprintf(out, "\n");
    }
    
    /* Print line map */
    if (prog->line_count > 0) {
        fprintf(out, ".LINE_MAP\n");
        for (i = 0; i < prog->line_count; i++) {
            fprintf(out, "  Line %d -> PC %u\n", 
                    prog->line_map[i].line_number, 
                    prog->line_map[i].pc_offset);
        }
        fprintf(out, "\n");
    }
    
    /* Print DATA pools if present */
    if (prog->data_count > 0) {
        fprintf(out, ".DATA\n");
        for (i = 0; i < prog->data_count; i++) {
            DataEntry *entry = &prog->data_entries[i];
            if (entry->type == DATA_NUMERIC) {
                fprintf(out, "  [%lu] NUMERIC: %.15g\n", (unsigned long)i, 
                        prog->data_numeric_pool[entry->value.numeric_idx]);
            } else {
                fprintf(out, "  [%lu] STRING: \"%s\"\n", (unsigned long)i,
                        prog->data_string_pool[entry->value.string_idx]);
            }
        }
        fprintf(out, "\n");
    }
    
    /* Print bytecode */
    fprintf(out, ".CODE\n");
    for (i = 0; i < prog->code_len; i++) {
        Instruction inst = prog->code[i];
        const char *name = get_opcode_name(inst.opcode);
        size_t j;
        
        /* Find associated line number */
        for (j = 0; j < prog->line_count; j++) {
            if (prog->line_map[j].pc_offset == i) {
                fprintf(out, "\n; Line %d\n", prog->line_map[j].line_number);
                break;
            }
        }
        
        /* Print instruction */
        fprintf(out, "%04lu: %-16s", (unsigned long)i, name);
        
        if (has_operand(inst.opcode)) {
            /* Add context for operand */
            switch (inst.opcode) {
                case OP_PUSH_CONST:
                    if (inst.operand < prog->const_count) {
                        fprintf(out, "%d  ; %.15g", inst.operand, 
                                prog->const_pool[inst.operand]);
                    } else {
                        fprintf(out, "%d", inst.operand);
                    }
                    break;
                
                case OP_STR_PUSH:
                case OP_INPUT_PROMPT:
                    if (inst.operand < prog->string_count) {
                        fprintf(out, "%d  ; \"%s\"", inst.operand,
                                prog->string_pool[inst.operand]);
                    } else {
                        fprintf(out, "%d", inst.operand);
                    }
                    break;
                
                case OP_PUSH_VAR:
                case OP_POP_VAR:
                case OP_INPUT_NUM:
                case OP_INPUT_STR:
                case OP_STR_POP_VAR:
                case OP_STR_PUSH_VAR:
                case OP_ARRAY_GET_1D:
                case OP_ARRAY_SET_1D:
                case OP_ARRAY_GET_2D:
                case OP_ARRAY_SET_2D:
                case OP_DIM_1D:
                case OP_DIM_2D:
                case OP_STR_ARRAY_GET_1D:
                case OP_STR_ARRAY_SET_1D:
                case OP_STR_ARRAY_GET_2D:
                case OP_STR_ARRAY_SET_2D:
                case OP_FOR_INIT:
                case OP_FOR_NEXT:
                    /* Find variable name */
                    for (j = 0; j < prog->var_count; j++) {
                        if (prog->var_table[j].slot == inst.operand) {
                            fprintf(out, "%d  ; %s", inst.operand,
                                    prog->var_table[j].name);
                            break;
                        }
                    }
                    if (j >= prog->var_count) {
                        fprintf(out, "%d", inst.operand);
                    }
                    break;
                
                case OP_JUMP:
                case OP_JUMP_IF_FALSE:
                case OP_JUMP_IF_TRUE:
                case OP_GOSUB:
                    /* Find target line */
                    for (j = 0; j < prog->line_count; j++) {
                        if (prog->line_map[j].pc_offset == inst.operand) {
                            fprintf(out, "%d  ; -> Line %d", inst.operand,
                                    prog->line_map[j].line_number);
                            break;
                        }
                    }
                    if (j >= prog->line_count) {
                        fprintf(out, "%d", inst.operand);
                    }
                    break;
                
                default:
                    fprintf(out, "%d", inst.operand);
                    break;
            }
        }
        
        fprintf(out, "\n");
    }
    
    /* Cleanup */
    if (argc == 3) {
        fclose(out);
        printf("Disassembled to %s\n", argv[2]);
    }
    compiled_program_free(prog);
    
    return 0;
}
