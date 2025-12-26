/* vm.h - Virtual machine executor */
#ifndef VM_H
#define VM_H

#include "compiler.h"
#include "bytecode.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Array storage */
typedef struct {
    VarType type;                /* ARRAY_1D or ARRAY_2D */
    size_t dim1;                 /* First dimension size */
    size_t dim2;                 /* Second dimension (0 for 1D) */
    int is_string;               /* 1 for string arrays, 0 for numeric */
    union {
        double *data;            /* Numeric array data */
        char **str_data;         /* String array data */
    } u;
} ArrayData;

/* FOR loop state */
typedef struct {
    uint16_t var_slot;           /* Loop variable slot */
    double limit;                /* TO value */
    double step;                 /* STEP value (default 1.0) */
    uint32_t loop_start_pc;      /* PC of first instruction in loop body */
} ForLoopState;

/* VM State */
typedef struct {
    /* Execution State */
    uint32_t pc;                 /* Program counter */
    uint8_t running;             /* 1 if executing, 0 if stopped */
    uint8_t break_flag;          /* Set when BREAK key pressed */
    
    /* Value Stack (for expression evaluation) */
    double *stack;               /* Stack of numeric values */
    size_t stack_top;            /* Index of top element */
    size_t stack_capacity;       /* Allocated capacity */
    
    /* String Stack (for string expressions) */
    char **str_stack;            /* Stack of string pointers */
    size_t str_stack_top;
    size_t str_stack_capacity;
    
    /* Call Stack (for GOSUB/RETURN) */
    uint32_t *call_stack;        /* Return addresses */
    size_t call_top;             /* Top of call stack */
    size_t call_capacity;        /* Allocated capacity */
    
    /* FOR Loop Stack */
    ForLoopState *for_stack;     /* Active FOR loops */
    size_t for_top;              /* Top of FOR stack */
    size_t for_capacity;         /* Allocated capacity */
    
    /* Variable Storage (parallel arrays indexed by slot) */
    double *num_vars;            /* Numeric variable values */
    char **str_vars;             /* String variable values */
    ArrayData *arrays;           /* Array storage */
    size_t var_capacity;         /* Allocated slots */
    
    /* File I/O */
    FILE *file_handles[8];       /* File handles for channels 1-7 (0 unused) */
    uint8_t file_status[8];      /* Status codes for each channel (0=OK, 1=error) */
    long file_positions[8];      /* Current file positions */
    
    /* Global State */
    uint32_t trap_line;          /* TRAP error handler PC offset (0 = none) */
    uint8_t trap_enabled;        /* Is TRAP active? */
    uint8_t trap_triggered;      /* Was trap just triggered? (skip pc increment) */
    size_t data_pointer;         /* Current position in DATA pool */
    
    /* Print state (for PRINT newline handling) */
    uint8_t print_needs_newline; /* Track if newline is needed */
    char print_last_char;        /* Last character printed (for space suppression logic) */
    uint8_t print_after_tab;     /* Set after PRINT_TAB to suppress number's leading space */
    uint8_t print_channel;       /* Current output channel (0=stdout, 1-7=file handles) */
    int print_column;            /* Current print column (1-based, for TAB function) */
    int print_width;             /* Print width for current channel (default 80) */
    
    /* Trigonometric mode */
    uint8_t deg_mode;            /* 1 for degree mode, 0 for radian mode (default) */
    
    /* Random number generator state (Linear Congruential Generator) */
    uint32_t rnd_seed;           /* Current RNG seed */
    double last_rnd;             /* Last random number generated (for RND(0)) */
    
    /* INPUT buffer state (for comma-separated input) */
    char input_buffer[256];      /* Buffer for INPUT line */
    char *input_ptr;             /* Current position in input buffer */
    int input_available;         /* 1 if there's unparsed input in buffer */
    
    /* Simulated memory buffer for PEEK/POKE (64KB) */
    unsigned char *memory;       /* 64KB memory buffer (0-65535) */
    
    /* Reference to compiled program */
    CompiledProgram *program;
    
} VMState;

/* VM functions */
VMState* vm_init(CompiledProgram *program);
void vm_free(VMState *vm);
void vm_execute(VMState *vm);

/* Stack operations */
void vm_push(VMState *vm, double value);
double vm_pop(VMState *vm);
void vm_str_push(VMState *vm, const char *str);
char* vm_str_pop(VMState *vm);

/* Call stack operations */
void vm_call_push(VMState *vm, uint32_t return_addr);
uint32_t vm_call_pop(VMState *vm);

/* FOR stack operations */
void vm_for_push(VMState *vm, ForLoopState state);
ForLoopState* vm_for_top(VMState *vm);
void vm_for_pop(VMState *vm);

/* Error handling */
void vm_error(VMState *vm, const char *message);

/* Helper function for runtime line lookup */
int32_t vm_find_line_offset(VMState *vm, uint16_t line_number);

#endif /* VM_H */
