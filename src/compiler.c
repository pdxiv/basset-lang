/* compiler.c - Bytecode compiler implementation */
#define _POSIX_C_SOURCE 200112L  /* Enable snprintf */
#include "compiler.h"
#include "tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* Maximum variables (Atari BASIC compatibility) */
#define MAX_NUMERIC_VARS 128
#define MAX_STRING_VARS 128
#define MAX_ARRAYS 64

/* K&R C compatible strdup */
static char* my_strdup(const char *s) {
    char *d;
    if (!s) return NULL;
    d = malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}

/* Create new compiler state */
CompilerState* compiler_state_new(void) {
    CompilerState *cs = calloc(1, sizeof(CompilerState));
    if (!cs) return NULL;
    
    cs->program = calloc(1, sizeof(CompiledProgram));
    if (!cs->program) {
        free(cs);
        return NULL;
    }
    
    /* Initialize capacities */
    cs->program->code_capacity = 256;
    cs->program->code = malloc(sizeof(Instruction) * cs->program->code_capacity);
    
    cs->program->const_capacity = 64;
    cs->program->const_pool = malloc(sizeof(double) * cs->program->const_capacity);
    
    cs->program->string_capacity = 64;
    cs->program->string_pool = malloc(sizeof(char*) * cs->program->string_capacity);
    
    cs->program->var_capacity = 64;
    cs->program->var_table = malloc(sizeof(VariableInfo) * cs->program->var_capacity);
    
    cs->program->line_capacity = 256;
    cs->program->line_map = malloc(sizeof(LineMapping) * cs->program->line_capacity);
    
    cs->program->data_capacity = 64;
    cs->program->data_entries = malloc(sizeof(DataEntry) * cs->program->data_capacity);
    
    cs->program->data_numeric_capacity = 64;
    cs->program->data_numeric_pool = malloc(sizeof(double) * cs->program->data_numeric_capacity);
    
    cs->program->data_string_capacity = 64;
    cs->program->data_string_pool = malloc(sizeof(char*) * cs->program->data_string_capacity);
    
    cs->fixup_capacity = 64;
    cs->jump_fixups = malloc(sizeof(JumpFixup) * cs->fixup_capacity);
    
    return cs;
}

/* Free compiler state */
void compiler_state_free(CompilerState *cs) {
    if (!cs) return;
    
    if (cs->jump_fixups) free(cs->jump_fixups);
    
    /* Don't free program - it's returned to caller */
    free(cs);
}

/* Free compiled program */
void compiled_program_free(CompiledProgram *prog) {
    size_t i;
    if (!prog) return;
    
    if (prog->code) free(prog->code);
    if (prog->const_pool) free(prog->const_pool);
    
    if (prog->string_pool) {
        for (i = 0; i < prog->string_count; i++) {
            if (prog->string_pool[i]) free(prog->string_pool[i]);
        }
        free(prog->string_pool);
    }
    
    if (prog->var_table) {
        for (i = 0; i < prog->var_count; i++) {
            if (prog->var_table[i].name) free(prog->var_table[i].name);
        }
        free(prog->var_table);
    }
    
    if (prog->line_map) free(prog->line_map);
    if (prog->data_entries) free(prog->data_entries);
    if (prog->data_numeric_pool) free(prog->data_numeric_pool);
    
    if (prog->data_string_pool) {
        for (i = 0; i < prog->data_string_count; i++) {
            if (prog->data_string_pool[i]) free(prog->data_string_pool[i]);
        }
        free(prog->data_string_pool);
    }
    
    free(prog);
}

/* Find variable in symbol table, returns slot or -1 */
int compiler_find_variable(CompilerState *cs, const char *name) {
    size_t i;
    for (i = 0; i < cs->program->var_count; i++) {
        if (strcmp(cs->program->var_table[i].name, name) == 0) {
            return cs->program->var_table[i].slot;
        }
    }
    return -1;
}

/* Add variable to symbol table, returns slot */
int compiler_add_variable(CompilerState *cs, const char *name, VarType type) {
    int existing;
    VariableInfo *var;
    int is_string = (type == VAR_STRING);
    int is_array = (type == VAR_ARRAY_1D || type == VAR_ARRAY_2D);
    int num_count = 0;
    int str_count = 0;
    int array_count = 0;
    size_t i;
    
    /* Check if already exists */
    existing = compiler_find_variable(cs, name);
    if (existing >= 0) return existing;
    
    /* Count existing variables by type to enforce limits */
    for (i = 0; i < cs->program->var_count; i++) {
        VarType vtype = cs->program->var_table[i].type;
        if (vtype == VAR_STRING) {
            str_count++;
        } else if (vtype == VAR_ARRAY_1D || vtype == VAR_ARRAY_2D) {
            array_count++;
        } else {
            num_count++;
        }
    }
    
    /* Check limits */
    if (is_string && str_count >= MAX_STRING_VARS) {
        fprintf(stderr, "Error: Too many string variables (maximum %d)\n", MAX_STRING_VARS);
        fprintf(stderr, "  Variable '%s' cannot be allocated.\n", name);
        return -1;
    }
    if (is_array && array_count >= MAX_ARRAYS) {
        fprintf(stderr, "Error: Too many arrays (maximum %d)\n", MAX_ARRAYS);
        fprintf(stderr, "  Array '%s' cannot be allocated.\n", name);
        return -1;
    }
    if (!is_string && !is_array && num_count >= MAX_NUMERIC_VARS) {
        fprintf(stderr, "Error: Too many numeric variables (maximum %d)\n", MAX_NUMERIC_VARS);
        fprintf(stderr, "  Variable '%s' cannot be allocated.\n", name);
        return -1;
    }
    
    /* Expand if needed */
    if (cs->program->var_count >= cs->program->var_capacity) {
        cs->program->var_capacity *= 2;
        cs->program->var_table = realloc(cs->program->var_table,
            sizeof(VariableInfo) * cs->program->var_capacity);
    }
    
    var = &cs->program->var_table[cs->program->var_count];
    var->name = my_strdup(name);
    var->slot = cs->program->var_count;
    var->type = type;
    var->array_dim1 = 0;
    var->array_dim2 = 0;
    
    cs->program->var_count++;
    return var->slot;
}

/* Add numeric constant to pool, returns index */
uint16_t compiler_add_const(CompilerState *cs, double value) {
    size_t i;
    
    /* Check if constant already exists */
    for (i = 0; i < cs->program->const_count; i++) {
        if (cs->program->const_pool[i] == value) {
            return i;
        }
    }
    
    /* Expand if needed */
    if (cs->program->const_count >= cs->program->const_capacity) {
        cs->program->const_capacity *= 2;
        cs->program->const_pool = realloc(cs->program->const_pool,
            sizeof(double) * cs->program->const_capacity);
    }
    
    cs->program->const_pool[cs->program->const_count] = value;
    return cs->program->const_count++;
}

/* Add string to pool, returns index */
uint16_t compiler_add_string(CompilerState *cs, const char *str) {
    size_t i;
    
    /* Check if string already exists */
    for (i = 0; i < cs->program->string_count; i++) {
        if (strcmp(cs->program->string_pool[i], str) == 0) {
            return i;
        }
    }
    
    /* Expand if needed */
    if (cs->program->string_count >= cs->program->string_capacity) {
        cs->program->string_capacity *= 2;
        cs->program->string_pool = realloc(cs->program->string_pool,
            sizeof(char*) * cs->program->string_capacity);
    }
    
    cs->program->string_pool[cs->program->string_count] = my_strdup(str);
    return cs->program->string_count++;
}

/* Emit instruction */
void compiler_emit(CompilerState *cs, uint8_t opcode, uint16_t operand) {
    Instruction inst;
    
    /* Expand if needed */
    if (cs->program->code_len >= cs->program->code_capacity) {
        cs->program->code_capacity *= 2;
        cs->program->code = realloc(cs->program->code,
            sizeof(Instruction) * cs->program->code_capacity);
    }
    
    inst.opcode = opcode;
    inst.flags = 0;
    inst.operand = operand;
    
    cs->program->code[cs->program->code_len++] = inst;
}

/* Emit instruction without operand */
void compiler_emit_no_operand(CompilerState *cs, uint8_t opcode) {
    compiler_emit(cs, opcode, 0);
}

/* Emit raw operand (for jump tables in ON GOTO/GOSUB) */
void compiler_emit_raw(CompilerState *cs, uint16_t operand) {
    Instruction inst;
    
    /* Expand if needed */
    if (cs->program->code_len >= cs->program->code_capacity) {
        cs->program->code_capacity *= 2;
        cs->program->code = realloc(cs->program->code,
            sizeof(Instruction) * cs->program->code_capacity);
    }
    
    inst.opcode = OP_NOP;  /* Use NOP as placeholder */
    inst.flags = 0;
    inst.operand = operand;
    
    cs->program->code[cs->program->code_len++] = inst;
}

