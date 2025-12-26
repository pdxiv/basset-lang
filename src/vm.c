/* vm.c - Virtual machine executor */
#define _POSIX_C_SOURCE 200112L  /* Enable snprintf */
#include "vm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* K&R C compatible strdup */
static char* my_strdup(const char *s) {
    char *d;
    if (!s) return NULL;
    d = malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}

/* Validate numeric input string */
static int is_valid_numeric_input(const char *str) {
    const char *p = str;
    int has_digit = 0;
    int has_decimal = 0;
    int has_e = 0;
    
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') return 0;
    if (*p == '+' || *p == '-') p++;
    
    while (*p) {
        if (*p >= '0' && *p <= '9') {
            has_digit = 1;
            p++;
        } else if (*p == '.' && !has_decimal && !has_e) {
            has_decimal = 1;
            p++;
        } else if ((*p == 'e' || *p == 'E') && !has_e && has_digit) {
            has_e = 1;
            p++;
            if (*p == '+' || *p == '-') p++;
            if (*p < '0' || *p > '9') return 0;
        } else if (*p == ' ' || *p == '\t') {
            p++;
        } else {
            return 0;
        }
    }
    
    return has_digit;
}

/* Initialize VM */
VMState* vm_init(CompiledProgram *program) {
    size_t i;
    VMState *vm;
    
    if (!program) return NULL;
    
    vm = calloc(1, sizeof(VMState));
    if (!vm) return NULL;
    
    /* Initialize stacks */
    vm->stack_capacity = 256;
    vm->stack = malloc(sizeof(double) * vm->stack_capacity);
    vm->stack_top = 0;
    
    vm->str_stack_capacity = 256;
    vm->str_stack = malloc(sizeof(char*) * vm->str_stack_capacity);
    vm->str_stack_top = 0;
    
    vm->call_capacity = 64;
    vm->call_stack = malloc(sizeof(uint32_t) * vm->call_capacity);
    vm->call_top = 0;
    
    vm->for_capacity = 32;
    vm->for_stack = malloc(sizeof(ForLoopState) * vm->for_capacity);
    vm->for_top = 0;
    
    /* Initialize variables */
    vm->var_capacity = program->var_count;
    vm->num_vars = calloc(vm->var_capacity, sizeof(double));
    vm->str_vars = calloc(vm->var_capacity, sizeof(char*));
    vm->arrays = calloc(vm->var_capacity, sizeof(ArrayData));
    
    /* Initialize string variables and array flags */
    for (i = 0; i < vm->var_capacity; i++) {
        vm->str_vars[i] = my_strdup("");
        /* Set is_string flag for arrays based on variable type */
        if (program->var_table[i].type == VAR_STRING ||
            (program->var_table[i].name && strchr(program->var_table[i].name, '$'))) {
            vm->arrays[i].is_string = 1;
        } else {
            vm->arrays[i].is_string = 0;
        }
    }
    
    /* Initialize file handles */
    for (i = 0; i < 8; i++) {
        vm->file_handles[i] = NULL;
        vm->file_status[i] = 0;
        vm->file_positions[i] = 0;
    }
    
    /* Reset state */
    vm->pc = 0;
    vm->running = 1;
    vm->data_pointer = 0;
    vm->trap_line = 0;
    vm->trap_enabled = 0;
    vm->trap_triggered = 0;
    vm->print_needs_newline = 0;
    vm->print_last_char = '\0';
    vm->print_after_tab = 0;
    vm->print_channel = 0;  /* Default to stdout */
    vm->print_column = 1;   /* Start at column 1 */
    vm->print_width = 80;   /* Default 80-column width */
    vm->deg_mode = 0;  /* Default to radian mode */
    
    /* Initialize RNG with a default seed - Microsoft BASIC compatible */
    vm->rnd_seed = 327680;  /* Default seed used by many BASIC implementations */
    vm->last_rnd = 0.5;     /* Initial RND(0) value */
    
    /* Initialize INPUT buffer state */
    vm->input_buffer[0] = '\0';
    vm->input_ptr = NULL;
    vm->input_available = 0;
    
    /* Allocate simulated memory buffer (64KB) for PEEK/POKE */
    vm->memory = calloc(65536, 1);
    if (!vm->memory) {
        free(vm->stack);
        free(vm->str_stack);
        free(vm->call_stack);
        free(vm->for_stack);
        free(vm->num_vars);
        free(vm->str_vars);
        free(vm->arrays);
        free(vm);
        return NULL;
    }
    
    vm->program = program;
    
    return vm;
}

/* Free VM */
void vm_free(VMState *vm) {
    size_t i;
    
    if (!vm) return;
    
    if (vm->stack) free(vm->stack);
    
    if (vm->str_stack) {
        for (i = 0; i < vm->str_stack_top; i++) {
            if (vm->str_stack[i]) free(vm->str_stack[i]);
        }
        free(vm->str_stack);
    }
    
    if (vm->call_stack) free(vm->call_stack);
    if (vm->for_stack) free(vm->for_stack);
    
    if (vm->num_vars) free(vm->num_vars);
    
    if (vm->str_vars) {
        for (i = 0; i < vm->var_capacity; i++) {
            if (vm->str_vars[i]) free(vm->str_vars[i]);
        }
        free(vm->str_vars);
    }
    
    if (vm->arrays) {
        for (i = 0; i < vm->var_capacity; i++) {
            if (vm->arrays[i].is_string && vm->arrays[i].u.str_data) {
                size_t j, total_size;
                total_size = vm->arrays[i].dim1 * (vm->arrays[i].dim2 ? vm->arrays[i].dim2 : 1);
                for (j = 0; j < total_size; j++) {
                    if (vm->arrays[i].u.str_data[j]) free(vm->arrays[i].u.str_data[j]);
                }
                free(vm->arrays[i].u.str_data);
            } else if (!vm->arrays[i].is_string && vm->arrays[i].u.data) {
                free(vm->arrays[i].u.data);
            }
        }
        free(vm->arrays);
    }
    
    /* Close any open file handles */
    for (i = 0; i < 8; i++) {
        if (vm->file_handles[i]) {
            fclose(vm->file_handles[i]);
            vm->file_handles[i] = NULL;
        }
    }
    
    /* Free simulated memory buffer */
    if (vm->memory) {
        free(vm->memory);
    }
    
    free(vm);
}

/* Stack operations */
void vm_push(VMState *vm, double value) {
    if (vm->stack_top >= vm->stack_capacity) {
        vm->stack_capacity *= 2;
        vm->stack = realloc(vm->stack, sizeof(double) * vm->stack_capacity);
    }
    vm->stack[vm->stack_top++] = value;
}

double vm_pop(VMState *vm) {
    if (vm->stack_top == 0) {
        vm_error(vm, "STACK UNDERFLOW");
        return 0.0;
    }
    return vm->stack[--vm->stack_top];
}

void vm_str_push(VMState *vm, const char *str) {
    if (vm->str_stack_top >= vm->str_stack_capacity) {
        vm->str_stack_capacity *= 2;
        vm->str_stack = realloc(vm->str_stack, sizeof(char*) * vm->str_stack_capacity);
    }
    vm->str_stack[vm->str_stack_top++] = my_strdup(str ? str : "");
}

char* vm_str_pop(VMState *vm) {
    if (vm->str_stack_top == 0) {
        vm_error(vm, "STRING STACK UNDERFLOW");
        return my_strdup("");
    }
    return vm->str_stack[--vm->str_stack_top];
}

/* Call stack operations */
void vm_call_push(VMState *vm, uint32_t return_addr) {
    if (vm->call_top >= vm->call_capacity) {
        vm->call_capacity *= 2;
        vm->call_stack = realloc(vm->call_stack, sizeof(uint32_t) * vm->call_capacity);
    }
    vm->call_stack[vm->call_top++] = return_addr;
}

uint32_t vm_call_pop(VMState *vm) {
    if (vm->call_top == 0) {
        vm_error(vm, "RETURN WITHOUT GOSUB");
        return 0;
    }
    return vm->call_stack[--vm->call_top];
}

/* FOR stack operations */
void vm_for_push(VMState *vm, ForLoopState state) {
    if (vm->for_top >= vm->for_capacity) {
        vm->for_capacity *= 2;
        vm->for_stack = realloc(vm->for_stack, sizeof(ForLoopState) * vm->for_capacity);
    }
    vm->for_stack[vm->for_top++] = state;
}

