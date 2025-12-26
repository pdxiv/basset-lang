/* compiler.h - Bytecode compiler */
#ifndef COMPILER_H
#define COMPILER_H

#include "bytecode.h"
#include "parser.h"
#include <stddef.h>
#include <stdint.h>

/* Variable types */
typedef enum {
    VAR_NUMERIC,
    VAR_STRING,
    VAR_ARRAY_1D,
    VAR_ARRAY_2D
} VarType;

/* Variable information in symbol table */
typedef struct {
    char *name;                  /* Variable name (e.g., "A", "NAME$") */
    uint16_t slot;               /* Slot number in VM arrays */
    VarType type;                /* NUMERIC, STRING, ARRAY_1D, ARRAY_2D */
    uint16_t array_dim1;         /* For arrays: first dimension */
    uint16_t array_dim2;         /* For 2D arrays: second dimension */
} VariableInfo;

/* Line number to PC mapping */
typedef struct {
    uint16_t line_number;        /* BASIC line number */
    uint32_t pc_offset;          /* Bytecode offset for this line */
} LineMapping;

/* Jump fixup (forward references) */
typedef enum {
    JUMP_ABSOLUTE,
    JUMP_GOSUB,
    JUMP_ON_GOTO,
    JUMP_ON_GOSUB
} JumpType;

typedef struct {
    uint32_t instruction_offset; /* PC of instruction to patch */
    uint16_t target_line;        /* Line number to jump to */
    JumpType type;               /* Type of jump */
} JumpFixup;

/* DATA entry types */
typedef enum {
    DATA_NUMERIC,
    DATA_STRING,
    DATA_NULL       /* Empty/null value - converts based on READ variable type */
} DataType;

typedef struct {
    DataType type;               /* NUMERIC, STRING, or NULL */
    union {
        size_t numeric_idx;      /* Index into data_numeric_pool */
        size_t string_idx;       /* Index into data_string_pool */
    } value;
} DataEntry;

/* Compiled program structure */
typedef struct {
    /* Bytecode */
    Instruction *code;           /* Array of instructions */
    size_t code_len;             /* Number of instructions */
    size_t code_capacity;        /* Allocated capacity */
    
    /* Constant Pools */
    double *const_pool;          /* Numeric constants */
    size_t const_count;
    size_t const_capacity;
    
    char **string_pool;          /* String constants */
    size_t string_count;
    size_t string_capacity;
    
    /* DATA Statement Storage */
    double *data_numeric_pool;   /* Numeric DATA values */
    size_t data_numeric_count;
    size_t data_numeric_capacity;
    
    char **data_string_pool;     /* String DATA values */
    size_t data_string_count;
    size_t data_string_capacity;
    
    DataEntry *data_entries;     /* Ordered list with type info */
    size_t data_count;
    size_t data_capacity;
    
    /* Symbol Table */
    VariableInfo *var_table;     /* Maps variable names to slots */
    size_t var_count;
    size_t var_capacity;
    
    /* Line Number Mapping */
    LineMapping *line_map;       /* Sorted array: line_num → PC offset */
    size_t line_count;
    size_t line_capacity;
    
} CompiledProgram;

/* Forward declaration for compilation dispatch */
struct CompilerState_t;
typedef struct CompilerState_t CompilerState;

/* Statement compiler function pointer type */
typedef void (*StatementCompiler)(CompilerState *cs, ParseNode *stmt);

/* Statement compilation dispatch entry */
typedef struct {
    unsigned char token;          /* Token type (e.g., TOK_PRINT) */
    StatementCompiler compiler;   /* Function to compile this statement */
} CompilationEntry;

/* Compiler state (used during compilation) */
struct CompilerState_t {
    CompiledProgram *program;
    
    /* Jump fixups (forward references) */
    JumpFixup *jump_fixups;
    size_t fixup_count;
    size_t fixup_capacity;
    
    /* Current line being compiled */
    uint16_t current_line;
    
    /* Error handling */
    int has_error;
    char error_msg[256];
    
};

/* Compiler functions */
CompiledProgram* compiler_compile(ParseNode *root);
void compiled_program_free(CompiledProgram *prog);

/* Helper functions for compiler */
CompilerState* compiler_state_new(void);
void compiler_state_free(CompilerState *cs);

/* Variable management */
int compiler_find_variable(CompilerState *cs, const char *name);
int compiler_add_variable(CompilerState *cs, const char *name, VarType type);

/* Constant pool management */
uint16_t compiler_add_const(CompilerState *cs, double value);
uint16_t compiler_add_string(CompilerState *cs, const char *str);

/* Code emission */
void compiler_emit(CompilerState *cs, uint8_t opcode, uint16_t operand);
void compiler_emit_no_operand(CompilerState *cs, uint8_t opcode);

/* Line mapping */
void compiler_add_line_mapping(CompilerState *cs, uint16_t line, uint32_t pc);
int32_t compiler_find_line_offset(CompilerState *cs, uint16_t line);

/* Jump fixups */
void compiler_add_jump_fixup(CompilerState *cs, uint32_t pc, uint16_t target_line, JumpType type);
void compiler_resolve_jumps(CompilerState *cs);

#endif /* COMPILER_H */