/* Add line mapping */
void compiler_add_line_mapping(CompilerState *cs, uint16_t line, uint32_t pc) {
    LineMapping *map;
    
    /* Expand if needed */
    if (cs->program->line_count >= cs->program->line_capacity) {
        cs->program->line_capacity *= 2;
        cs->program->line_map = realloc(cs->program->line_map,
            sizeof(LineMapping) * cs->program->line_capacity);
    }
    
    map = &cs->program->line_map[cs->program->line_count++];
    map->line_number = line;
    map->pc_offset = pc;
}

/* Find line offset (binary search) */
int32_t compiler_find_line_offset(CompilerState *cs, uint16_t line) {
    int left = 0;
    int right = cs->program->line_count - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (cs->program->line_map[mid].line_number == line) {
            return cs->program->line_map[mid].pc_offset;
        } else if (cs->program->line_map[mid].line_number < line) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;  /* Not found */
}

/* Add jump fixup for forward reference */
void compiler_add_jump_fixup(CompilerState *cs, uint32_t pc, uint16_t target_line, JumpType type) {
    JumpFixup *fixup;
    
    /* Expand if needed */
    if (cs->fixup_count >= cs->fixup_capacity) {
        cs->fixup_capacity *= 2;
        cs->jump_fixups = realloc(cs->jump_fixups,
            sizeof(JumpFixup) * cs->fixup_capacity);
    }
    
    fixup = &cs->jump_fixups[cs->fixup_count++];
    fixup->instruction_offset = pc;
    fixup->target_line = target_line;
    fixup->type = type;
}

/* Resolve all jump fixups */
void compiler_resolve_jumps(CompilerState *cs) {
    size_t i;
    
    for (i = 0; i < cs->fixup_count; i++) {
        JumpFixup *fixup = &cs->jump_fixups[i];
        int32_t offset = compiler_find_line_offset(cs, fixup->target_line);
        
        if (offset < 0) {
            snprintf(cs->error_msg, sizeof(cs->error_msg),
                "Undefined line number: %d", fixup->target_line);
            cs->has_error = 1;
            return;
        }
        
        /* Patch the instruction */
        cs->program->code[fixup->instruction_offset].operand = offset;
    }
}

/* Forward declarations for compilation functions */
static void compile_statement(CompilerState *cs, ParseNode *stmt);
static void compile_expression(CompilerState *cs, ParseNode *expr);
static void discover_variables_in_tree(CompilerState *cs, ParseNode *node);

/* Determine variable type from name */
static VarType get_var_type(const char *name) {
    if (strchr(name, '$')) return VAR_STRING;
    return VAR_NUMERIC;
}

/* Phase 1: Variable Discovery - walk tree and register all variables */
static void discover_variables_in_tree(CompilerState *cs, ParseNode *node) {
    size_t i;
    
    if (!node) return;
    
    /* If this is a variable reference, register it */
    if (node->type == NODE_VARIABLE && node->text) {
        VarType type = get_var_type(node->text);
        compiler_add_variable(cs, node->text, type);
    }
    
    /* Handle DIM statements specially */
    if (node->type == NODE_STATEMENT && node->token == TOK_DIM) {
        /* DIM statements declare arrays - we'll handle them in statement compilation */
        /* For now just register the variable name */
        if (node->child_count > 0 && node->children[0]) {
            ParseNode *var_node = node->children[0];
            /* Navigate to actual variable */
            while (var_node && var_node->type == NODE_EXPRESSION && var_node->child_count > 0) {
                var_node = var_node->children[0];
            }
            if (var_node && var_node->type == NODE_VARIABLE && var_node->text) {
                /* Will determine dimensions during statement compilation */
                compiler_add_variable(cs, var_node->text, VAR_ARRAY_1D);
            }
        }
    }
    
    /* Recurse to children */
    for (i = 0; i < node->child_count; i++) {
        discover_variables_in_tree(cs, node->children[i]);
    }
}

/* Compile expression - post-order traversal */
static void compile_expression(CompilerState *cs, ParseNode *expr) {
    uint16_t idx;
    int slot;
    

    
    if (!expr) {
        compiler_emit(cs, OP_PUSH_CONST, compiler_add_const(cs, 0.0));
        return;
    }
    
    switch (expr->type) {
        case NODE_CONSTANT:
            /* Check if it's a string constant */
            if (expr->token == TOK_STRING) {
                idx = compiler_add_string(cs, expr->text);

                compiler_emit(cs, OP_STR_PUSH, idx);
            } else {
                /* Numeric constant */
                idx = compiler_add_const(cs, expr->value);

                compiler_emit(cs, OP_PUSH_CONST, idx);
            }
            break;
            
        case NODE_VARIABLE:
            slot = compiler_find_variable(cs, expr->text);
            if (slot < 0) {
                /* Variable not found - add it */
                slot = compiler_add_variable(cs, expr->text, get_var_type(expr->text));
            }
            
            /* Check if it's an array access */
            if (expr->child_count > 0) {
                /* Array access */
                int is_string = strchr(expr->text, '$') != NULL;
                compile_expression(cs, expr->children[0]);  /* Index */
                if (expr->child_count > 1) {
                    /* 2D array */
                    compile_expression(cs, expr->children[1]);
                    if (is_string) {
                        compiler_emit(cs, OP_STR_ARRAY_GET_2D, slot);
                    } else {
                        compiler_emit(cs, OP_ARRAY_GET_2D, slot);
                    }
                } else {
                    /* 1D array */
                    if (is_string) {
                        compiler_emit(cs, OP_STR_ARRAY_GET_1D, slot);
                    } else {
                        compiler_emit(cs, OP_ARRAY_GET_1D, slot);
                    }
                }
            } else {
                /* Simple variable */
                if (strchr(expr->text, '$')) {
                    /* String variable */
                    compiler_emit(cs, OP_STR_PUSH_VAR, slot);
                } else {
                    compiler_emit(cs, OP_PUSH_VAR, slot);
                }
            }
            break;
            

            
        case NODE_OPERATOR:
            /* Binary or unary operator - compile operands first (postfix) */
            if (expr->child_count >= 2) {
                /* Binary operator */
                compile_expression(cs, expr->children[0]);  /* Left */
                compile_expression(cs, expr->children[1]);  /* Right */
                
                /* Emit operator */
                switch (expr->token) {
                    case TOK_CPLUS:   compiler_emit_no_operand(cs, OP_ADD); break;
                    case TOK_CMINUS:  compiler_emit_no_operand(cs, OP_SUB); break;
                    case TOK_CMUL:    compiler_emit_no_operand(cs, OP_MUL); break;
                    case TOK_CDIV:    compiler_emit_no_operand(cs, OP_DIV); break;
                    case TOK_CEXP:    compiler_emit_no_operand(cs, OP_POW); break;
                    case TOK_CEQ:     compiler_emit_no_operand(cs, OP_EQ); break;
                    case TOK_CNE:     compiler_emit_no_operand(cs, OP_NE); break;
                    case TOK_CLT:     compiler_emit_no_operand(cs, OP_LT); break;
                    case TOK_CLE:     compiler_emit_no_operand(cs, OP_LE); break;
                    case TOK_CGT:     compiler_emit_no_operand(cs, OP_GT); break;
                    case TOK_CGE:     compiler_emit_no_operand(cs, OP_GE); break;
                    case TOK_CAND:    compiler_emit_no_operand(cs, OP_AND); break;
                    case TOK_COR:     compiler_emit_no_operand(cs, OP_OR); break;
                    default:
                        /* Unknown operator */
                        break;
                }
            } else if (expr->child_count == 1) {
                /* Unary operator */
                compile_expression(cs, expr->children[0]);
                if (expr->token == TOK_CMINUS || expr->token == TOK_CUMINUS) {
                    compiler_emit_no_operand(cs, OP_NEG);
                } else if (expr->token == TOK_CNOT) {
                    compiler_emit_no_operand(cs, OP_NOT);
                }
            }
            break;
            
        case NODE_FUNCTION_CALL:
            /* Compile function arguments (pushed in order) */
            if (expr->child_count > 0) {
                size_t i;
                for (i = 0; i < expr->child_count; i++) {
                    compile_expression(cs, expr->children[i]);
                }
            }
            
            /* Emit function opcode */
            switch (expr->token) {
                case TOK_CSIN: compiler_emit_no_operand(cs, OP_FUNC_SIN); break;
                case TOK_CCOS: compiler_emit_no_operand(cs, OP_FUNC_COS); break;
                case TOK_CATN: compiler_emit_no_operand(cs, OP_FUNC_ATN); break;
                case TOK_CEXP_F: compiler_emit_no_operand(cs, OP_FUNC_EXP); break;
                case TOK_CLOG: compiler_emit_no_operand(cs, OP_FUNC_LOG); break;
                case TOK_CCLOG: compiler_emit_no_operand(cs, OP_FUNC_CLOG); break;
                case TOK_CSQR: compiler_emit_no_operand(cs, OP_FUNC_SQR); break;
                case TOK_CABS: compiler_emit_no_operand(cs, OP_FUNC_ABS); break;
                case TOK_CINT: compiler_emit_no_operand(cs, OP_FUNC_INT); break;
                case TOK_CRND: compiler_emit_no_operand(cs, OP_FUNC_RND); break;
                case TOK_CSGN: compiler_emit_no_operand(cs, OP_FUNC_SGN); break;
                case TOK_CPEEK: compiler_emit_no_operand(cs, OP_FUNC_PEEK); break;
                
                /* String functions */
                case TOK_CLEFT: compiler_emit_no_operand(cs, OP_STR_LEFT); break;
                case TOK_CRIGHT: compiler_emit_no_operand(cs, OP_STR_RIGHT); break;
                case TOK_CMID: 
                    /* MID$ can have 2 or 3 arguments */
                    if (expr->child_count == 2) {
                        /* MID$(str, start) - return from start to end */
                        compiler_emit_no_operand(cs, OP_STR_MID_2);
                    } else {
                        /* MID$(str, start, len) - return len chars from start */
                        compiler_emit_no_operand(cs, OP_STR_MID);
                    }
                    break;
                case TOK_CLEN: compiler_emit_no_operand(cs, OP_STR_LEN); break;
                case TOK_CCHR: compiler_emit_no_operand(cs, OP_STR_CHR); break;
                case TOK_CASC: compiler_emit_no_operand(cs, OP_STR_ASC); break;
                case TOK_CSTR: compiler_emit_no_operand(cs, OP_STR_STR); break;
                case TOK_CVAL: compiler_emit_no_operand(cs, OP_STR_VAL); break;
                
                /* TAB function - special handling for PRINT */
                case TOK_CTAB: compiler_emit_no_operand(cs, OP_TAB_FUNC); break;
                
                default: break;
            }
            break;
            
        case NODE_EXPRESSION:
            /* Unwrap expression node */
            if (expr->child_count == 1) {
                compile_expression(cs, expr->children[0]);
            } else if (expr->child_count == 0) {
                compiler_emit(cs, OP_PUSH_CONST, compiler_add_const(cs, 0.0));
            } else {
                /* Multiple children - check if this represents an operator expression */
                /* Look for an operator node among the children */
                int found_operator = 0;
                size_t i;
                for (i = 0; i < expr->child_count; i++) {
                    if (expr->children[i] && expr->children[i]->type == NODE_OPERATOR) {
                        /* Found an operator - compile this as the operator node */
                        compile_expression(cs, expr->children[i]);
                        found_operator = 1;
                        break;
                    }
                }
                if (!found_operator) {
                    /* No operator found - just compile all children sequentially (shouldn't happen) */
                    for (i = 0; i < expr->child_count; i++) {
                        compile_expression(cs, expr->children[i]);
                    }
                }
            }
            break;
            
        default:
            /* Unknown node type */
            compiler_emit(cs, OP_PUSH_CONST, compiler_add_const(cs, 0.0));
            break;
    }
}

