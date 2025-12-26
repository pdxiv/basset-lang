/* basset_asm.c - Assemble text bytecode to binary format */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bytecode.h"
#include "bytecode_file.h"
#include "compiler.h"

/* strdup is not in C90 standard, so we define our own */
static char* my_strdup(const char *s) {
    char *copy = (char*)malloc(strlen(s) + 1);
    if (copy) {
        strcpy(copy, s);
    }
    return copy;
}

#define MAX_LINE 1024

/* Parse variable type string */
static VarType parse_var_type(const char *type_str) {
    if (strcmp(type_str, "STRING") == 0) return VAR_STRING;
    if (strcmp(type_str, "ARRAY_1D") == 0) return VAR_ARRAY_1D;
    if (strcmp(type_str, "ARRAY_2D") == 0) return VAR_ARRAY_2D;
    return VAR_NUMERIC;
}

/* Opcode lookup table */
typedef struct {
    const char *name;
    uint8_t opcode;
} OpcodeEntry;

static const OpcodeEntry opcode_table[] = {
    {"PUSH_CONST", OP_PUSH_CONST},
    {"PUSH_VAR", OP_PUSH_VAR},
    {"POP_VAR", OP_POP_VAR},
    {"DUP", OP_DUP},
    {"POP", OP_POP},
    {"STR_POP_VAR", OP_STR_POP_VAR},
    {"STR_PUSH_VAR", OP_STR_PUSH_VAR},
    {"ADD", OP_ADD},
    {"SUB", OP_SUB},
    {"MUL", OP_MUL},
    {"DIV", OP_DIV},
    {"MOD", OP_MOD},
    {"POW", OP_POW},
    {"NEG", OP_NEG},
    {"EQ", OP_EQ},
    {"NE", OP_NE},
    {"LT", OP_LT},
    {"LE", OP_LE},
    {"GT", OP_GT},
    {"GE", OP_GE},
    {"AND", OP_AND},
    {"OR", OP_OR},
    {"NOT", OP_NOT},
    {"STR_PUSH", OP_STR_PUSH},
    {"STR_CONCAT", OP_STR_CONCAT},
    {"STR_LEN", OP_STR_LEN},
    {"STR_VAL", OP_STR_VAL},
    {"STR_CHR", OP_STR_CHR},
    {"STR_STR", OP_STR_STR},
    {"STR_ASC", OP_STR_ASC},
    {"STR_LEFT", OP_STR_LEFT},
    {"STR_RIGHT", OP_STR_RIGHT},
    {"STR_MID", OP_STR_MID},
    {"STR_MID_2", OP_STR_MID_2},
    {"ARRAY_GET_1D", OP_ARRAY_GET_1D},
    {"ARRAY_SET_1D", OP_ARRAY_SET_1D},
    {"ARRAY_GET_2D", OP_ARRAY_GET_2D},
    {"ARRAY_SET_2D", OP_ARRAY_SET_2D},
    {"DIM_1D", OP_DIM_1D},
    {"DIM_2D", OP_DIM_2D},
    {"STR_ARRAY_GET_1D", OP_STR_ARRAY_GET_1D},
    {"STR_ARRAY_SET_1D", OP_STR_ARRAY_SET_1D},
    {"STR_ARRAY_GET_2D", OP_STR_ARRAY_GET_2D},
    {"STR_ARRAY_SET_2D", OP_STR_ARRAY_SET_2D},
    {"JUMP", OP_JUMP},
    {"JUMP_IF_FALSE", OP_JUMP_IF_FALSE},
    {"JUMP_IF_TRUE", OP_JUMP_IF_TRUE},
    {"JUMP_LINE", OP_JUMP_LINE},
    {"GOSUB", OP_GOSUB},
    {"GOSUB_LINE", OP_GOSUB_LINE},
    {"RETURN", OP_RETURN},
    {"ON_GOTO", OP_ON_GOTO},
    {"ON_GOSUB", OP_ON_GOSUB},
    {"FOR_INIT", OP_FOR_INIT},
    {"FOR_NEXT", OP_FOR_NEXT},
    {"PRINT_NUM", OP_PRINT_NUM},
    {"PRINT_STR", OP_PRINT_STR},
    {"PRINT_NEWLINE", OP_PRINT_NEWLINE},
    {"PRINT_SPACE", OP_PRINT_SPACE},
    {"PRINT_TAB", OP_PRINT_TAB},
    {"PRINT_NOSEP", OP_PRINT_NOSEP},
    {"INPUT_NUM", OP_INPUT_NUM},
    {"INPUT_STR", OP_INPUT_STR},
    {"INPUT_PROMPT", OP_INPUT_PROMPT},
    {"GET", OP_GET},
    {"PUT", OP_PUT},
    {"XIO", OP_XIO},
    {"DATA_READ_NUM", OP_DATA_READ_NUM},
    {"DATA_READ_STR", OP_DATA_READ_STR},
    {"FUNC_SIN", OP_FUNC_SIN},
    {"FUNC_COS", OP_FUNC_COS},
    {"FUNC_TAN", OP_FUNC_TAN},
    {"FUNC_ATN", OP_FUNC_ATN},
    {"FUNC_EXP", OP_FUNC_EXP},
    {"FUNC_LOG", OP_FUNC_LOG},
    {"FUNC_CLOG", OP_FUNC_CLOG},
    {"FUNC_SQR", OP_FUNC_SQR},
    {"FUNC_ABS", OP_FUNC_ABS},
    {"FUNC_INT", OP_FUNC_INT},
    {"FUNC_RND", OP_FUNC_RND},
    {"FUNC_SGN", OP_FUNC_SGN},
    {"FUNC_PEEK", OP_FUNC_PEEK},
    {"TRAP", OP_TRAP},
    {"TRAP_DISABLE", OP_TRAP_DISABLE},
    {"END", OP_END},
    {"STOP", OP_STOP},
    {"RESTORE", OP_RESTORE},
    {"RESTORE_LINE", OP_RESTORE_LINE},
    {"DEG", OP_DEG},
    {"RAD", OP_RAD},
    {"RANDOMIZE", OP_RANDOMIZE},
    {"NOP", OP_NOP},
    {"HALT", OP_HALT},
    {NULL, 0}
};