ForLoopState* vm_for_top(VMState *vm) {
    if (vm->for_top == 0) {
        vm_error(vm, "NEXT WITHOUT FOR");
        return NULL;
    }
    return &vm->for_stack[vm->for_top - 1];
}

void vm_for_pop(VMState *vm) {
    if (vm->for_top > 0) {
        vm->for_top--;
    }
}

/* Get output FILE* based on current print channel */
static FILE* vm_get_output_file(VMState *vm) {
    if (vm->print_channel == 0) {
        return stdout;
    }
    if (vm->print_channel >= 1 && vm->print_channel <= 7) {
        FILE *fh = vm->file_handles[vm->print_channel];
        if (fh) {
            return fh;
        }
        /* File not open, fall back to stdout and show error */
        fprintf(stderr, "WARNING: File channel %d not open, using stdout\n", vm->print_channel);
        return stdout;
    }
    /* Invalid channel, use stdout */
    fprintf(stderr, "WARNING: Invalid print channel %d, using stdout\n", vm->print_channel);
    return stdout;
}

/* Get next comma-separated value from INPUT buffer */
static char* get_next_input_value(VMState *vm) {
    static char value_buffer[256];
    char *start, *end;
    size_t val_len;
    
    /* If no input available, read a new line */
    if (!vm->input_available) {
        size_t len;
        printf("? ");
        fflush(stdout);
        
        if (!fgets(vm->input_buffer, sizeof(vm->input_buffer), stdin)) {
            value_buffer[0] = '\0';
            return value_buffer;
        }
        
        /* Remove trailing newline */
        len = strlen(vm->input_buffer);
        if (len > 0 && vm->input_buffer[len-1] == '\n') {
            vm->input_buffer[len-1] = '\0';
        }
        
        /* Echo the input line (for piped/redirected input) */
        printf("%s\n", vm->input_buffer);
        
        vm->input_ptr = vm->input_buffer;
        vm->input_available = 1;
    }
    
    /* Skip leading whitespace */
    while (*vm->input_ptr == ' ' || *vm->input_ptr == '\t') {
        vm->input_ptr++;
    }
    
    /* Check if we've reached end of input */
    if (*vm->input_ptr == '\0') {
        vm->input_available = 0;
        value_buffer[0] = '\0';
        return value_buffer;
    }
    
    start = vm->input_ptr;
    
    /* Handle quoted strings */
    if (*start == '"') {
        start++;
        end = start;
        while (*end != '\0' && *end != '"') {
            end++;
        }
        
        val_len = end - start;
        if (val_len >= sizeof(value_buffer)) {
            val_len = sizeof(value_buffer) - 1;
        }
        strncpy(value_buffer, start, val_len);
        value_buffer[val_len] = '\0';
        
        if (*end == '"') {
            vm->input_ptr = end + 1;
            /* Skip comma if present after closing quote */
            if (*vm->input_ptr == ',') {
                vm->input_ptr++;
            } else if (*vm->input_ptr == '\0') {
                vm->input_available = 0;
            }
        } else {
            vm->input_ptr = end;
            vm->input_available = 0;
        }
    } else {
        char *trim_end;
        /* Find next comma or end of string */
        end = start;
        while (*end != '\0' && *end != ',') {
            end++;
        }
        
        /* Trim trailing whitespace from value */
        trim_end = end;
        while (trim_end > start && (*(trim_end-1) == ' ' || *(trim_end-1) == '\t')) {
            trim_end--;
        }
        
        /* Copy value to buffer */
        val_len = trim_end - start;
        if (val_len >= sizeof(value_buffer)) {
            val_len = sizeof(value_buffer) - 1;
        }
        strncpy(value_buffer, start, val_len);
        value_buffer[val_len] = '\0';
        
        /* Advance pointer past comma if present */
        if (*end == ',') {
            vm->input_ptr = end + 1;
            /* Still more values available */
        } else {
            vm->input_ptr = end;
            vm->input_available = 0;  /* Last value on line */
        }
    }
    
    return value_buffer;
}

/* Error handling */
void vm_error(VMState *vm, const char *message) {
    /* Check if TRAP is enabled */
    if (vm->trap_enabled && vm->trap_line > 0) {
        /* Clear stacks to avoid corruption */
        vm->stack_top = 0;
        vm->str_stack_top = 0;
        
        /* Jump to trap handler instead of halting */
        vm->pc = vm->trap_line;
        vm->trap_enabled = 0;  /* Disable trap to avoid infinite loops */
        vm->trap_triggered = 1;  /* Flag that we jumped */
        return;
    }
    
    /* No trap - print error and halt */
    fprintf(stderr, "ERROR - %s\n", message);
    vm->running = 0;
}

/* Get variable name by slot */
static const char* vm_get_var_name(VMState *vm, uint16_t slot) {
    if (slot < vm->program->var_count) {
        return vm->program->var_table[slot].name;
    }
    return "?";
}