/* Helper: recursively find first NODE_VARIABLE or NODE_CONSTANT in tree */
static ParseNode* find_leaf_node(ParseNode *node, NodeType target_type) {
    int i;
    if (!node) return NULL;
    if (node->type == target_type) return node;
    
    for (i = 0; i < node->child_count; i++) {
        ParseNode *result = find_leaf_node(node->children[i], target_type);
        if (result) return result;
    }
    return NULL;
}

/* Compile assignment statement */
static void compile_assignment(CompilerState *cs, ParseNode *stmt) {
    ParseNode *var_node, *expr_node;
    ParseNode *actual_var, *actual_const;
    int slot;
    
    
    if (stmt->child_count < 2) return;
    
    var_node = stmt->children[0];
    expr_node = stmt->children[1];
    
    
    /* Find the actual variable node (might be wrapped in EXPRESSION nodes) */
    actual_var = find_leaf_node(var_node, NODE_VARIABLE);
    if (!actual_var || !actual_var->text) {
        return;
    }
    
    
    slot = compiler_find_variable(cs, actual_var->text);
    if (slot < 0) {
        slot = compiler_add_variable(cs, actual_var->text, get_var_type(actual_var->text));
    }
    
    
    /* Find the actual constant/expression (might be wrapped) */
    actual_const = find_leaf_node(expr_node, NODE_CONSTANT);
    if (actual_const) {
        /* Compile the constant directly */
        compile_expression(cs, actual_const);
    } else {
        /* Not a simple constant - compile the whole expression tree */
        compile_expression(cs, expr_node);
    }
    
    
    /* Check if it's array assignment */
    if (actual_var->child_count > 0) {
        /* Array assignment: ARR(I) = value */
        /* Need to compile indices first, then value */
        /* For now, emit POP_VAR - will handle arrays properly later */
        compiler_emit(cs, OP_POP_VAR, slot);
    } else {
        /* Simple variable assignment */
        compiler_emit(cs, OP_POP_VAR, slot);
    }
}

/* Compile PRINT statement */
static void compile_print(CompilerState *cs, ParseNode *stmt) {
    size_t i, j;
    size_t first_item = 0;
    int has_trailing_separator = 0;
    int is_string;
    int has_channel = 0;
    
    /* Check if this is PRINT #channel by detecting the pattern:
     * First child is expression, second child is also expression (not separator)
     * This indicates PRINT #channel, expr1, expr2...
     * Normal PRINT has: expr1, separator, expr2, separator...
     */
    if (stmt->child_count >= 2) {
        ParseNode *first = stmt->children[0];
        ParseNode *second = stmt->children[1];
        
        if (first && second &&
            first->type != NODE_OPERATOR &&
            second->type != NODE_OPERATOR) {
            /* Both are expressions, so first is channel */
            has_channel = 1;
            first_item = 1;
        }
    } else if (stmt->child_count == 1) {
        /* Could be PRINT #channel with nothing after, or just PRINT expr */
        /* For now, assume it's just a regular print */
        /* If we need to support PRINT #1 with no output, we'd need better detection */
    }
    
    /* If we have a channel, compile it and emit opcode to set channel */
    if (has_channel) {
        compile_expression(cs, stmt->children[0]);
        compiler_emit_no_operand(cs, OP_SET_PRINT_CHANNEL);
    }
    
    /* Process each child (argument or separator), starting from first_item */
    for (i = first_item; i < stmt->child_count; i++) {
        ParseNode *child = stmt->children[i];
        
        /* Skip null children */
        if (!child) continue;
        
        if (child->type == NODE_OPERATOR && (child->token == TOK_CSC || child->token == TOK_CCOM)) {
            /* Separator (semicolon or comma) */
            if (child->token == TOK_CSC) {
                /* Semicolon - no opcode needed, just mark as having separator */
                has_trailing_separator = 1;
            } else if (child->token == TOK_CCOM) {
                /* Comma - print tab */
                compiler_emit_no_operand(cs, OP_PRINT_TAB);
                has_trailing_separator = 1;
            }
        } else {
            /* Check if this is a TAB() function - special handling */
            if (child->type == NODE_FUNCTION_CALL && child->token == TOK_CTAB) {
                /* TAB doesn't print a value, it just positions cursor */
                compile_expression(cs, child);  /* This will emit OP_TAB_FUNC */
                has_trailing_separator = 1;  /* Treat like a separator */
                continue;  /* Don't emit PRINT_NUM/PRINT_STR */
            }
            
            /* Expression to print - compile it */
            compile_expression(cs, child);
            
            /* Determine if it's string or numeric by checking the top-level node type */
            /* For expressions, we default to numeric unless it's clearly a string */
            is_string = 0;
            if (child->type == NODE_CONSTANT && child->token == TOK_STRING) {
                is_string = 1;
            } else if (child->type == NODE_VARIABLE && strchr(child->text, '$')) {
                is_string = 1;
            } else if (child->type == NODE_FUNCTION_CALL && 
                       (child->token == TOK_CLEFT || child->token == TOK_CRIGHT || 
                        child->token == TOK_CMID || child->token == TOK_CCHR ||
                        child->token == TOK_CSTR)) {
                is_string = 1;
            }
            /* For NODE_EXPRESSION, check if it contains a string variable or function */
            else if (child->type == NODE_EXPRESSION && child->child_count > 0) {
                /* Recursively check children for string indicators */
                for (j = 0; j < child->child_count; j++) {
                    if (child->children[j]) {
                        if ((child->children[j]->type == NODE_VARIABLE && strchr(child->children[j]->text, '$')) ||
                            (child->children[j]->type == NODE_FUNCTION_CALL && 
                             (child->children[j]->token == TOK_CLEFT || child->children[j]->token == TOK_CRIGHT || 
                              child->children[j]->token == TOK_CMID || child->children[j]->token == TOK_CCHR ||
                              child->children[j]->token == TOK_CSTR))) {
                            is_string = 1;
                            break;
                        }
                    }
                }
            }
            
            if (is_string) {
                compiler_emit_no_operand(cs, OP_PRINT_STR);
            } else {
                compiler_emit_no_operand(cs, OP_PRINT_NUM);
            }
            
            has_trailing_separator = 0;
        }
    }
    
    /* Print newline unless trailing separator */
    if (!has_trailing_separator) {
        compiler_emit_no_operand(cs, OP_PRINT_NEWLINE);
    }
}