/* Find opcode by name */
static int find_opcode(const char *name, uint8_t *opcode) {
    int i;
    for (i = 0; opcode_table[i].name != NULL; i++) {
        if (strcmp(opcode_table[i].name, name) == 0) {
            *opcode = opcode_table[i].opcode;
            return 1;
        }
    }
    return 0;
}

/* Trim whitespace */
static char* trim(char *str) {
    char *end;
    
    while (isspace(*str)) str++;
    if (*str == 0) return str;
    
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = 0;
    
    return str;
}

int main(int argc, char **argv) {
    FILE *in;
    char line[MAX_LINE];
    CompiledProgram *prog;
    int line_num = 0;
    enum {
        SECTION_NONE,
        SECTION_CONST,
        SECTION_STRING,
        SECTION_VAR,
        SECTION_LINE,
        SECTION_DATA,
        SECTION_CODE
    } section = SECTION_NONE;
    
    /* Check arguments */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.txt> <output.abc>\n", argv[0]);
        fprintf(stderr, "  Assembles text bytecode to binary format\n");
        return 1;
    }
    
    /* Open input */
    in = fopen(argv[1], "r");
    if (!in) {
        fprintf(stderr, "Error: Cannot open input file\n");
        return 1;
    }
    
    /* Create empty program */
    prog = calloc(1, sizeof(CompiledProgram));
    if (!prog) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(in);
        return 1;
    }
    
    /* Parse file */
    while (fgets(line, sizeof(line), in)) {
        char *p = trim(line);
        line_num++;
        
        /* Skip empty lines and comments */
        if (*p == 0 || *p == ';') continue;
        
        /* Check for section headers */
        if (strcmp(p, ".CONST_POOL") == 0) {
            section = SECTION_CONST;
            continue;
        } else if (strcmp(p, ".STRING_POOL") == 0) {
            section = SECTION_STRING;
            continue;
        } else if (strcmp(p, ".VARIABLES") == 0) {
            section = SECTION_VAR;
            continue;
        } else if (strcmp(p, ".LINE_MAP") == 0) {
            section = SECTION_LINE;
            continue;
        } else if (strcmp(p, ".DATA") == 0) {
            section = SECTION_DATA;
            continue;
        } else if (strcmp(p, ".CODE") == 0) {
            section = SECTION_CODE;
            continue;
        }
        
        /* Parse based on section */
        switch (section) {
            case SECTION_CONST: {
                /* Format: [idx] = value */
                unsigned long idx_ul;
                size_t idx;
                double value;
                if (sscanf(p, " [%lu] = %lf", &idx_ul, &value) == 2) {
                    idx = (size_t)idx_ul;
                    /* Expand pool if needed */
                    while (prog->const_count <= idx) {
                        prog->const_count++;
                        prog->const_pool = realloc(prog->const_pool, 
                            sizeof(double) * prog->const_count);
                    }
                    prog->const_pool[idx] = value;
                }
                break;
            }
            
            case SECTION_STRING: {
                /* Format: [idx] = "string" */
                unsigned long idx_ul;
                size_t idx;
                char *quote1, *quote2;
                if (sscanf(p, " [%lu]", &idx_ul) == 1) {
                    idx = (size_t)idx_ul;
                    quote1 = strchr(p, '"');
                    if (quote1) {
                        quote2 = strrchr(quote1 + 1, '"');
                        if (quote2) {
                            *quote2 = 0;
                            /* Expand pool if needed */
                            while (prog->string_count <= idx) {
                                prog->string_count++;
                                prog->string_pool = realloc(prog->string_pool,
                                    sizeof(char*) * prog->string_count);
                            }
                            prog->string_pool[idx] = my_strdup(quote1 + 1);
                        }
                    }
                }
                break;
            }
            
            case SECTION_VAR: {
                /* Format: [slot] NAME : TYPE [(dim1[,dim2])] */
                unsigned long slot_ul;
                size_t slot;
                char name[256];
                char type_str[32];
                char rest[256];
                int n_scanned;
                uint16_t dim1 = 0, dim2 = 0;
                
                n_scanned = sscanf(p, " [%lu] %255s : %31s %255[^\n]", &slot_ul, name, type_str, rest);
                if (n_scanned >= 3) {
                    slot = (size_t)slot_ul;
                    
                    /* Parse array dimensions if present */
                    if (n_scanned == 4) {
                        if (sscanf(rest, " (%hu,%hu)", &dim1, &dim2) != 2) {
                            sscanf(rest, " (%hu)", &dim1);
                        }
                    }
                    
                    /* Expand table if needed */
                    while (prog->var_count <= slot) {
                        prog->var_count++;
                        prog->var_table = realloc(prog->var_table,
                            sizeof(VariableInfo) * prog->var_count);
                        memset(&prog->var_table[prog->var_count - 1], 0, sizeof(VariableInfo));
                    }
                    
                    /* Set variable info */
                    prog->var_table[slot].name = my_strdup(name);
                    prog->var_table[slot].slot = (uint16_t)slot;
                    prog->var_table[slot].type = parse_var_type(type_str);
                    prog->var_table[slot].array_dim1 = dim1;
                    prog->var_table[slot].array_dim2 = dim2;
                }
                break;
            }
            
            case SECTION_LINE: {
                /* Format: Line NUM -> PC OFFSET */
                uint16_t line_no;
                uint32_t pc_offset;
                if (sscanf(p, " Line %hu -> PC %u", &line_no, &pc_offset) == 2) {
                    prog->line_count++;
                    prog->line_map = realloc(prog->line_map,
                        sizeof(LineMapping) * prog->line_count);
                    prog->line_map[prog->line_count - 1].line_number = line_no;
                    prog->line_map[prog->line_count - 1].pc_offset = pc_offset;
                }
                break;
            }
            
            case SECTION_CODE: {
                /* Format: ADDR: OPCODE [OPERAND] [; comment] */
                unsigned long addr_ul;
                size_t addr;
                char opcode_name[32];
                uint16_t operand = 0;
                uint8_t opcode;
                Instruction inst;
                
                if (sscanf(p, "%lu: %31s %hu", &addr_ul, opcode_name, &operand) >= 2) {
                    addr = (size_t)addr_ul;
                    if (find_opcode(opcode_name, &opcode)) {
                        inst.opcode = opcode;
                        inst.flags = 0;
                        inst.operand = operand;
                        
                        /* Expand code if needed */
                        while (prog->code_len <= addr) {
                            prog->code_len++;
                            prog->code = realloc(prog->code,
                                sizeof(Instruction) * prog->code_len);
                        }
                        prog->code[addr] = inst;
                    } else {
                        fprintf(stderr, "Warning line %d: Unknown opcode '%s'\n",
                                line_num, opcode_name);
                    }
                }
                break;
            }
            
            default:
                break;
        }
    }
    
    fclose(in);
    
    /* Save to binary */
    if (!bytecode_file_save(argv[2], prog)) {
        compiled_program_free(prog);
        return 1;
    }
    
    printf("Assembled %s -> %s\n", argv[1], argv[2]);
    printf("  %lu instructions\n", (unsigned long)prog->code_len);
    
    compiled_program_free(prog);
    return 0;
}