/* Find line offset (binary search) */
int32_t vm_find_line_offset(VMState *vm, uint16_t line_number) {
    int left = 0;
    int right = vm->program->line_count - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (vm->program->line_map[mid].line_number == line_number) {
            return vm->program->line_map[mid].pc_offset;
        } else if (vm->program->line_map[mid].line_number < line_number) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

/* Main VM execution loop */
void vm_execute(VMState *vm) {
    while (vm->running && vm->pc < vm->program->code_len) {
        Instruction inst = vm->program->code[vm->pc];
        
        switch (inst.opcode) {
            /* Stack Operations */
            case OP_PUSH_CONST: {
                double value = vm->program->const_pool[inst.operand];
                vm_push(vm, value);
                vm->pc++;
                break;
            }
            
            case OP_PUSH_VAR: {
                double value = vm->num_vars[inst.operand];
                vm_push(vm, value);
                vm->pc++;
                break;
            }
            
            case OP_STR_PUSH_VAR: {
                const char *str = vm->str_vars[inst.operand];
                if (str) {
                    vm_str_push(vm, str);
                } else {
                    vm_str_push(vm, "");
                }
                vm->pc++;
                break;
            }
            
            case OP_POP_VAR: {
                double value = vm_pop(vm);
                vm->num_vars[inst.operand] = value;
                vm->pc++;
                break;
            }
            
            case OP_STR_POP_VAR: {
                const char *str = vm_str_pop(vm);
                if (vm->str_vars[inst.operand]) {
                    free(vm->str_vars[inst.operand]);
                }
                vm->str_vars[inst.operand] = my_strdup(str);
                vm->pc++;
                break;
            }
            
            case OP_DUP: {
                if (vm->stack_top > 0) {
                    double value = vm->stack[vm->stack_top - 1];
                    vm_push(vm, value);
                }
                vm->pc++;
                break;
            }
            
            case OP_POP: {
                vm_pop(vm);
                vm->pc++;
                break;
            }
            
            /* Arithmetic Operations */
            case OP_ADD: {
                double b = vm_pop(vm);
                double a = vm_pop(vm);
                vm_push(vm, a + b);
                vm->pc++;
                break;
            }
            
            case OP_SUB: {
                double b = vm_pop(vm);
                double a = vm_pop(vm);
                vm_push(vm, a - b);
                vm->pc++;
                break;
            }
            
            case OP_MUL: {
                double b = vm_pop(vm);
                double a = vm_pop(vm);
                vm_push(vm, a * b);
                vm->pc++;
                break;
            }
            
            case OP_DIV: {
                double b = vm_pop(vm);
                double a = vm_pop(vm);
                if (b == 0.0) {
                    vm_error(vm, "DIVISION BY ZERO");
                    if (vm->trap_triggered) {
                        vm->trap_triggered = 0;
                        break;  /* Don't increment pc, trap handler set it */
                    }
                } else {
                    vm_push(vm, a / b);
                }
                vm->pc++;
                break;
            }
            
            case OP_MOD: {
                double b = vm_pop(vm);
                double a = vm_pop(vm);
                vm_push(vm, fmod(a, b));
                vm->pc++;
                break;
            }
            
            case OP_POW: {
                double b = vm_pop(vm);
                double a = vm_pop(vm);
                vm_push(vm, pow(a, b));
                vm->pc++;
                break;
            }
            
            case OP_NEG: {
                double a = vm_pop(vm);
                vm_push(vm, -a);
                vm->pc++;
                break;
            }
            
            /* Comparison Operations */
            case OP_EQ: {
                /* Check if we have strings on string stack */
                if (vm->str_stack_top >= 2) {
                    char *b = vm_str_pop(vm);
                    char *a = vm_str_pop(vm);
                    int result = (strcmp(a, b) == 0) ? 1 : 0;
                    free(a);
                    free(b);
                    vm_push(vm, (double)result);
                } else {
                    /* Numeric comparison */
                    double b = vm_pop(vm);
                    double a = vm_pop(vm);
                    vm_push(vm, (a == b) ? 1.0 : 0.0);
                }
                vm->pc++;
                break;
            }
            
            case OP_NE: {
                /* Check if we have strings on string stack */
                if (vm->str_stack_top >= 2) {
                    char *b = vm_str_pop(vm);
                    char *a = vm_str_pop(vm);
                    int result = (strcmp(a, b) != 0) ? 1 : 0;
                    free(a);
                    free(b);
                    vm_push(vm, (double)result);
                } else {
                    /* Numeric comparison */
                    double b = vm_pop(vm);
                    double a = vm_pop(vm);
                    vm_push(vm, (a != b) ? 1.0 : 0.0);
                }
                vm->pc++;
                break;
            }
            
            case OP_LT: {
                /* Check if we have strings on string stack */
                if (vm->str_stack_top >= 2) {
                    char *b = vm_str_pop(vm);
                    char *a = vm_str_pop(vm);
                    int result = (strcmp(a, b) < 0) ? 1 : 0;
                    free(a);
                    free(b);
                    vm_push(vm, (double)result);
                } else {
                    /* Numeric comparison */
                    double b = vm_pop(vm);
                    double a = vm_pop(vm);
                    vm_push(vm, (a < b) ? 1.0 : 0.0);
                }
                vm->pc++;
                break;
            }
            
            case OP_LE: {
                /* Check if we have strings on string stack */
                if (vm->str_stack_top >= 2) {
                    char *b = vm_str_pop(vm);
                    char *a = vm_str_pop(vm);
                    int result = (strcmp(a, b) <= 0) ? 1 : 0;
                    free(a);
                    free(b);
                    vm_push(vm, (double)result);
                } else {
                    /* Numeric comparison */
                    double b = vm_pop(vm);
                    double a = vm_pop(vm);
                    vm_push(vm, (a <= b) ? 1.0 : 0.0);
                }
                vm->pc++;
                break;
            }
            
            case OP_GT: {
                /* Check if we have strings on string stack */
                if (vm->str_stack_top >= 2) {
                    char *b = vm_str_pop(vm);
                    char *a = vm_str_pop(vm);
                    int result = (strcmp(a, b) > 0) ? 1 : 0;
                    free(a);
                    free(b);
                    vm_push(vm, (double)result);
                } else {
                    /* Numeric comparison */
                    double b = vm_pop(vm);
                    double a = vm_pop(vm);
                    vm_push(vm, (a > b) ? 1.0 : 0.0);
                }
                vm->pc++;
                break;
            }
            
            case OP_GE: {
                /* Check if we have strings on string stack */
                if (vm->str_stack_top >= 2) {
                    char *b = vm_str_pop(vm);
                    char *a = vm_str_pop(vm);
                    int result = (strcmp(a, b) >= 0) ? 1 : 0;
                    free(a);
                    free(b);
                    vm_push(vm, (double)result);
                } else {
                    /* Numeric comparison */
                    double b = vm_pop(vm);
                    double a = vm_pop(vm);
                    vm_push(vm, (a >= b) ? 1.0 : 0.0);
                }
                vm->pc++;
                break;
            }
            
            /* Logical Operations */
            case OP_AND: {
                double b = vm_pop(vm);
                double a = vm_pop(vm);
                vm_push(vm, (a != 0.0 && b != 0.0) ? 1.0 : 0.0);
                vm->pc++;
                break;
            }
            
            case OP_OR: {
                double b = vm_pop(vm);
                double a = vm_pop(vm);
                vm_push(vm, (a != 0.0 || b != 0.0) ? 1.0 : 0.0);
                vm->pc++;
                break;
            }
            
            case OP_NOT: {
                double a = vm_pop(vm);
                vm_push(vm, (a == 0.0) ? 1.0 : 0.0);
                vm->pc++;
                break;
            }
            
            /* String Operations */
            case OP_STR_PUSH: {
                const char *str = vm->program->string_pool[inst.operand];
                vm_str_push(vm, str);
                vm->pc++;
                break;
            }
            
            case OP_STR_LEFT: {
                /* LEFT$(str, len) - pops len, then str */
                double len_d = vm_pop(vm);
                const char *str = vm_str_pop(vm);
                int len = (int)len_d;
                char *result = malloc(len + 1);
                int i;
                
                if (len < 0) len = 0;
                for (i = 0; i < len && str[i]; i++) {
                    result[i] = str[i];
                }
                result[i] = '\0';
                vm_str_push(vm, result);
                free(result);
                vm->pc++;
                break;
            }
            
            case OP_STR_RIGHT: {
                /* RIGHT$(str, len) - pops len, then str */
                double len_d = vm_pop(vm);
                const char *str = vm_str_pop(vm);
                int len = (int)len_d;
                int str_len = strlen(str);
                int start;
                
                if (len < 0) len = 0;
                if (len > str_len) len = str_len;
                start = str_len - len;
                vm_str_push(vm, str + start);
                vm->pc++;
                break;
            }
            
            case OP_STR_MID: {
                /* MID$(str, start, len) - pops len, start, then str */
                /* If len is missing (only 2 args), it's treated as rest of string */
                double len_d = vm_pop(vm);
                double start_d = vm_pop(vm);
                const char *str = vm_str_pop(vm);
                int start = (int)start_d;
                int len = (int)len_d;
                int str_len = strlen(str);
                char *result;
                int i;
                
                /* Classic BASIC uses 1-based string indexing */
                start--;
                if (start < 0) start = 0;
                if (start >= str_len) {
                    vm_str_push(vm, "");
                    vm->pc++;
                    break;
                }
                
                if (len < 0 || len > str_len - start) {
                    len = str_len - start;
                }
                
                result = malloc(len + 1);
                for (i = 0; i < len && str[start + i]; i++) {
                    result[i] = str[start + i];
                }
                result[i] = '\0';
                vm_str_push(vm, result);
                free(result);
                vm->pc++;
                break;
            }
            
            case OP_STR_MID_2: {
                /* MID$(str, start) - 2 args, return from start to end */
                double start_d = vm_pop(vm);
                const char *str = vm_str_pop(vm);
                int start = (int)start_d;
                int str_len = strlen(str);
                char *result;
                int len;
                int i;
                
                /* Classic BASIC uses 1-based string indexing */
                start--;
                if (start < 0) start = 0;
                if (start >= str_len) {
                    vm_str_push(vm, "");
                    vm->pc++;
                    break;
                }
                
                /* Return rest of string from start position */
                len = str_len - start;
                result = malloc(len + 1);
                for (i = 0; i < len && str[start + i]; i++) {
                    result[i] = str[start + i];
                }
                result[i] = '\0';
                vm_str_push(vm, result);
                free(result);
                vm->pc++;
                break;
            }
            
            case OP_STR_LEN: {
                const char *str = vm_str_pop(vm);
                double len = (double)strlen(str);
                vm_push(vm, len);
                vm->pc++;
                break;
            }
            
            case OP_STR_CHR: {
                double code = vm_pop(vm);
                char result[2];
                result[0] = (char)(int)code;
                result[1] = '\0';
                vm_str_push(vm, result);
                vm->pc++;
                break;
            }
            
            case OP_STR_ASC: {
                const char *str = vm_str_pop(vm);
                double code = str[0] ? (double)(unsigned char)str[0] : 0.0;
                vm_push(vm, code);
                vm->pc++;
                break;
            }
            
            case OP_STR_STR: {
                double value = vm_pop(vm);
                char buffer[64];
                sprintf(buffer, "%g", value);
                vm_str_push(vm, buffer);
                vm->pc++;
                break;
            }
            
            case OP_STR_VAL: {
                const char *str = vm_str_pop(vm);
                double value = atof(str);
                vm_push(vm, value);
                vm->pc++;
                break;
            }
            
            /* Control Flow */
            case OP_JUMP: {
                vm->pc = inst.operand;
                break;
            }
            
            case OP_JUMP_IF_FALSE: {
                double cond = vm_pop(vm);
                if (cond == 0.0) {
                    vm->pc = inst.operand;
                } else {
                    vm->pc++;
                }
                break;
            }
            
            case OP_JUMP_IF_TRUE: {
                double cond = vm_pop(vm);
                if (cond != 0.0) {
                    vm->pc = inst.operand;
                } else {
                    vm->pc++;
                }
                break;
            }
            
            case OP_JUMP_LINE: {
                double line_d = vm_pop(vm);
                uint16_t line = (uint16_t)line_d;
                int32_t offset = vm_find_line_offset(vm, line);
                
                if (offset < 0) {
                    vm_error(vm, "UNDEF'D STATEMENT");
                } else {
                    vm->pc = offset;
                }
                break;
            }
            
            case OP_GOSUB: {
                vm_call_push(vm, vm->pc + 1);
                vm->pc = inst.operand;
                break;
            }
            
            case OP_GOSUB_LINE: {
                double line_d = vm_pop(vm);
                uint16_t line = (uint16_t)line_d;
                int32_t offset = vm_find_line_offset(vm, line);
                
                if (offset < 0) {
                    vm_error(vm, "UNDEF'D STATEMENT");
                } else {
                    vm_call_push(vm, vm->pc + 1);
                    vm->pc = offset;
                }
                break;
            }
            
            case OP_ON_GOTO: {
                double index_d = vm_pop(vm);
                int index = (int)index_d;
                uint16_t count = inst.operand;
                
                /* Index is 1-based; if out of range, skip the targets and continue */
                if (index >= 1 && index <= count) {
                    /* Read the target from the jump table */
                    uint32_t target = vm->program->code[vm->pc + index].operand;
                    vm->pc = target;
                } else {
                    /* Skip the jump table */
                    vm->pc += count + 1;
                }
                break;
            }
            
            case OP_ON_GOSUB: {
                double index_d = vm_pop(vm);
                int index = (int)index_d;
                uint16_t count = inst.operand;
                
                /* Index is 1-based; if out of range, skip the targets and continue */
                if (index >= 1 && index <= count) {
                    /* Read the target from the jump table */
                    uint32_t target = vm->program->code[vm->pc + index].operand;
                    /* Push return address (after the jump table) */
                    vm_call_push(vm, vm->pc + count + 1);
                    vm->pc = target;
                } else {
                    /* Skip the jump table */
                    vm->pc += count + 1;
                }
                break;
            }
            
            case OP_RETURN: {
                vm->pc = vm_call_pop(vm);
                break;
            }
            
            case OP_FOR_INIT: {
                ForLoopState state;
                double step, limit, start;
                
                step = vm_pop(vm);
                limit = vm_pop(vm);
                start = vm_pop(vm);
                
                state.step = step;
                state.limit = limit;
                state.var_slot = inst.operand;
                state.loop_start_pc = vm->pc + 1;
                
                vm->num_vars[inst.operand] = start;
                vm_for_push(vm, state);
                
                vm->pc++;
                break;
            }
            
            case OP_FOR_NEXT: {
                ForLoopState *loop;
                double new_val;
                int done;
                uint16_t var_slot;
                
                loop = vm_for_top(vm);
                if (!loop) break;
                
                /* Check if variable was specified (0xFFFF means no variable) */
                if (inst.operand != 0xFFFF) {
                    /* Variable specified - validate it matches the FOR loop */
                    if (loop->var_slot != inst.operand) {
                        char err_msg[256];
                        snprintf(err_msg, sizeof(err_msg),
                                "NEXT variable mismatch: expected %s, got %s",
                                vm_get_var_name(vm, loop->var_slot),
                                vm_get_var_name(vm, inst.operand));
                        vm_error(vm, err_msg);
                        break;
                    }
                    var_slot = inst.operand;
                } else {
                    /* No variable specified - use the FOR loop's variable */
                    var_slot = loop->var_slot;
                }
                
                new_val = vm->num_vars[var_slot] + loop->step;
                vm->num_vars[var_slot] = new_val;
                
                if (loop->step > 0) {
                    done = (new_val > loop->limit);
                } else {
                    done = (new_val < loop->limit);
                }
                
                if (!done) {
                    vm->pc = loop->loop_start_pc;
                } else {
                    vm_for_pop(vm);
                    vm->pc++;
                }
                break;
            }
            
            /* I/O Operations */
            case OP_SET_PRINT_CHANNEL: {
                /* Pop channel number from stack and set as current print channel */
                double chan_val = vm_pop(vm);
                int chan = (int)chan_val;
                
                /* Validate channel (0 = stdout, 1-7 = file channels) */
                if (chan < 0 || chan > 7) {
                    fprintf(stderr, "WARNING: Invalid print channel %d, using stdout\n", chan);
                    vm->print_channel = 0;
                } else {
                    vm->print_channel = (uint8_t)chan;
                }
                vm->pc++;
                break;
            }
            
            case OP_PRINT_NUM: {
                double value = vm_pop(vm);
                char buffer[32];
                size_t len;
                FILE *out = vm_get_output_file(vm);
                
                /* Format the number (buffer is large enough for %.12g output) */
                sprintf(buffer, "%.12g", value);
                
                /* Classic BASIC always prints leading space for ALL numbers (sign field),
                 * UNLESS immediately after a comma (PRINT_TAB) */
                if (!vm->print_after_tab) {
                    fprintf(out, " ");
                    vm->print_last_char = ' ';
                    vm->print_column++;
                }
                
                /* Print the number */
                fprintf(out, "%s", buffer);
                len = strlen(buffer);
                if (len > 0) {
                    vm->print_last_char = buffer[len - 1];
                }
                vm->print_column += len;
                
                /* Add trailing space unless followed by NEWLINE, TAB, or NOSEP */
                if (vm->pc + 1 < vm->program->code_len) {
                    uint8_t next_op = vm->program->code[vm->pc + 1].opcode;
                    if (next_op != OP_PRINT_NEWLINE && next_op != OP_PRINT_TAB && next_op != OP_PRINT_NOSEP) {
                        fprintf(out, " ");
                        vm->print_last_char = ' ';
                        vm->print_column++;
                    }
                }
                
                vm->print_after_tab = 0;  /* Reset after consuming */
                vm->print_needs_newline = 1;
                vm->pc++;
                break;
            }
            
            case OP_PRINT_STR: {
                char *str = vm_str_pop(vm);
                size_t len = strlen(str);
                FILE *out = vm_get_output_file(vm);
                
                fprintf(out, "%s", str);
                if (len > 0) {
                    vm->print_last_char = str[len - 1];
                }
                vm->print_column += len;
                free(str);
                vm->print_after_tab = 0;  /* Reset after any print operation */
                vm->print_needs_newline = 1;
                vm->pc++;
                break;
            }
            
            case OP_PRINT_NEWLINE: {
                FILE *out = vm_get_output_file(vm);
                fprintf(out, "\n");
                fflush(out);  /* Ensure data is written to file */
                vm->print_needs_newline = 0;
                vm->print_last_char = '\n';
                vm->print_after_tab = 0;
                vm->print_column = 1;  /* Reset to column 1 */
                
                /* Reset print channel to stdout after completing a PRINT statement */
                vm->print_channel = 0;
                
                vm->pc++;
                break;
            }
            
            case OP_PRINT_SPACE: {
                FILE *out = vm_get_output_file(vm);
                fprintf(out, " ");
                vm->print_last_char = ' ';
                vm->print_after_tab = 0;
                vm->pc++;
                break;
            }
            
            case OP_PRINT_TAB: {
                FILE *out = vm_get_output_file(vm);
                /* Classic BASIC uses 10-character print zones for commas */
                /* Just output a single space and set flag to suppress next number's leading space */
                fprintf(out, " ");
                vm->print_last_char = ' ';
                vm->print_column++;
                vm->print_after_tab = 1;  /* Signal to suppress leading space on next number */
                vm->pc++;
                break;
            }
            
            case OP_TAB_FUNC: {
                /* TAB(n) function - move cursor to column n */
                double target_col_d = vm_pop(vm);
                int target_col = (int)target_col_d;
                FILE *out = vm_get_output_file(vm);
                int spaces_needed;
                
                /* Handle edge cases per Microsoft BASIC spec */
                if (target_col < 1) {
                    target_col = 1;
                }
                
                /* If target > width, calculate position = target MOD width */
                if (target_col > vm->print_width) {
                    target_col = target_col % vm->print_width;
                    if (target_col == 0) target_col = vm->print_width;
                }
                
                /* If current position >= target, move to next line */
                if (vm->print_column >= target_col) {
                    fprintf(out, "\n");
                    vm->print_column = 1;
                }
                
                /* Print spaces to reach target column */
                spaces_needed = target_col - vm->print_column;
                while (spaces_needed > 0) {
                    fprintf(out, " ");
                    vm->print_column++;
                    spaces_needed--;
                }
                
                vm->print_last_char = ' ';
                vm->print_after_tab = 1;  /* Suppress leading space on next number */
                vm->pc++;
                break;
            }
            
            case OP_PRINT_NOSEP: {
                /* Semicolon separator - suppress spacing on next number */
                vm->print_after_tab = 1;  /* Reuse flag to suppress leading space */
                vm->pc++;
                break;
            }
            
            case OP_INPUT_PROMPT: {
                /* Display prompt string before INPUT */
                const char *prompt = vm->program->string_pool[inst.operand];
                printf("%s", prompt);
                fflush(stdout);
                vm->pc++;
                break;
            }
            
            case OP_INPUT_NUM: {
                char *value_str;
                int input_valid = 0;
                
                while (!input_valid) {
                    value_str = get_next_input_value(vm);
                    
                    if (strlen(value_str) == 0 && !vm->input_available) {
                        /* Empty input or EOF */
                        input_valid = 1;
                        vm->num_vars[inst.operand] = 0.0;
                    } else if (is_valid_numeric_input(value_str)) {
                        vm->num_vars[inst.operand] = atof(value_str);
                        input_valid = 1;
                    } else {
                        printf("ERROR - 18\n");
                        vm->input_available = 0;  /* Force new input line */
                    }
                }
                
                vm->pc++;
                break;
            }
            
            case OP_INPUT_STR: {
                char *value_str = get_next_input_value(vm);
                
                if (vm->str_vars[inst.operand]) {
                    free(vm->str_vars[inst.operand]);
                }
                vm->str_vars[inst.operand] = my_strdup(value_str);
                
                vm->pc++;
                break;
            }
            
            /* File I/O Operations */
            case OP_OPEN: {
                /* Stack: channel, mode, aux, filename */
                char *filename;
                double mode;
                double channel;
                int chan;
                
                filename = vm_str_pop(vm);
                (void)vm_pop(vm);  /* aux parameter (unused) */
                mode = vm_pop(vm);
                channel = vm_pop(vm);
                chan = (int)channel;
                
                if (chan < 1 || chan > 7) {
                    vm_error(vm, "Invalid channel number");
                    free(filename);
                    break;
                }
                
                /* Close existing handle if open */
                if (vm->file_handles[chan]) {
                    fclose(vm->file_handles[chan]);
                    vm->file_handles[chan] = NULL;
                }
                
                /* Open file based on mode */
                /* Mode 4 = read, 8 = write, 12 = read/write */
                if (mode == 4) {
                    vm->file_handles[chan] = fopen(filename, "rb");
                } else if (mode == 8) {
                    vm->file_handles[chan] = fopen(filename, "wb");
                } else if (mode == 12) {
                    vm->file_handles[chan] = fopen(filename, "r+b");
                    if (!vm->file_handles[chan]) {
                        /* If file doesn't exist, create it */
                        vm->file_handles[chan] = fopen(filename, "w+b");
                    }
                } else {
                    vm->file_handles[chan] = fopen(filename, "rb");
                }
                
                if (!vm->file_handles[chan]) {
                    vm->file_status[chan] = 170;  /* File not found error */
                } else {
                    vm->file_status[chan] = 1;  /* OK */
                    vm->file_positions[chan] = 0;
                }
                
                free(filename);
                vm->pc++;
                break;
            }
            
            case OP_CLOSE: {
                /* Stack: channel */
                double channel = vm_pop(vm);
                int chan = (int)channel;
                
                if (chan >= 1 && chan <= 7 && vm->file_handles[chan]) {
                    fclose(vm->file_handles[chan]);
                    vm->file_handles[chan] = NULL;
                    vm->file_status[chan] = 1;  /* OK */
                    
                    /* Reset print channel to stdout if we closed the current print channel */
                    if (vm->print_channel == chan) {
                        vm->print_channel = 0;
                    }
                }
                
                vm->pc++;
                break;
            }
            
            case OP_GET: {
                /* Stack: channel - Push: byte value */
                double channel;
                int chan;
                int byte;
                
                channel = vm_pop(vm);
                chan = (int)channel;
                
                if (chan < 1 || chan > 7 || !vm->file_handles[chan]) {
                    vm_error(vm, "Channel not open");
                    break;
                }
                
                byte = fgetc(vm->file_handles[chan]);
                if (byte == EOF) {
                    vm->file_status[chan] = 3;  /* EOF */
                    vm_push(vm, 0.0);
                } else {
                    vm->file_status[chan] = 1;  /* OK */
                    vm_push(vm, (double)byte);
                    vm->file_positions[chan]++;
                }
                
                vm->pc++;
                break;
            }
            
            case OP_PUT: {
                /* Stack: channel, value */
                double value;
                double channel;
                int chan;
                int byte;
                
                value = vm_pop(vm);
                channel = vm_pop(vm);
                chan = (int)channel;
                
                if (chan < 1 || chan > 7 || !vm->file_handles[chan]) {
                    vm_error(vm, "Channel not open");
                    break;
                }
                
                byte = (int)value & 0xFF;
                if (fputc(byte, vm->file_handles[chan]) == EOF) {
                    vm->file_status[chan] = 144;  /* Device done error */
                } else {
                    vm->file_status[chan] = 1;  /* OK */
                    vm->file_positions[chan]++;
                }
                
                vm->pc++;
                break;
            }
            
            case OP_NOTE: {
                /* Stack: channel - Push: sector, byte_position */
                double channel;
                int chan;
                long pos;
                long sector;
                long byte_pos;
                
                channel = vm_pop(vm);
                chan = (int)channel;
                
                if (chan < 1 || chan > 7 || !vm->file_handles[chan]) {
                    vm_error(vm, "Channel not open");
                    break;
                }
                
                pos = ftell(vm->file_handles[chan]);
                sector = pos / 125;  /* Classic BASIC file positioning (125-byte sectors) */
                byte_pos = pos % 125;
                
                vm_push(vm, (double)sector);
                vm_push(vm, (double)byte_pos);
                vm->file_positions[chan] = pos;
                
                vm->pc++;
                break;
            }
            
            case OP_POINT: {
                /* Stack: channel, sector, byte_position */
                double byte_pos;
                double sector;
                double channel;
                int chan;
                long pos;
                
                byte_pos = vm_pop(vm);
                sector = vm_pop(vm);
                channel = vm_pop(vm);
                chan = (int)channel;
                
                if (chan < 1 || chan > 7 || !vm->file_handles[chan]) {
                    vm_error(vm, "Channel not open");
                    break;
                }
                
                pos = (long)sector * 125 + (long)byte_pos;
                if (fseek(vm->file_handles[chan], pos, SEEK_SET) != 0) {
                    vm->file_status[chan] = 1;  /* Error */
                } else {
                    vm->file_status[chan] = 0;
                    vm->file_positions[chan] = pos;
                }
                
                vm->pc++;
                break;
            }
            
            case OP_STATUS: {
                /* Stack: channel - Push: status */
                double channel = vm_pop(vm);
                int chan = (int)channel;
                
                if (chan < 1 || chan > 7) {
                    vm_push(vm, 1.0);  /* Invalid channel */
                } else {
                    vm_push(vm, (double)vm->file_status[chan]);
                }
                
                vm->pc++;
                break;
            }
            
            /* Math Functions */
            case OP_FUNC_SIN: {
                double x = vm_pop(vm);
                if (vm->deg_mode) {
                    vm_push(vm, sin(x * M_PI / 180.0));
                } else {
                    vm_push(vm, sin(x));
                }
                vm->pc++;
                break;
            }
            
            case OP_FUNC_COS: {
                double x = vm_pop(vm);
                if (vm->deg_mode) {
                    vm_push(vm, cos(x * M_PI / 180.0));
                } else {
                    vm_push(vm, cos(x));
                }
                vm->pc++;
                break;
            }
            
            case OP_FUNC_TAN: {
                double x = vm_pop(vm);
                if (vm->deg_mode) {
                    vm_push(vm, tan(x * M_PI / 180.0));
                } else {
                    vm_push(vm, tan(x));
                }
                vm->pc++;
                break;
            }
            
            case OP_FUNC_ATN: {
                double x = vm_pop(vm);
                if (vm->deg_mode) {
                    vm_push(vm, atan(x) * 180.0 / M_PI);
                } else {
                    vm_push(vm, atan(x));
                }
                vm->pc++;
                break;
            }
            
            case OP_FUNC_EXP: {
                double x = vm_pop(vm);
                vm_push(vm, exp(x));
                vm->pc++;
                break;
            }
            
            case OP_FUNC_LOG: {
                double x = vm_pop(vm);
                if (x <= 0) {
                    vm_error(vm, "LOG OF NEGATIVE NUMBER");
                } else {
                    vm_push(vm, log(x));
                }
                vm->pc++;
                break;
            }
            
            case OP_FUNC_CLOG: {
                double x = vm_pop(vm);
                if (x <= 0) {
                    vm_error(vm, "LOG OF NEGATIVE NUMBER");
                } else {
                    vm_push(vm, log10(x));
                }
                vm->pc++;
                break;
            }
            
            case OP_FUNC_SQR: {
                double x = vm_pop(vm);
                if (x < 0) {
                    vm_error(vm, "SQRT OF NEGATIVE NUMBER");
                } else {
                    vm_push(vm, sqrt(x));
                }
                vm->pc++;
                break;
            }
            
            case OP_FUNC_ABS: {
                double x = vm_pop(vm);
                vm_push(vm, fabs(x));
                vm->pc++;
                break;
            }
            
            case OP_FUNC_INT: {
                double x = vm_pop(vm);
                vm_push(vm, floor(x));
                vm->pc++;
                break;
            }
            
            case OP_FUNC_RND: {
                double x = vm_pop(vm);
                double result;
                
                if (x < 0) {
                    /* RND(negative) - reseed the generator */
                    /* Use absolute value as seed */
                    vm->rnd_seed = (uint32_t)(fabs(x) * 1000000.0);
                    if (vm->rnd_seed == 0) vm->rnd_seed = 1;
                    
                    /* Generate new random number with new seed */
                    /* Linear Congruential Generator (Knuth-style, Microsoft BASIC compatible) */
                    /* Formula: seed = (seed * 214013 + 2531011) mod 2^32 */
                    vm->rnd_seed = (vm->rnd_seed * 214013 + 2531011) & 0xFFFFFFFF;
                    result = (double)(vm->rnd_seed >> 16) / 65536.0;  /* Use upper 16 bits, normalize to [0,1) */
                    vm->last_rnd = result;
                } else if (x == 0) {
                    /* RND(0) - return last random number */
                    result = vm->last_rnd;
                } else {
                    /* RND(positive) - generate new random number */
                    /* Linear Congruential Generator */
                    vm->rnd_seed = (vm->rnd_seed * 214013 + 2531011) & 0xFFFFFFFF;
                    result = (double)(vm->rnd_seed >> 16) / 65536.0;
                    vm->last_rnd = result;
                }
                
                vm_push(vm, result);
                vm->pc++;
                break;
            }
            
            case OP_FUNC_SGN: {
                double x = vm_pop(vm);
                if (x > 0) vm_push(vm, 1.0);
                else if (x < 0) vm_push(vm, -1.0);
                else vm_push(vm, 0.0);
                vm->pc++;
                break;
            }
            
            case OP_FUNC_PEEK: {
                /* PEEK(address) - returns byte value at memory location */
                double addr_d = vm_pop(vm);
                int addr = (int)addr_d;
                unsigned char value = 0;
                
                /* Validate address range (0-65535) */
                if (addr < 0 || addr > 65535) {
                    vm_error(vm, "ILLEGAL ADDRESS IN PEEK");
                    break;
                }
                
                /* Read from simulated memory buffer */
                value = vm->memory[addr];
                vm_push(vm, (double)value);
                
                vm->pc++;
                break;
            }
            
            case OP_POKE: {
                /* POKE address, value - writes byte value to memory location */
                double value_d = vm_pop(vm);
                double addr_d = vm_pop(vm);
                int addr = (int)addr_d;
                int byte_val = (int)value_d;
                
                /* Validate address range (0-65535) */
                if (addr < 0 || addr > 65535) {
                    vm_error(vm, "ILLEGAL ADDRESS IN POKE");
                    break;
                }
                
                /* Only use low order byte (0-255) per Microsoft BASIC spec */
                byte_val = byte_val & 0xFF;
                
                /* Write to simulated memory buffer */
                vm->memory[addr] = (unsigned char)byte_val;
                
                vm->pc++;
                break;
            }
            
            /* Array Operations */
            case OP_DIM_1D: {
                double size_d = vm_pop(vm);
                size_t size = (size_t)size_d + 1;  /* Classic BASIC: DIM A(10) allocates 0-10 */
                int is_string = vm->arrays[inst.operand].is_string;
                size_t i;
                
                /* Free existing array */
                if (is_string && vm->arrays[inst.operand].u.str_data) {
                    size_t old_size = vm->arrays[inst.operand].dim1;
                    for (i = 0; i < old_size; i++) {
                        if (vm->arrays[inst.operand].u.str_data[i]) free(vm->arrays[inst.operand].u.str_data[i]);
                    }
                    free(vm->arrays[inst.operand].u.str_data);
                } else if (!is_string && vm->arrays[inst.operand].u.data) {
                    free(vm->arrays[inst.operand].u.data);
                }
                
                vm->arrays[inst.operand].type = VAR_ARRAY_1D;
                vm->arrays[inst.operand].dim1 = size;
                vm->arrays[inst.operand].dim2 = 0;
                
                if (is_string) {
                    vm->arrays[inst.operand].u.str_data = calloc(size, sizeof(char*));
                    for (i = 0; i < size; i++) {
                        vm->arrays[inst.operand].u.str_data[i] = my_strdup("");
                    }
                } else {
                    vm->arrays[inst.operand].u.data = calloc(size, sizeof(double));
                }
                
                vm->pc++;
                break;
            }
            
            case OP_DIM_2D: {
                double col_d = vm_pop(vm);
                double row_d = vm_pop(vm);
                size_t rows = (size_t)row_d + 1;
                size_t cols = (size_t)col_d + 1;
                int is_string = vm->arrays[inst.operand].is_string;
                size_t i;
                
                /* Free existing array */
                if (is_string && vm->arrays[inst.operand].u.str_data) {
                    size_t old_size = vm->arrays[inst.operand].dim1 * vm->arrays[inst.operand].dim2;
                    for (i = 0; i < old_size; i++) {
                        if (vm->arrays[inst.operand].u.str_data[i]) free(vm->arrays[inst.operand].u.str_data[i]);
                    }
                    free(vm->arrays[inst.operand].u.str_data);
                } else if (!is_string && vm->arrays[inst.operand].u.data) {
                    free(vm->arrays[inst.operand].u.data);
                }
                
                vm->arrays[inst.operand].type = VAR_ARRAY_2D;
                vm->arrays[inst.operand].dim1 = rows;
                vm->arrays[inst.operand].dim2 = cols;
                
                if (is_string) {
                    vm->arrays[inst.operand].u.str_data = calloc(rows * cols, sizeof(char*));
                    for (i = 0; i < rows * cols; i++) {
                        vm->arrays[inst.operand].u.str_data[i] = my_strdup("");
                    }
                } else {
                    vm->arrays[inst.operand].u.data = calloc(rows * cols, sizeof(double));
                }
                
                vm->pc++;
                break;
            }
            
            case OP_ARRAY_GET_1D: {
                double idx_d = vm_pop(vm);
                size_t idx = (size_t)idx_d;
                
                /* Auto-dimension if not already dimensioned */
                if (vm->arrays[inst.operand].u.data == NULL) {
                    vm->arrays[inst.operand].type = VAR_ARRAY_1D;
                    vm->arrays[inst.operand].dim1 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].dim2 = 0;
                    vm->arrays[inst.operand].is_string = 0;
                    vm->arrays[inst.operand].u.data = calloc(11, sizeof(double));
                }
                
                if (idx >= vm->arrays[inst.operand].dim1) {
                    fprintf(stderr, "Array index %lu out of bounds (max %lu) for array slot %d at PC=%d\n", 
                            (unsigned long)idx, (unsigned long)(vm->arrays[inst.operand].dim1 - 1), inst.operand, vm->pc);
                    vm_error(vm, "ARRAY BOUNDS ERROR");
                } else {
                    vm_push(vm, vm->arrays[inst.operand].u.data[idx]);
                }
                
                vm->pc++;
                break;
            }
            
            case OP_ARRAY_SET_1D: {
                double value, idx_d;
                size_t idx;
                
                if (vm->stack_top < 2) {
                    vm_error(vm, "STACK UNDERFLOW");
                    break;
                }
                
                value = vm_pop(vm);
                idx_d = vm_pop(vm);
                idx = (size_t)idx_d;
                
                /* Auto-dimension if not already dimensioned */
                if (vm->arrays[inst.operand].u.data == NULL) {
                    vm->arrays[inst.operand].type = VAR_ARRAY_1D;
                    vm->arrays[inst.operand].dim1 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].dim2 = 0;
                    vm->arrays[inst.operand].is_string = 0;
                    vm->arrays[inst.operand].u.data = calloc(11, sizeof(double));
                }
                
                if (idx >= vm->arrays[inst.operand].dim1) {
                    vm_error(vm, "ARRAY BOUNDS ERROR");
                } else {
                    vm->arrays[inst.operand].u.data[idx] = value;
                }
                
                vm->pc++;
                break;
            }
            
            case OP_ARRAY_GET_2D: {
                double col_d = vm_pop(vm);
                double row_d = vm_pop(vm);
                size_t row = (size_t)row_d;
                size_t col = (size_t)col_d;
                size_t cols;
                
                /* Auto-dimension if not already dimensioned */
                if (vm->arrays[inst.operand].u.data == NULL) {
                    vm->arrays[inst.operand].type = VAR_ARRAY_2D;
                    vm->arrays[inst.operand].dim1 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].dim2 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].is_string = 0;
                    vm->arrays[inst.operand].u.data = calloc(11 * 11, sizeof(double));
                }
                
                cols = vm->arrays[inst.operand].dim2;
                
                if (row >= vm->arrays[inst.operand].dim1 || col >= cols) {
                    vm_error(vm, "ARRAY BOUNDS ERROR");
                } else {
                    vm_push(vm, vm->arrays[inst.operand].u.data[row * cols + col]);
                }
                
                vm->pc++;
                break;
            }
            
            case OP_ARRAY_SET_2D: {
                double value = vm_pop(vm);
                double col_d = vm_pop(vm);
                double row_d = vm_pop(vm);
                size_t row = (size_t)row_d;
                size_t col = (size_t)col_d;
                size_t cols;
                
                /* Auto-dimension if not already dimensioned */
                if (vm->arrays[inst.operand].u.data == NULL) {
                    vm->arrays[inst.operand].type = VAR_ARRAY_2D;
                    vm->arrays[inst.operand].dim1 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].dim2 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].is_string = 0;
                    vm->arrays[inst.operand].u.data = calloc(11 * 11, sizeof(double));
                }
                
                cols = vm->arrays[inst.operand].dim2;
                
                if (row >= vm->arrays[inst.operand].dim1 || col >= cols) {
                    vm_error(vm, "ARRAY BOUNDS ERROR");
                } else {
                    vm->arrays[inst.operand].u.data[row * cols + col] = value;
                }
                
                vm->pc++;
                break;
            }
            
            /* String Array Operations */
            case OP_STR_ARRAY_GET_1D: {
                double idx_d = vm_pop(vm);
                size_t idx = (size_t)idx_d;
                
                /* Auto-dimension if not already dimensioned */
                if (vm->arrays[inst.operand].u.str_data == NULL) {
                    size_t i;
                    vm->arrays[inst.operand].type = VAR_ARRAY_1D;
                    vm->arrays[inst.operand].dim1 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].dim2 = 0;
                    vm->arrays[inst.operand].is_string = 1;
                    vm->arrays[inst.operand].u.str_data = calloc(11, sizeof(char*));
                    for (i = 0; i < 11; i++) {
                        vm->arrays[inst.operand].u.str_data[i] = my_strdup("");
                    }
                }
                
                if (idx >= vm->arrays[inst.operand].dim1) {
                    vm_error(vm, "ARRAY BOUNDS ERROR");
                } else {
                    vm_str_push(vm, vm->arrays[inst.operand].u.str_data[idx]);
                }
                
                vm->pc++;
                break;
            }
            
            case OP_STR_ARRAY_SET_1D: {
                const char *value;
                double idx_d;
                size_t idx;
                
                if (vm->str_stack_top < 1 || vm->stack_top < 1) {
                    vm_error(vm, "STACK UNDERFLOW");
                    break;
                }
                
                value = vm_str_pop(vm);
                idx_d = vm_pop(vm);
                idx = (size_t)idx_d;
                
                /* Auto-dimension if not already dimensioned */
                if (vm->arrays[inst.operand].u.str_data == NULL) {
                    size_t i;
                    vm->arrays[inst.operand].type = VAR_ARRAY_1D;
                    vm->arrays[inst.operand].dim1 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].dim2 = 0;
                    vm->arrays[inst.operand].is_string = 1;
                    vm->arrays[inst.operand].u.str_data = calloc(11, sizeof(char*));
                    for (i = 0; i < 11; i++) {
                        vm->arrays[inst.operand].u.str_data[i] = my_strdup("");
                    }
                }
                
                if (idx >= vm->arrays[inst.operand].dim1) {
                    vm_error(vm, "ARRAY BOUNDS ERROR");
                } else {
                    if (vm->arrays[inst.operand].u.str_data[idx]) {
                        free(vm->arrays[inst.operand].u.str_data[idx]);
                    }
                    vm->arrays[inst.operand].u.str_data[idx] = my_strdup(value);
                }
                
                vm->pc++;
                break;
            }
            
            case OP_STR_ARRAY_GET_2D: {
                double col_d = vm_pop(vm);
                double row_d = vm_pop(vm);
                size_t row = (size_t)row_d;
                size_t col = (size_t)col_d;
                size_t cols;
                
                /* Auto-dimension if not already dimensioned */
                if (vm->arrays[inst.operand].u.str_data == NULL) {
                    size_t i;
                    vm->arrays[inst.operand].type = VAR_ARRAY_2D;
                    vm->arrays[inst.operand].dim1 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].dim2 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].is_string = 1;
                    vm->arrays[inst.operand].u.str_data = calloc(11 * 11, sizeof(char*));
                    for (i = 0; i < 11 * 11; i++) {
                        vm->arrays[inst.operand].u.str_data[i] = my_strdup("");
                    }
                }
                
                cols = vm->arrays[inst.operand].dim2;
                
                if (row >= vm->arrays[inst.operand].dim1 || col >= cols) {
                    vm_error(vm, "ARRAY BOUNDS ERROR");
                } else {
                    vm_str_push(vm, vm->arrays[inst.operand].u.str_data[row * cols + col]);
                }
                
                vm->pc++;
                break;
            }
            
            case OP_STR_ARRAY_SET_2D: {
                const char *value = vm_str_pop(vm);
                double col_d = vm_pop(vm);
                double row_d = vm_pop(vm);
                size_t row = (size_t)row_d;
                size_t col = (size_t)col_d;
                size_t cols;
                
                /* Auto-dimension if not already dimensioned */
                if (vm->arrays[inst.operand].u.str_data == NULL) {
                    size_t i;
                    vm->arrays[inst.operand].type = VAR_ARRAY_2D;
                    vm->arrays[inst.operand].dim1 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].dim2 = 11;  /* Default 0-10 */
                    vm->arrays[inst.operand].is_string = 1;
                    vm->arrays[inst.operand].u.str_data = calloc(11 * 11, sizeof(char*));
                    for (i = 0; i < 11 * 11; i++) {
                        vm->arrays[inst.operand].u.str_data[i] = my_strdup("");
                    }
                }
                
                cols = vm->arrays[inst.operand].dim2;
                
                if (row >= vm->arrays[inst.operand].dim1 || col >= cols) {
                    vm_error(vm, "ARRAY BOUNDS ERROR");
                } else {
                    if (vm->arrays[inst.operand].u.str_data[row * cols + col]) {
                        free(vm->arrays[inst.operand].u.str_data[row * cols + col]);
                    }
                    vm->arrays[inst.operand].u.str_data[row * cols + col] = my_strdup(value);
                }
                
                vm->pc++;
                break;
            }
            
            /* DATA/READ Operations */
            case OP_DATA_READ_NUM: {
                if (vm->data_pointer >= vm->program->data_count) {
                    vm_error(vm, "OUT OF DATA");
                } else {
                    DataEntry *entry = &vm->program->data_entries[vm->data_pointer++];
                    
                    if (entry->type == DATA_STRING) {
                        /* String to numeric conversion (per Microsoft BASIC spec) */
                        /* Identifiers in DATA convert to 0, not error */
                        const char *str = vm->program->data_string_pool[entry->value.string_idx];
                        double value = atof(str);  /* atof returns 0 for non-numeric strings */
                        vm->num_vars[inst.operand] = value;
                    } else if (entry->type == DATA_NUMERIC) {
                        double value = vm->program->data_numeric_pool[entry->value.numeric_idx];
                        vm->num_vars[inst.operand] = value;
                    } else if (entry->type == DATA_NULL) {
                        /* NULL data item - convert to 0 for numeric variable */
                        vm->num_vars[inst.operand] = 0;
                    } else {
                        vm_error(vm, "TYPE MISMATCH IN DATA");
                    }
                }
                
                vm->pc++;
                break;
            }
            
            case OP_DATA_READ_STR: {
                if (vm->data_pointer >= vm->program->data_count) {
                    vm_error(vm, "OUT OF DATA");
                } else {
                    DataEntry *entry = &vm->program->data_entries[vm->data_pointer++];
                    char buffer[64];
                    const char *str_value;
                    
                    if (entry->type == DATA_STRING) {
                        /* Direct string value */
                        str_value = vm->program->data_string_pool[entry->value.string_idx];
                    } else if (entry->type == DATA_NUMERIC) {
                        /* Numeric to string conversion (per Microsoft BASIC spec) */
                        double value = vm->program->data_numeric_pool[entry->value.numeric_idx];
                        sprintf(buffer, "%g", value);
                        str_value = buffer;
                    } else if (entry->type == DATA_NULL) {
                        /* NULL data item - convert to empty string for string variable */
                        str_value = "";
                    } else {
                        vm_error(vm, "TYPE MISMATCH IN DATA");
                        break;
                    }
                    
                    if (vm->str_vars[inst.operand]) {
                        free(vm->str_vars[inst.operand]);
                    }
                    vm->str_vars[inst.operand] = my_strdup(str_value);
                }
                
                vm->pc++;
                break;
            }
            
            case OP_RESTORE: {
                vm->data_pointer = 0;
                vm->pc++;
                break;
            }
            
            case OP_RESTORE_LINE: {
                /* RESTORE with line number - for now just reset to beginning */
                /* TODO: Could track line-specific data positions if needed */
                vm->data_pointer = 0;
                vm->pc++;
                break;
            }
            
            /* System Operations */
            case OP_TRAP: {
                /* Set trap line */
                vm->trap_line = inst.operand;
                vm->trap_enabled = 1;
                vm->pc++;
                break;
            }
            
            case OP_TRAP_DISABLE: {
                vm->trap_enabled = 0;
                vm->pc++;
                break;
            }
            
            /* File I/O Operations */
            case OP_XIO: {
                /* Stack: command, channel, aux1, aux2, device_string (TOS) */
                char *device = vm_str_pop(vm);
                int channel, command;
                (void)vm_pop(vm);  /* aux2 - not used */
                (void)vm_pop(vm);  /* aux1 - not used */
                channel = (int)vm_pop(vm);
                command = (int)vm_pop(vm);
                
                if (channel < 1 || channel >= 8) {
                    free(device);
                    vm_error(vm, "Invalid channel number");
                    break;
                }
                
                switch (command) {
                    case 3: /* OPEN for input */
                        if (vm->file_handles[channel]) {
                            fclose(vm->file_handles[channel]);
                        }
                        vm->file_handles[channel] = fopen(device, "rb");
                        if (!vm->file_handles[channel]) {
                            free(device);
                            vm_error(vm, "Cannot open file for reading");
                        }
                        break;
                        
                    case 8: /* OPEN for output */
                        if (vm->file_handles[channel]) {
                            fclose(vm->file_handles[channel]);
                        }
                        vm->file_handles[channel] = fopen(device, "wb");
                        if (!vm->file_handles[channel]) {
                            free(device);
                            vm_error(vm, "Cannot open file for writing");
                        }
                        break;
                        
                    case 12: /* CLOSE */
                        if (vm->file_handles[channel]) {
                            fclose(vm->file_handles[channel]);
                            vm->file_handles[channel] = NULL;
                        }
                        break;
                        
                    case 34: /* DELETE file */
                        if (remove(device) == 0) {
                            printf("File '%s' deleted\n", device);
                        } else {
                            free(device);
                            vm_error(vm, "Cannot delete file");
                        }
                        break;
                        
                    default:
                        free(device);
                        vm_error(vm, "Unsupported XIO command");
                        break;
                }
                
                free(device);
                vm->pc++;
                break;
            }
            
            case OP_END:
            case OP_STOP: {
                vm->running = 0;
                break;
            }
            
            case OP_DEG: {
                vm->deg_mode = 1;
                vm->pc++;
                break;
            }
            
            case OP_RAD: {
                vm->deg_mode = 0;
                vm->pc++;
                break;
            }
            
            case OP_RANDOMIZE: {
                /* Pop seed value from stack */
                double seed_val = vm_pop(vm);
                uint32_t seed;
                
                /* Use provided value as seed (could be from TIMER or numeric expression) */
                seed = (uint32_t)fabs(seed_val);
                if (seed == 0) seed = 1;  /* Ensure non-zero seed */
                
                /* Set the RNG seed */
                vm->rnd_seed = seed;
                
                /* Generate first random number so RND(0) works */
                vm->rnd_seed = (vm->rnd_seed * 214013 + 2531011) & 0xFFFFFFFF;
                vm->last_rnd = ((vm->rnd_seed >> 16) & 0xFFFF) / 65536.0;
                
                vm->pc++;
                break;
            }
            
            case OP_CLR: {
                size_t i, j;
                /* Clear all numeric variables */
                for (i = 0; i < vm->var_capacity; i++) {
                    vm->num_vars[i] = 0.0;
                }
                /* Clear all string variables */
                for (i = 0; i < vm->var_capacity; i++) {
                    if (vm->str_vars[i]) {
                        free(vm->str_vars[i]);
                        vm->str_vars[i] = NULL;
                    }
                }
                /* Clear all arrays */
                for (i = 0; i < vm->var_capacity; i++) {
                    if (vm->arrays[i].u.data) {
                        free(vm->arrays[i].u.data);
                        vm->arrays[i].u.data = NULL;
                    }
                    if (vm->arrays[i].u.str_data) {
                        for (j = 0; j < vm->arrays[i].dim1 * (vm->arrays[i].dim2 ? vm->arrays[i].dim2 : 1); j++) {
                            if (vm->arrays[i].u.str_data[j]) {
                                free(vm->arrays[i].u.str_data[j]);
                            }
                        }
                        free(vm->arrays[i].u.str_data);
                        vm->arrays[i].u.str_data = NULL;
                    }
                    vm->arrays[i].dim1 = 0;
                    vm->arrays[i].dim2 = 0;
                }
                vm->pc++;
                break;
            }
            
            case OP_POP_GOSUB: {
                /* Pop one entry from the GOSUB stack without jumping */
                if (vm->call_top > 0) {
                    vm->call_top--;
                }
                vm->pc++;
                break;
            }
            
            case OP_NOP: {
                vm->pc++;
                break;
            }
            
            default: {
                fprintf(stderr, "Unknown opcode: 0x%02X at PC=%d\n", inst.opcode, vm->pc);
                vm->running = 0;
                break;
            }
        }
    }
}