/* Helper to recursively find and emit INPUT opcodes for all variables */
static void find_input_variables(CompilerState *cs, ParseNode *node) {
    size_t i;
    int slot;
    
    if (!node) return;
    
    /* If this is a variable, emit INPUT opcode for it */
    if (node->type == NODE_VARIABLE && node->text) {
        slot = compiler_find_variable(cs, node->text);
        if (slot < 0) {
            slot = compiler_add_variable(cs, node->text, get_var_type(node->text));
        }
        
        if (strchr(node->text, '$')) {
            compiler_emit(cs, OP_INPUT_STR, slot);
        } else {
            compiler_emit(cs, OP_INPUT_NUM, slot);
        }
    }
    
    /* Always recurse into children */
    for (i = 0; i < node->child_count; i++) {
        find_input_variables(cs, node->children[i]);
    }
}

/* Helper to recursively find and emit INPUT_PROMPT for string constant */
static int find_input_prompt(CompilerState *cs, ParseNode *node) {
    size_t i;
    
    if (!node) return 0;
    
    /* If this is a string constant, emit INPUT_PROMPT */
    if (node->type == NODE_CONSTANT && node->token == TOK_STRING && node->text) {
        int str_idx = compiler_add_string(cs, node->text);
        compiler_emit(cs, OP_INPUT_PROMPT, str_idx);
        return 1;
    }
    
    /* Recurse into children */
    for (i = 0; i < node->child_count; i++) {
        if (find_input_prompt(cs, node->children[i])) {
            return 1;
        }
    }
    
    return 0;
}

/* Compile INPUT statement */
static void compile_input(CompilerState *cs, ParseNode *stmt) {
    size_t i;
    
    /* First pass: find and emit prompt if present */
    find_input_prompt(cs, stmt);
    
    /* Second pass: find all variables and emit INPUT opcodes */
    for (i = 0; i < stmt->child_count; i++) {
        find_input_variables(cs, stmt->children[i]);
    }
}

/* Compile IF-THEN statement */
static void compile_if_then(CompilerState *cs, ParseNode *stmt) {
    ParseNode *condition, *then_part, *else_part;
    uint32_t jump_if_false_offset, jump_skip_else_offset;
    int i;
    
    /* IF structure: [condition, THEN|empty, then_body, else_clause]
       condition is at children[0]
       THEN keyword (optional) at children[1] (might not be present)
       then_body is at children[2]  
       else_clause (optional) is at children[3] */
    if (stmt->child_count < 3) return;
    
    condition = stmt->children[0];
    then_part = stmt->children[2];
    else_part = (stmt->child_count >= 4) ? stmt->children[3] : NULL;
    
    /* Compile condition */
    compile_expression(cs, condition);
    
    /* Emit conditional jump - if false, skip to ELSE or end */
    jump_if_false_offset = cs->program->code_len;
    compiler_emit(cs, OP_JUMP_IF_FALSE, 0);  /* Placeholder */
    
    /* Compile THEN part - it's an EXPRESSION containing statements */
    if (then_part && then_part->child_count > 0) {
        for (i = 0; i < then_part->child_count; i++) {
            ParseNode *child = then_part->children[i];
            if (child->type == NODE_STATEMENT) {
                compile_statement(cs, child);
            } else if (child->type == NODE_EXPRESSION) {
                compile_expression(cs, child);
            } else if (child->type == NODE_CONSTANT && child->token == TOK_NUMBER) {
                /* THEN linenum - GOTO */
                uint16_t line = (uint16_t)child->value;
                int32_t offset = compiler_find_line_offset(cs, line);
                if (offset >= 0) {
                    compiler_emit(cs, OP_JUMP, offset);
                } else {
                    compiler_emit(cs, OP_JUMP, 0xFFFF);
                    compiler_add_jump_fixup(cs, cs->program->code_len - 1, line, JUMP_ABSOLUTE);
                }
            }
        }
    } else if (then_part && then_part->type == NODE_CONSTANT && then_part->token == TOK_NUMBER) {
        /* THEN linenum without wrapper */
        uint16_t line = (uint16_t)then_part->value;
        int32_t offset = compiler_find_line_offset(cs, line);
        if (offset >= 0) {
            compiler_emit(cs, OP_JUMP, offset);
        } else {
            compiler_emit(cs, OP_JUMP, 0xFFFF);
            compiler_add_jump_fixup(cs, cs->program->code_len - 1, line, JUMP_ABSOLUTE);
        }
    }
    
    /* If there's an ELSE clause, emit jump to skip it after THEN executes */
    if (else_part && else_part->child_count > 0) {
        jump_skip_else_offset = cs->program->code_len;
        compiler_emit(cs, OP_JUMP, 0);  /* Placeholder - will patch to skip ELSE */
        
        /* Patch the conditional jump to point here (start of ELSE) */
        cs->program->code[jump_if_false_offset].operand = cs->program->code_len;
        
        /* Compile ELSE part - starts with ELSE token, then action */
        /* else_part->children[0] is TOK_ELSE, children[1] is the IFA node */
        if (else_part->child_count >= 2) {
            ParseNode *else_action = else_part->children[1];
            
            /* IFA can be either a line number or a statement */
            /* Parse tree for line number: EXPRESSION -> EXPRESSION -> CONSTANT(TOK_NUMBER) */
            if (else_action && else_action->child_count >= 1 &&
                else_action->children[0] && else_action->children[0]->child_count >= 1 &&
                else_action->children[0]->children[0] &&
                else_action->children[0]->children[0]->type == NODE_CONSTANT && 
                else_action->children[0]->children[0]->token == TOK_NUMBER) {
                /* ELSE linenum - GOTO */
                uint16_t line = (uint16_t)else_action->children[0]->children[0]->value;
                int32_t offset = compiler_find_line_offset(cs, line);
                if (offset >= 0) {
                    compiler_emit(cs, OP_JUMP, offset);
                } else {
                    compiler_emit(cs, OP_JUMP, 0xFFFF);
                    compiler_add_jump_fixup(cs, cs->program->code_len - 1, line, JUMP_ABSOLUTE);
                }
            }
            /* Otherwise, IFA contains statement(s) */
            else {
                int32_t offset;
                if (else_action && else_action->child_count > 0) {
                    for (i = 0; i < else_action->child_count; i++) {
                        ParseNode *child = else_action->children[i];
                        if (child->type == NODE_STATEMENT) {
                            compile_statement(cs, child);
                        } else if (child->type == NODE_EXPRESSION) {
                            compile_expression(cs, child);
                        } else if (child->type == NODE_CONSTANT && child->token == TOK_NUMBER) {
                            /* Line number inside expression - GOTO */
                            uint16_t line = (uint16_t)child->value;
                            offset = compiler_find_line_offset(cs, line);
                            if (offset >= 0) {
                                compiler_emit(cs, OP_JUMP, offset);
                            } else {
                                compiler_emit(cs, OP_JUMP, 0xFFFF);
                                compiler_add_jump_fixup(cs, cs->program->code_len - 1, line, JUMP_ABSOLUTE);
                            }
                        }
                    }
                }
            }
        }
        
        /* Patch the jump-skip-else to point here (after ELSE) */
        cs->program->code[jump_skip_else_offset].operand = cs->program->code_len;
    } else {
        /* No ELSE clause - patch jump to skip THEN part to here */
        cs->program->code[jump_if_false_offset].operand = cs->program->code_len;
    }
}

/* Compile GOTO statement */
static void compile_goto(CompilerState *cs, ParseNode *stmt) {
    ParseNode *target;
    int32_t offset;
    
    if (stmt->child_count < 1) return;
    
    target = stmt->children[0];
    
    /* Check if it's a constant line number or variable */
    if (target->type == NODE_CONSTANT) {
        uint16_t line = (uint16_t)target->value;
        
        /* Try to find the line */
        offset = compiler_find_line_offset(cs, line);
        
        if (offset >= 0) {
            /* Line already compiled */
            compiler_emit(cs, OP_JUMP, offset);
        } else {
            /* Forward reference - need to fix up later */
            compiler_emit(cs, OP_JUMP, 0xFFFF);
            compiler_add_jump_fixup(cs, cs->program->code_len - 1, line, JUMP_ABSOLUTE);
        }
    } else {
        /* Variable GOTO - runtime lookup */
        compile_expression(cs, target);
        compiler_emit_no_operand(cs, OP_JUMP_LINE);
    }
}

/* Compile GOSUB statement */
static void compile_gosub(CompilerState *cs, ParseNode *stmt) {
    ParseNode *target;
    int32_t offset;
    
    if (stmt->child_count < 1) return;
    
    target = stmt->children[0];
    
    if (target->type == NODE_CONSTANT) {
        uint16_t line = (uint16_t)target->value;
        offset = compiler_find_line_offset(cs, line);
        
        if (offset >= 0) {
            compiler_emit(cs, OP_GOSUB, offset);
        } else {
            compiler_emit(cs, OP_GOSUB, 0xFFFF);
            compiler_add_jump_fixup(cs, cs->program->code_len - 1, line, JUMP_GOSUB);
        }
    } else {
        compile_expression(cs, target);
        compiler_emit_no_operand(cs, OP_GOSUB_LINE);
    }
}

/* Compile ON...GOTO/GOSUB statement */
static void compile_on(CompilerState *cs, ParseNode *stmt) {
    ParseNode *expr, *on1_node, *expl_node, *current;
    int is_gosub = 0;
    size_t count = 0;
    size_t i;
    uint32_t *line_numbers;
    size_t line_count = 0;
    
    /* ON structure: [expr, ON1, EXPL, EOS]
       expr is the index expression
       ON1 is either GOTO or GOSUB
       EXPL is the comma-separated list of line numbers */
    
    if (stmt->child_count < 3) return;
    
    expr = stmt->children[0];
    on1_node = stmt->children[1];
    expl_node = stmt->children[2];
    
    /* Determine if this is GOTO or GOSUB by checking the ON1 node */
    /* ON1 can be an expression containing the token */
    if (on1_node) {
        if (on1_node->token == TOK_CGS || on1_node->token == TOK_GOSUB_S) {
            is_gosub = 1;
        } else if (on1_node->type == NODE_EXPRESSION && on1_node->child_count > 0) {
            ParseNode *tok_node = on1_node->children[0];
            if (tok_node && (tok_node->token == TOK_CGS || tok_node->token == TOK_GOSUB_S)) {
                is_gosub = 1;
            }
        }
    }
    
    /* Count the number of targets in the expression list */
    current = expl_node;
    while (current && current->type == NODE_EXPRESSION) {
        if (current->child_count > 0) {
            count++;
            /* Navigate to next EXPL via EXPL1 */
            if (current->child_count > 1 && current->children[1]) {
                current = current->children[1];
                /* EXPL1 has comma at [0], next EXPL at [1] */
                if (current->child_count > 1) {
                    current = current->children[1];
                } else {
                    break;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }
    
    if (count == 0) return;
    
    /* Allocate array for line numbers */
    line_numbers = malloc(sizeof(uint32_t) * count);
    
    /* Extract line numbers from expression list */
    current = expl_node;
    i = 0;
    while (current && current->type == NODE_EXPRESSION && i < count) {
        if (current->child_count > 0 && current->children[0]) {
            ParseNode *line_expr = current->children[0];
            if (line_expr->type == NODE_CONSTANT) {
                line_numbers[i++] = (uint32_t)line_expr->value;
            }
        }
        
        /* Navigate to next */
        if (current->child_count > 1 && current->children[1]) {
            current = current->children[1];
            if (current->child_count > 1) {
                current = current->children[1];
            } else {
                break;
            }
        } else {
            break;
        }
    }
    line_count = i;
    
    /* Compile the index expression */
    compile_expression(cs, expr);
    
    /* Emit the ON instruction with count */
    if (is_gosub) {
        compiler_emit(cs, OP_ON_GOSUB, line_count);
    } else {
        compiler_emit(cs, OP_ON_GOTO, line_count);
    }
    
    /* Emit each target line number */
    for (i = 0; i < line_count; i++) {
        int32_t offset = compiler_find_line_offset(cs, line_numbers[i]);
        if (offset >= 0) {
            /* Line already compiled */
            compiler_emit_raw(cs, offset);
        } else {
            /* Forward reference */
            uint32_t fixup_pc = cs->program->code_len;
            compiler_emit_raw(cs, 0xFFFF);
            compiler_add_jump_fixup(cs, fixup_pc, line_numbers[i], 
                                    is_gosub ? JUMP_GOSUB : JUMP_ABSOLUTE);
        }
    }
    
    free(line_numbers);
}

/* Compile FOR statement */
static void compile_for(CompilerState *cs, ParseNode *stmt) {
    ParseNode *var_node, *start_expr, *limit_expr, *step_expr = NULL;
    int slot;
    
    /* FOR structure: [var, =, start, TO, limit, STEP?, step_val?, eos] */
    if (stmt->child_count < 5) return;
    
    var_node = stmt->children[0];      /* Variable */
    start_expr = stmt->children[2];    /* Start value (skip = at [1]) */
    limit_expr = stmt->children[4];    /* Limit value (skip TO at [3]) */
    
    /* Check for STEP - if child_count > 6 and child[5] is STEP token */
    if (stmt->child_count > 6) {
        step_expr = stmt->children[6];  /* Step value */
    }
    
    /* Navigate to variable */
    while (var_node && var_node->type == NODE_EXPRESSION && var_node->child_count > 0) {
        var_node = var_node->children[0];
    }
    
    if (!var_node || !var_node->text) return;
    
    slot = compiler_find_variable(cs, var_node->text);
    if (slot < 0) {
        slot = compiler_add_variable(cs, var_node->text, VAR_NUMERIC);
    }
    
    /* Compile: start, limit, step (in that order for stack) */
    compile_expression(cs, start_expr);
    compile_expression(cs, limit_expr);
    
    if (step_expr && step_expr->type != NODE_EXPRESSION) {
        compile_expression(cs, step_expr);
    } else {
        /* Default STEP 1 */
        compiler_emit(cs, OP_PUSH_CONST, compiler_add_const(cs, 1.0));
    }
    
    /* Emit FOR_INIT */
    compiler_emit(cs, OP_FOR_INIT, slot);
}

/* Compile NEXT statement */
/* Helper function to recursively emit FOR_NEXT for all variables in NEXTVL tree */
static int compile_next_variables(CompilerState *cs, ParseNode *node, const char **emitted_vars, int *emitted_count) {
    size_t i;
    int slot;
    int count = 0;
    int j;
    
    if (!node) return 0;
    
    /* Skip EOS tokens and commas */
    if (node->token == TOK_CCR || node->token == TOK_CEOS || node->token == TOK_CCOM) {
        return 0;
    }
    
    /* If this is a variable node, emit FOR_NEXT for it if we haven't seen this name before */
    if (node->type == NODE_VARIABLE && node->text) {
        /* Check if we've already emitted FOR_NEXT for a variable with this name */
        for (j = 0; j < *emitted_count; j++) {
            if (strcmp(emitted_vars[j], node->text) == 0) {
                return 0;  /* Already emitted this variable name */
            }
        }
        
        /* Mark this variable name as emitted */
        if (*emitted_count < 100) {  /* Arbitrary limit */
            emitted_vars[*emitted_count] = node->text;
            (*emitted_count)++;
        }
        
        slot = compiler_find_variable(cs, node->text);
        compiler_emit(cs, OP_FOR_NEXT, (slot < 0) ? 0xFFFF : slot);
        return 1;  /* Emitted one */
    }
    
    /* For non-variable nodes, recursively process children */
    for (i = 0; i < node->child_count; i++) {
        int child_emitted = compile_next_variables(cs, node->children[i], emitted_vars, emitted_count);
        count += child_emitted;
    }
    
    return count;
}

static void compile_next(CompilerState *cs, ParseNode *stmt) {
    size_t i;
    const char *emitted_vars[100];  /* Track emitted variable names to avoid duplicates */
    int emitted_count = 0;
    
    /* NEXT can have multiple variables separated by commas: NEXT Y,X */
    /* This is equivalent to NEXT Y : NEXT X */
    /* Process only the first non-EOS child which should be NEXTVL */
    for (i = 0; i < stmt->child_count; i++) {
        if (stmt->children[i] && 
            stmt->children[i]->token != TOK_CCR && 
            stmt->children[i]->token != TOK_CEOS) {
            if (compile_next_variables(cs, stmt->children[i], emitted_vars, &emitted_count)) {
                return;  /* Found and emitted variables */
            }
        }
    }
    
    /* No variables - emit NEXT with no variable (closes innermost loop) */
    compiler_emit(cs, OP_FOR_NEXT, 0xFFFF);
}

/* Compile DIM statement */
static void compile_dim(CompilerState *cs, ParseNode *stmt) {
    ParseNode *var_expr, *subscript_list;
    ParseNode *var_node, *dim1_node = NULL, *dim2_node = NULL;
    const char *name;
    int slot;
    double d1, d2;
    
    if (stmt->child_count < 2) return;
    
    var_expr = stmt->children[0];
    if (!var_expr || var_expr->child_count < 1) return;
    
    /* The subscript list is in var_expr->children[0] */
    subscript_list = var_expr->children[0];
    if (!subscript_list || subscript_list->child_count < 3) return;
    
    /* Structure: [var_name, left_paren, dim1, optionalComma_dim2, right_paren] */
    var_node = subscript_list->children[0];
    if (var_node && var_node->child_count > 0) {
        var_node = var_node->children[0];  /* Unwrap EXPRESSION */
    }
    if (!var_node || var_node->type != 2) return;  /* Must be VARIABLE */
    
    name = var_node->text;
    dim1_node = subscript_list->children[2];  /* First dimension at index 2 */
    
    /* Check if it's 2D (has 5 children instead of 3-4) */
    if (subscript_list->child_count >= 5) {
        /* 2D array: children[3] is EXPRESSION containing [comma, dim2] */
        ParseNode *dim2_part = subscript_list->children[3];
        if (dim2_part && dim2_part->child_count >= 2) {
            dim2_node = dim2_part->children[1];
        }
    }
    
    /* Extract dimension values */
    if (!dim1_node || dim1_node->type != 3) return;
    d1 = dim1_node->value;
    
    slot = compiler_find_variable(cs, name);
    if (slot < 0) {
        slot = compiler_add_variable(cs, name, dim2_node ? VAR_ARRAY_2D : VAR_ARRAY_1D);
    }
    
    /* Push dimensions onto stack before DIM opcode */
    if (dim2_node && dim2_node->type == 3) {
        /* 2D array - push row, col, then emit OP_DIM_2D */
        d2 = dim2_node->value;
        compiler_emit(cs, OP_PUSH_CONST, compiler_add_const(cs, d1));  /* row */
        compiler_emit(cs, OP_PUSH_CONST, compiler_add_const(cs, d2));  /* col */
        compiler_emit(cs, OP_DIM_2D, slot);
    } else {
        /* 1D array - push size, then emit OP_DIM_1D */
        compiler_emit(cs, OP_PUSH_CONST, compiler_add_const(cs, d1));
        compiler_emit(cs, OP_DIM_1D, slot);
    }
}

/* Compile DATA statement - collect into data pool */
/* Forward declaration for recursive helper */
static void extract_data_values_recursive(CompilerState *cs, ParseNode *node);

static void compile_data(CompilerState *cs, ParseNode *stmt) {
    ParseNode *data_list;
    
    /* With table-driven DATA parsing, structure is:
     * stmt -> children[0] = DATA_LIST
     *         children[1] = EOS
     * DATA_LIST contains DATA_VAL and DATA_TAIL nodes
     * DATA_TAIL recursively contains [comma, DATA_VAL, DATA_TAIL, ...]
     */
    
    if (stmt->child_count == 0) return;
    
    /* Get the DATA_LIST node */
    data_list = stmt->children[0];
    
    /* Recursively extract all DATA values */
    extract_data_values_recursive(cs, data_list);
    
    /* DATA statements don't generate runtime code */
}

/* Helper function to recursively extract DATA values from parse tree */
static void extract_data_values_recursive(CompilerState *cs, ParseNode *node) {
    size_t i;
    DataEntry entry;
    
    if (!node) return;
    
    /* Debug: print node info */
    /*
    fprintf(stderr, "extract_data: node type=%d, token=%d, child_count=%d, text=%s, value=%g\n",
            node->type, node->token, (int)node->child_count,
            node->text ? node->text : "(null)", node->value);
    */
    
    /* Check for null DATA value pattern: DATA_TAIL with 2 children (comma + tail) */
    /* This represents "DATA_TAIL = , DATA_TAIL" alternative (null value) */
    if (node->type == NODE_EXPRESSION && node->child_count == 2 &&
        node->children[0]->type == NODE_OPERATOR && node->children[0]->token == TOK_CCOM &&
        node->children[1]->type == NODE_EXPRESSION) {
        /* This is a null DATA value (e.g., the middle value in "DATA 1,,3") */
        /*
        fprintf(stderr, "  --> Detected NULL data item\n");
        */
        entry.type = DATA_NULL;
        entry.value.numeric_idx = 0;  /* Not used for NULL type */
        
        if (cs->program->data_count >= cs->program->data_capacity) {
            cs->program->data_capacity *= 2;
            cs->program->data_entries = realloc(cs->program->data_entries,
                sizeof(DataEntry) * cs->program->data_capacity);
        }
        cs->program->data_entries[cs->program->data_count++] = entry;
        /*
        fprintf(stderr, "  --> Added NULL data entry, data_count now=%d\n",
                (int)cs->program->data_count);
        */
        
        /* Continue processing the tail */
        extract_data_values_recursive(cs, node->children[1]);
        return;
    }
    
    /* Check for empty/null DATA value (epsilon production) - no longer used */
    /* The new grammar handles nulls via DATA_TAIL = , DATA_TAIL pattern above */
    /* Keep this check for safety but it should never match now */
    if (node->child_count == 0 && node->text == NULL && node->type == NODE_EXPRESSION) {
        /* Epsilon terminator - don't add as data */
        return;
    }
    
    /* If this is a constant or identifier, add it to the data pool */
    if (node->type == NODE_CONSTANT || node->type == NODE_VARIABLE) {
        if (node->token == TOK_STRING) {
            /* String data */
            if (cs->program->data_string_count >= cs->program->data_string_capacity) {
                cs->program->data_string_capacity *= 2;
                cs->program->data_string_pool = realloc(cs->program->data_string_pool,
                    sizeof(char*) * cs->program->data_string_capacity);
            }
            cs->program->data_string_pool[cs->program->data_string_count] = my_strdup(node->text);
            
            entry.type = DATA_STRING;
            entry.value.string_idx = cs->program->data_string_count++;
        } else if (node->token == TOK_IDENT && node->text) {
            /* Identifier in DATA - store as string (per Microsoft BASIC spec) */
            if (cs->program->data_string_count >= cs->program->data_string_capacity) {
                cs->program->data_string_capacity *= 2;
                cs->program->data_string_pool = realloc(cs->program->data_string_pool,
                    sizeof(char*) * cs->program->data_string_capacity);
            }
            cs->program->data_string_pool[cs->program->data_string_count] = my_strdup(node->text);
            
            entry.type = DATA_STRING;
            entry.value.string_idx = cs->program->data_string_count++;
        } else {
            /* Numeric data */
            if (cs->program->data_numeric_count >= cs->program->data_numeric_capacity) {
                cs->program->data_numeric_capacity *= 2;
                cs->program->data_numeric_pool = realloc(cs->program->data_numeric_pool,
                    sizeof(double) * cs->program->data_numeric_capacity);
            }
            
            if (node->token == TOK_NUMBER) {
                cs->program->data_numeric_pool[cs->program->data_numeric_count] = node->value;
            } else {
                cs->program->data_numeric_pool[cs->program->data_numeric_count] = 0;
            }
            
            entry.type = DATA_NUMERIC;
            entry.value.numeric_idx = cs->program->data_numeric_count++;
        }
        
        /* Add to data entries */
        if (cs->program->data_count >= cs->program->data_capacity) {
            cs->program->data_capacity *= 2;
            cs->program->data_entries = realloc(cs->program->data_entries,
                sizeof(DataEntry) * cs->program->data_capacity);
        }
        cs->program->data_entries[cs->program->data_count++] = entry;
        /*
        fprintf(stderr, "  --> Added data entry: type=%d, idx=%d, data_count now=%d\n",
                entry.type, entry.value.numeric_idx, (int)cs->program->data_count);
        */
        return;
    }
    
    /* Otherwise, recursively process all children */
    for (i = 0; i < node->child_count; i++) {
        extract_data_values_recursive(cs, node->children[i]);
    }
}

/* Compile READ statement */
/* Forward declaration for recursive helper */
static void compile_read_recursive(CompilerState *cs, ParseNode *node);

static void compile_read(CompilerState *cs, ParseNode *stmt) {
    ParseNode *nsvrl;
    
    /* READ statement structure:
     * stmt -> children[0] = NSVRL (variable list)
     *         children[1] = EOS
     * NSVRL is recursive like DATA_TAIL
     */
    
    if (stmt->child_count == 0) return;
    
    /* Get the NSVRL node (variable list) */
    nsvrl = stmt->children[0];
    
    /* Recursively process all variables */
    compile_read_recursive(cs, nsvrl);
}

/* Helper to recursively compile READ variables from parse tree */
static void compile_read_recursive(CompilerState *cs, ParseNode *node) {
    size_t i;
    
    if (!node) return;
    
    /* If this is a variable node, emit READ instruction */
    if ((node->type == NODE_VARIABLE || node->type == NODE_EXPRESSION) && node->text != NULL) {
        int slot;
        const char *var_name = node->text;
        
        slot = compiler_find_variable(cs, var_name);
        if (slot < 0) {
            slot = compiler_add_variable(cs, var_name, get_var_type(var_name));
        }
        
        if (strchr(var_name, '$')) {
            compiler_emit(cs, OP_DATA_READ_STR, slot);
        } else {
            compiler_emit(cs, OP_DATA_READ_NUM, slot);
        }
        return;
    }
    
    /* Otherwise, recursively process all children */
    for (i = 0; i < node->child_count; i++) {
        compile_read_recursive(cs, node->children[i]);
    }
}

/* Simple statement compilers (emit single opcode) */
static void compile_end_stmt(CompilerState *cs, ParseNode *stmt) {
    (void)stmt;
    compiler_emit_no_operand(cs, OP_END);
}

static void compile_stop_stmt(CompilerState *cs, ParseNode *stmt) {
    (void)stmt;
    compiler_emit_no_operand(cs, OP_STOP);
}

static void compile_return_stmt(CompilerState *cs, ParseNode *stmt) {
    (void)stmt;
    compiler_emit_no_operand(cs, OP_RETURN);
}

static void compile_clr_stmt(CompilerState *cs, ParseNode *stmt) {
    (void)stmt;
    compiler_emit_no_operand(cs, OP_CLR);
}

static void compile_pop_stmt(CompilerState *cs, ParseNode *stmt) {
    (void)stmt;
    compiler_emit_no_operand(cs, OP_POP_GOSUB);
}

static void compile_deg_stmt(CompilerState *cs, ParseNode *stmt) {
    (void)stmt;
    compiler_emit_no_operand(cs, OP_DEG);
}

static void compile_rad_stmt(CompilerState *cs, ParseNode *stmt) {
    (void)stmt;
    compiler_emit_no_operand(cs, OP_RAD);
}

static void compile_noop(CompilerState *cs, ParseNode *stmt) {
    /* No-op for statements like CLEAR, CLS, DEFxxx that we don't implement */
    (void)cs;
    (void)stmt;
}

/* Complex statement compilers */
static void compile_let_stmt(CompilerState *cs, ParseNode *stmt) {
    /* Assignment: children are [var_expr, eq_op, value_expr, eos] */
    if (stmt->child_count >= 3) {
        ParseNode *var_expr = stmt->children[0];
        ParseNode *value_expr = stmt->children[2];  /* Skip the '=' operator at child[1] */
        ParseNode *actual_var;
        int slot;
        
        /* Check if this is an array assignment (has subscripts) */
        if (var_expr->type == NODE_EXPRESSION && var_expr->child_count == 2) {
            ParseNode *var_part = var_expr->children[0];
            ParseNode *subscript_expr = var_expr->children[1];
            ParseNode *subscript1 = NULL;
            ParseNode *subscript2 = NULL;
            
            /* Unwrap variable */
            while (var_part && var_part->type == NODE_EXPRESSION && var_part->child_count > 0) {
                var_part = var_part->children[0];
            }
            
            /* Extract subscripts */
            if (subscript_expr && subscript_expr->child_count >= 2) {
                subscript1 = subscript_expr->children[1];
                /* For 2D arrays: child[2] is EXPRESSION containing [comma, sub2] */
                if (subscript_expr->child_count >= 4) {
                    ParseNode *middle = subscript_expr->children[2];
                    if (middle && middle->type == NODE_EXPRESSION && middle->child_count >= 2) {
                        subscript2 = middle->children[1];
                    }
                }
            }
            
            if (var_part && var_part->type == NODE_VARIABLE && var_part->text && subscript1) {
                /* Array assignment */
                int is_string = strchr(var_part->text, '$') != NULL;
                
                slot = compiler_find_variable(cs, var_part->text);
                if (slot < 0) {
                    slot = compiler_add_variable(cs, var_part->text, get_var_type(var_part->text));
                }
                
                /* Compile subscripts and value */
                compile_expression(cs, subscript1);
                if (subscript2) {
                    compile_expression(cs, subscript2);
                }
                compile_expression(cs, value_expr);
                
                /* Emit array store */
                if (subscript2) {
                    compiler_emit(cs, is_string ? OP_STR_ARRAY_SET_2D : OP_ARRAY_SET_2D, slot);
                } else {
                    compiler_emit(cs, is_string ? OP_STR_ARRAY_SET_1D : OP_ARRAY_SET_1D, slot);
                }
                return;
            }
        }
        
        /* Simple variable assignment */
        actual_var = var_expr;
        while (actual_var && actual_var->type == NODE_EXPRESSION && actual_var->child_count > 0) {
            actual_var = actual_var->children[0];
        }
        
        if (!actual_var || actual_var->type != NODE_VARIABLE || !actual_var->text) {
            return;
        }
        
        slot = compiler_find_variable(cs, actual_var->text);
        if (slot < 0) {
            slot = compiler_add_variable(cs, actual_var->text, get_var_type(actual_var->text));
        }
        
        compile_expression(cs, value_expr);
        
        if (get_var_type(actual_var->text) == VAR_STRING) {
            compiler_emit(cs, OP_STR_POP_VAR, slot);
        } else {
            compiler_emit(cs, OP_POP_VAR, slot);
        }
    }
}

static void compile_randomize_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count == 0 ||
        (stmt->children[0]->type == NODE_EXPRESSION &&
         stmt->children[0]->child_count == 1 &&
         (stmt->children[0]->children[0]->token == TOK_CCR ||
          stmt->children[0]->children[0]->token == TOK_CEOS))) {
        fprintf(stderr, "Error: RANDOMIZE requires an argument\n");
        return;
    }
    
    compile_expression(cs, stmt->children[0]);
    compiler_emit_no_operand(cs, OP_RANDOMIZE);
}

static void compile_trap_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 1) {
        ParseNode *target = stmt->children[0];
        if (target->type == NODE_CONSTANT) {
            uint16_t line = (uint16_t)target->value;
            int32_t offset = compiler_find_line_offset(cs, line);
            if (offset >= 0) {
                compiler_emit(cs, OP_TRAP, offset);
            } else {
                compiler_emit(cs, OP_TRAP, 0xFFFF);
                compiler_add_jump_fixup(cs, cs->program->code_len - 1, line, JUMP_ABSOLUTE);
            }
        }
    }
}

static void compile_restore_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count > 0) {
        compile_expression(cs, stmt->children[0]);
        compiler_emit_no_operand(cs, OP_RESTORE_LINE);
    } else {
        compiler_emit(cs, OP_RESTORE, 0);
    }
}

/* I/O statement compilers */
static void compile_open_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 7) {
        compile_expression(cs, stmt->children[1]); /* channel */
        compile_expression(cs, stmt->children[3]); /* mode */
        compile_expression(cs, stmt->children[5]); /* aux */
        compile_expression(cs, stmt->children[7]); /* filename */
        compiler_emit_no_operand(cs, OP_OPEN);
    }
}

static void compile_close_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 2) {
        compile_expression(cs, stmt->children[1]);
        compiler_emit_no_operand(cs, OP_CLOSE);
    } else {
        compiler_emit(cs, OP_PUSH_CONST, 0);
        compiler_emit_no_operand(cs, OP_CLOSE);
    }
}

static void compile_put_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 3) {
        compile_expression(cs, stmt->children[1]); /* channel */
        compile_expression(cs, stmt->children[3]); /* value */
        compiler_emit_no_operand(cs, OP_PUT);
    }
}

static void compile_get_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 3) {
        ParseNode *var = stmt->children[3];
        ParseNode *var_node;
        int slot;
        
        compile_expression(cs, stmt->children[1]);
        compiler_emit_no_operand(cs, OP_GET);
        
        var_node = var;
        while (var_node && var_node->type == NODE_EXPRESSION && var_node->child_count > 0) {
            var_node = var_node->children[0];
        }
        
        if (!var_node || !var_node->text) return;
        
        slot = compiler_find_variable(cs, var_node->text);
        if (slot < 0) {
            slot = compiler_add_variable(cs, var_node->text, get_var_type(var_node->text));
        }
        compiler_emit(cs, OP_POP_VAR, slot);
    }
}

static void compile_note_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 5) {
        ParseNode *sec_var = stmt->children[3];
        ParseNode *byte_var = stmt->children[5];
        ParseNode *sec_node, *byte_node;
        int sec_slot, byte_slot;
        
        compile_expression(cs, stmt->children[1]);
        compiler_emit_no_operand(cs, OP_NOTE);
        
        sec_node = sec_var;
        while (sec_node && sec_node->type == NODE_EXPRESSION && sec_node->child_count > 0) {
            sec_node = sec_node->children[0];
        }
        byte_node = byte_var;
        while (byte_node && byte_node->type == NODE_EXPRESSION && byte_node->child_count > 0) {
            byte_node = byte_node->children[0];
        }
        
        if (!sec_node || !sec_node->text || !byte_node || !byte_node->text) return;
        
        sec_slot = compiler_find_variable(cs, sec_node->text);
        if (sec_slot < 0) {
            sec_slot = compiler_add_variable(cs, sec_node->text, get_var_type(sec_node->text));
        }
        byte_slot = compiler_find_variable(cs, byte_node->text);
        if (byte_slot < 0) {
            byte_slot = compiler_add_variable(cs, byte_node->text, get_var_type(byte_node->text));
        }
        
        compiler_emit(cs, OP_POP_VAR, byte_slot);
        compiler_emit(cs, OP_POP_VAR, sec_slot);
    }
}

static void compile_point_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 5) {
        compile_expression(cs, stmt->children[1]); /* channel */
        compile_expression(cs, stmt->children[3]); /* sector */
        compile_expression(cs, stmt->children[5]); /* byte */
        compiler_emit_no_operand(cs, OP_POINT);
    }
}

static void compile_status_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 3) {
        ParseNode *var = stmt->children[3];
        ParseNode *var_node;
        int slot;
        
        compile_expression(cs, stmt->children[1]);
        compiler_emit_no_operand(cs, OP_STATUS);
        
        var_node = var;
        while (var_node && var_node->type == NODE_EXPRESSION && var_node->child_count > 0) {
            var_node = var_node->children[0];
        }
        
        if (!var_node || !var_node->text) return;
        
        slot = compiler_find_variable(cs, var_node->text);
        if (slot < 0) {
            slot = compiler_add_variable(cs, var_node->text, get_var_type(var_node->text));
        }
        compiler_emit(cs, OP_POP_VAR, slot);
    }
}

static void compile_xio_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 10) {
        compile_expression(cs, stmt->children[0]); /* command */
        compile_expression(cs, stmt->children[3]); /* channel */
        compile_expression(cs, stmt->children[5]); /* aux1 */
        compile_expression(cs, stmt->children[7]); /* aux2 */
        compile_expression(cs, stmt->children[9]); /* device/filename */
        compiler_emit_no_operand(cs, OP_XIO);
    }
}

static void compile_poke_stmt(CompilerState *cs, ParseNode *stmt) {
    /* POKE address, value
     * Parse tree: POKE <EXP> , <EXP> <EOS>
     * children[0] = address expression
     * children[1] = comma
     * children[2] = value expression
     */
    if (stmt->child_count >= 3) {
        compile_expression(cs, stmt->children[0]); /* address */
        compile_expression(cs, stmt->children[2]); /* value */
        compiler_emit_no_operand(cs, OP_POKE);
    }
}

/* Compilation dispatch table */
static const CompilationEntry compilation_table[] = {
    {TOK_IDENT, compile_let_stmt},
    {TOK_LET, compile_let_stmt},
    {TOK_PRINT, compile_print},
    {TOK_QUESTION, compile_print},
    {TOK_INPUT, compile_input},
    {TOK_IF, compile_if_then},
    {TOK_GOTO, compile_goto},
    {TOK_CGTO, compile_goto},
    {TOK_GOSUB_S, compile_gosub},
    {TOK_CGS, compile_gosub},
    {TOK_ON, compile_on},
    {TOK_RETURN, compile_return_stmt},
    {TOK_CLR, compile_clr_stmt},
    {TOK_CLEAR, compile_noop},
    {TOK_DEFINT, compile_noop},
    {TOK_DEFLNG, compile_noop},
    {TOK_DEFSNG, compile_noop},
    {TOK_DEFDBL, compile_noop},
    {TOK_DEFSTR, compile_noop},
    {TOK_CLS, compile_noop},
    {TOK_POP, compile_pop_stmt},
    {TOK_FOR, compile_for},
    {TOK_NEXT, compile_next},
    {TOK_END, compile_end_stmt},
    {TOK_DEG, compile_deg_stmt},
    {TOK_RAD, compile_rad_stmt},
    {TOK_RANDOMIZE, compile_randomize_stmt},
    {TOK_TRAP, compile_trap_stmt},
    {TOK_XIO, compile_xio_stmt},
    {TOK_STOP, compile_stop_stmt},
    {TOK_DIM, compile_dim},
    {TOK_DATA, compile_data},
    {TOK_READ, compile_read},
    {TOK_RESTORE, compile_restore_stmt},
    {TOK_OPEN, compile_open_stmt},
    {TOK_CLOSE, compile_close_stmt},
    {TOK_PUT, compile_put_stmt},
    {TOK_GET, compile_get_stmt},
    {TOK_NOTE, compile_note_stmt},
    {TOK_POINT, compile_point_stmt},
    {TOK_STATUS, compile_status_stmt},
    {TOK_POKE, compile_poke_stmt},
    {0, NULL}  /* Sentinel */
};

/* 
 * TABLE-DRIVEN STATEMENT COMPILATION
 * 
 * This function uses a dispatch table to compile statements instead of a large
 * switch statement. Each statement type is mapped to its specific compiler function
 * in the compilation_table array above.
 * 
 * Benefits:
 * - More maintainable: each statement has its own focused function
 * - Easier to extend: just add new entry to table
 * - Better code organization: related logic is grouped
 * - Consistent with table-driven parser design
 */
static void compile_statement(CompilerState *cs, ParseNode *stmt) {
    int i;
    
    if (!stmt) return;
    
    /* Handle expression statements (assignments) */
    if (stmt->type != NODE_STATEMENT) {
        if (stmt->type == NODE_OPERATOR && stmt->token == TOK_CEQ) {
            compile_assignment(cs, stmt);
        }
        return;
    }
    
    /* Look up statement compiler in dispatch table */
    for (i = 0; compilation_table[i].compiler != NULL; i++) {
        if (compilation_table[i].token == stmt->token) {
            compilation_table[i].compiler(cs, stmt);
            return;
        }
    }
    
    /* Unknown or unimplemented statement - silently ignore */
}

/* Main compilation entry point */
CompiledProgram* compiler_compile(ParseNode *root) {
    CompilerState *cs;
    CompiledProgram *prog;
    size_t i;
    
    if (!root) return NULL;
    
    cs = compiler_state_new();
    if (!cs) return NULL;
    
    /* Phase 1: Discover all variables */
    discover_variables_in_tree(cs, root);
    
    /* Phase 2 & 3: Compile each line */
    for (i = 0; i < root->child_count; i++) {
        ParseNode *stmt = root->children[i];
        uint16_t line_num;
        
        if (!stmt || stmt->type != NODE_STATEMENT) continue;
        
        /* Get line number */
        line_num = (uint16_t)stmt->line_number;
        cs->current_line = line_num;
        
        /* Record line position */
        compiler_add_line_mapping(cs, line_num, cs->program->code_len);
        
        /* Compile the statement */
        compile_statement(cs, stmt);
    }
    
    /* Phase 4: Resolve jump fixups */
    compiler_resolve_jumps(cs);
    
    if (cs->has_error) {
        fprintf(stderr, "Compilation error: %s\n", cs->error_msg);
        compiled_program_free(cs->program);
        compiler_state_free(cs);
        return NULL;
    }
    
    /* Return compiled program */
    prog = cs->program;
    cs->program = NULL;  /* Don't free it */
    compiler_state_free(cs);
    
    return prog;
}
