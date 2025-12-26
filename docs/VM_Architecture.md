# Virtual Machine Architecture

## Overview

The BASIC Virtual Machine (VM) is a bytecode interpreter that executes compiled BASIC programs. It implements a stack-based architecture with separate numeric and string stacks, variable storage, array support, and control flow mechanisms.

## Design Principles

1. **Stack-Based**: Operations work on stacks rather than registers
2. **Dual Stacks**: Separate stacks for numeric and string values
3. **Type-Specific**: Distinct operations for numeric and string types
4. **Fixed-Width Instructions**: All instructions are 4 bytes
5. **Backward Compatibility**: Implements classic BASIC semantics faithfully

---

## Memory Architecture

### VM State Structure
```c
typedef struct {
    /* Program Code */
    Instruction *code;           /* Bytecode instruction array */
    uint32_t code_size;          /* Number of instructions */
    uint32_t pc;                 /* Program counter (instruction index) */
    
    /* Constants */
    double *constants;           /* Numeric constant pool */
    uint32_t const_count;        /* Number of constants */
    char **string_constants;     /* String constant pool */
    uint32_t string_const_count; /* Number of string constants */
    
    /* Variables */
    double num_vars[128];        /* Numeric variables (0-127) */
    char *str_vars[128];         /* String variables (0-127) */
    Variable *var_table;         /* Variable name table */
    uint32_t var_count;          /* Number of variables */
    
    /* Arrays */
    Array *num_arrays[128];      /* Numeric arrays */
    Array *str_arrays[128];      /* String arrays */
    
    /* Stacks */
    double *num_stack;           /* Numeric stack */
    int num_stack_top;           /* Numeric stack pointer */
    int num_stack_size;          /* Numeric stack capacity */
    
    char **str_stack;            /* String stack */
    int str_stack_top;           /* String stack pointer */
    int str_stack_size;          /* String stack capacity */
    
    /* Control Flow */
    ForLoop for_stack[32];       /* FOR/NEXT loop contexts */
    int for_stack_top;           /* FOR stack pointer */
    
    uint32_t gosub_stack[64];    /* GOSUB return addresses */
    int gosub_stack_top;         /* GOSUB stack pointer */
    
    /* Line Map */
    LineMapEntry *line_map;      /* Line number → PC mapping */
    uint32_t line_count;         /* Number of lines */
    
    /* I/O */
    FILE *channels[8];           /* I/O channels (0=stdin/stdout, 1-7=files) */
    int print_channel;           /* Current output channel for PRINT */
    
    /* DATA Statement Support */
    const char **data_items;     /* DATA values */
    uint32_t data_count;         /* Number of DATA items */
    uint32_t data_ptr;           /* Current DATA read position */
    
    /* System State */
    int running;                 /* Program is executing */
    int error_code;              /* Last error code */
    int trap_line;               /* TRAP error handler line (-1 = disabled) */
    int deg_mode;                /* Trig mode: 1=degrees, 0=radians */
    
} VM;
```

---

## Stack Architecture

### Numeric Stack

**Purpose**: Stores numeric values (double-precision floating-point) during expression evaluation.

**Operations**:
- **PUSH**: `num_stack[++num_stack_top] = value`
- **POP**: `value = num_stack[num_stack_top--]`
- **PEEK**: `value = num_stack[num_stack_top]` (no change to stack)

**Growth**: Dynamic allocation, expands as needed

**Usage Examples**:
- Expression: `X + Y * 2`
  ```
  PUSH X        → [X]
  PUSH Y        → [X, Y]
  PUSH 2        → [X, Y, 2]
  MUL           → [X, Y*2]
  ADD           → [X+Y*2]
  ```

### String Stack

**Purpose**: Stores string values during expression evaluation.

**Operations**:
- **PUSH**: `str_stack[++str_stack_top] = strdup(str)`
- **POP**: `str = str_stack[str_stack_top--]`
- **Memory**: Strings are heap-allocated; VM manages lifecycle

**Growth**: Dynamic allocation, expands as needed

**Usage Examples**:
- Concatenation: `"HELLO" + " " + "WORLD"`
  ```
  PUSH "HELLO"       → ["HELLO"]
  PUSH " "           → ["HELLO", " "]
  CONCAT             → ["HELLO "]
  PUSH "WORLD"       → ["HELLO ", "WORLD"]
  CONCAT             → ["HELLO WORLD"]
  ```

---

## Variable Storage

### Numeric Variables
- **Slots**: 128 slots (0-127)
- **Type**: `double` (IEEE 754 double-precision)
- **Initial Value**: 0.0
- **Access**: Direct array indexing
- **Compiler**: Assigns slot numbers during compilation

### String Variables
- **Slots**: 128 slots (0-127)
- **Type**: `char*` (dynamically allocated)
- **Initial Value**: Empty string `""`
- **Memory**: Automatic deallocation when reassigned
- **Compiler**: Assigns separate slot numbers from numeric variables

### Variable Name Table
```c
typedef struct {
    char *name;          /* Variable name (e.g., "X", "NAME$") */
    uint8_t slot;        /* Slot number (0-127) */
    uint8_t is_string;   /* 1 if string variable, 0 if numeric */
} Variable;
```

**Purpose**: Maps variable names to slot numbers for runtime lookups (used by TRAP handler, debugger, etc.)

---

## Array Support

### Array Structure
```c
typedef struct {
    void *data;          /* Array data (double* or char**) */
    uint32_t dim1;       /* First dimension size (0 to dim1) */
    uint32_t dim2;       /* Second dimension size (0 to dim2, or 0 for 1D) */
    uint8_t is_string;   /* 1 if string array, 0 if numeric */
} Array;
```

### Dimensions
- **1D Arrays**: `DIM A(10)` → 11 elements (0 to 10)
- **2D Arrays**: `DIM B(5,3)` → 6×4 = 24 elements (0,0) to (5,3)
- **Maximum Size**: 32767 per dimension (classic BASIC limitation)

### Storage
- **Numeric 1D**: `double data[dim1+1]`
- **Numeric 2D**: `double data[(dim1+1) * (dim2+1)]` (row-major)
- **String 1D**: `char* data[dim1+1]`
- **String 2D**: `char* data[(dim1+1) * (dim2+1)]`

### Indexing
- **1D**: `index = subscript`
- **2D**: `index = row * (dim2+1) + col`

### Bounds Checking
Arrays are bounds-checked at runtime. Out-of-bounds access triggers error or TRAP.

---

## Control Flow Mechanisms

### FOR/NEXT Loop Stack

**Structure**:
```c
typedef struct {
    uint8_t var_slot;       /* Loop variable slot number */
    double limit;           /* TO limit value */
    double step;            /* STEP value (default 1.0) */
    uint32_t loop_start;    /* PC of first instruction in loop body */
} ForLoop;
```

**Stack**: 32-entry stack (supports 32 nested loops)

**Operation**:
1. **FOR**: Pushes loop context to stack
2. **NEXT**: Increments variable, checks limit, jumps back or pops stack
3. **Overflow**: More than 32 nested FOR loops triggers error

### GOSUB/RETURN Stack

**Purpose**: Stores return addresses for subroutine calls

**Structure**: Array of PC values (`uint32_t gosub_stack[64]`)

**Stack Size**: 64 entries (supports 64 nested GOSUBs)

**Operation**:
1. **GOSUB**: Pushes return PC (next instruction after GOSUB)
2. **RETURN**: Pops return PC and jumps to it
3. **Underflow**: RETURN without GOSUB triggers error

### Program Counter (PC)

**Type**: `uint32_t`

**Purpose**: Index of current instruction in code array

**Modification**:
- **Sequential**: `pc++` after each instruction
- **JUMP**: `pc = target`
- **JUMP_IF_FALSE/TRUE**: `pc = condition ? target : pc+1`
- **GOSUB**: Push `pc+1`, then `pc = target`
- **RETURN**: `pc = pop(gosub_stack)`
- **FOR_NEXT**: `pc = condition ? loop_start : pc+1`

---

## Line Number Mapping

### Purpose
BASIC programs use line numbers for GOTO/GOSUB. The line map translates line numbers to instruction addresses.

### Structure
```c
typedef struct {
    uint32_t line_number;    /* BASIC line number */
    uint32_t pc;             /* Instruction address */
} LineMapEntry;
```

### Lookup
- **Binary search** on sorted line_map array
- **GOTO 100**: Find entry where `line_number == 100`, jump to its `pc`
- **Not found**: Error or TRAP

### Implicit Line Numbers
Programs without explicit line numbers use implicit numbering (10, 20, 30, ...).

---

## I/O System

### Channels

**Channel 0**: Console (stdin/stdout)
- INPUT reads from stdin
- PRINT writes to stdout

**Channels 1-7**: Files or devices
- Opened with OPEN statement
- Accessed with GET, PUT, PRINT #n, INPUT #n
- Closed with CLOSE statement

### File Operations

#### OPEN
```
OPEN #channel, mode, aux1, aux2, "device:filename"
```
- Opens file or device on specified channel
- Mode: 4=input, 8=output, 12=update
- Aux1, Aux2: Device-specific parameters

#### CLOSE
```
CLOSE #channel
```
- Closes file on channel, releases resources

#### GET
```
GET #channel, var
```
- Reads one byte from channel into variable

#### PUT
```
PUT #channel, value
```
- Writes one byte (low 8 bits of value) to channel

#### PRINT #
```
PRINT #channel, expression
```
- Writes value to specified channel (like PRINT to stdout)

#### INPUT #
```
INPUT #channel, var
```
- Reads value from specified channel

#### NOTE
```
NOTE #channel, sector_var, byte_var
```
- Gets current file position (sector and byte offset)

#### POINT
```
POINT #channel, sector, byte
```
- Sets file position to sector and byte offset

#### STATUS
```
STATUS #channel, var
```
- Reads device status into variable

#### XIO
```
XIO command, #channel, aux1, aux2, "device"
```
- Extended I/O command (device-specific)

---

## DATA Statement Support

### Storage
```c
const char **data_items;     /* Array of DATA values as strings */
uint32_t data_count;         /* Total number of DATA values */
uint32_t data_ptr;           /* Current read position (0-based) */
```

### READ Statement
1. Reads `data_items[data_ptr]`
2. Converts to numeric or string as needed
3. Increments `data_ptr`
4. Wraps around at end (circular) or errors

### RESTORE Statement
- **RESTORE**: Sets `data_ptr = 0`
- **RESTORE line**: Sets `data_ptr` to first DATA item on/after that line

### Compilation
DATA statements are extracted during compilation and stored separately. They don't generate runtime bytecode.

---

## Error Handling

### Error Codes
Common errors:
- Division by zero
- Out of memory
- Array bounds exceeded
- RETURN without GOSUB
- NEXT without FOR
- File not found
- Syntax error in DATA
- Type mismatch

### TRAP Mechanism

#### Setting Trap Handler
```
TRAP line_number
```
- Sets `trap_line = line_number`
- On error, jump to that line instead of halting

#### Disabling Trap
```
TRAP 40000
```
- Sets `trap_line = -1`
- Errors halt program

#### Error Handling Flow
```
1. Error occurs during bytecode execution
2. If trap_line >= 0:
   a. Look up trap_line in line_map
   b. Set pc to that address
   c. Continue execution
3. Else:
   a. Print error message
   b. Halt VM
```

---

## Execution Loop

### Main Loop
```c
void vm_run(VM *vm) {
    vm->running = 1;
    
    while (vm->running && vm->pc < vm->code_size) {
        Instruction instr = vm->code[vm->pc];
        
        switch (instr.opcode) {
            case OP_PUSH_CONST:
                /* Push constant to stack */
                vm_push_num(vm, vm->constants[instr.operand]);
                vm->pc++;
                break;
                
            case OP_ADD:
                /* Pop two values, add, push result */
                double b = vm_pop_num(vm);
                double a = vm_pop_num(vm);
                vm_push_num(vm, a + b);
                vm->pc++;
                break;
                
            case OP_JUMP:
                /* Unconditional jump */
                vm->pc = instr.operand;
                break;
                
            case OP_END:
                /* End program */
                vm->running = 0;
                break;
                
            /* ... 130+ more opcodes ... */
        }
    }
}
```

### Execution Steps
1. **Fetch**: Read instruction at `code[pc]`
2. **Decode**: Switch on `opcode`
3. **Execute**: Perform operation (stack, variables, arrays, I/O, etc.)
4. **Advance**: Increment `pc` (or jump)
5. **Repeat**: Until `OP_HALT`, `OP_END`, or error

---

## Trigonometric Mode

### State
```c
int deg_mode;    /* 1 = degrees, 0 = radians */
```

### Statements
- **DEG**: Sets `deg_mode = 1`
- **RAD**: Sets `deg_mode = 0`

### Functions Affected
- **SIN, COS, TAN**: Input angle in degrees or radians
- **ATN**: Output angle in degrees or radians

### Conversion
- Degrees to radians: `radians = degrees * (M_PI / 180.0)`
- Radians to degrees: `degrees = radians * (180.0 / M_PI)`

---

## Random Number Generation

### State
```c
/* Uses C standard library rand()/srand() */
```

### RND Function
- **RND(0)**: Returns random number 0.0 ≤ x < 1.0
- **Implementation**: `(double)rand() / (RAND_MAX + 1.0)`

### RANDOMIZE Statement
- **Effect**: Seeds RNG with current time
- **Implementation**: `srand(time(NULL))`

---

## Memory Management

### String Memory
- **Allocation**: All strings dynamically allocated with `strdup()` or `malloc()`
- **Deallocation**: VM frees strings when:
  - String variable reassigned
  - String stack entry popped and not saved
  - Array deallocated
  - VM shutdown

### Automatic Cleanup
- **Variables**: Freed on `OP_CLR` or VM shutdown
- **Arrays**: Freed on `OP_CLR` or VM shutdown
- **Stacks**: Freed on VM shutdown

### Leak Prevention
- Every `strdup()` has corresponding `free()`
- String stack manages ownership carefully
- Array DIM checks for existing array and frees it

---

## Performance Characteristics

### Instruction Dispatch
- **Switch-based**: O(1) in practice (compiler optimizes to jump table)
- **Fixed-width instructions**: No decoding overhead

### Stack Operations
- **Push/Pop**: O(1) amortized (dynamic growth)
- **Typical depth**: < 100 entries for most programs

### Variable Access
- **Direct indexing**: O(1) access via slot number
- **No lookup**: Compiler resolves names to slots

### Array Access
- **Subscript calculation**: O(1) arithmetic
- **Bounds check**: O(1) comparison

### GOTO/GOSUB
- **Binary search**: O(log n) line lookup
- **Direct jump**: O(1) PC update

---

## Initialization and Cleanup

### vm_init()
1. Allocate instruction array
2. Initialize constant pools
3. Allocate variable arrays (set to 0.0 or "")
4. Allocate stacks (initial capacity)
5. Initialize I/O channels (channel 0 = console)
6. Set default state (deg_mode=0, trap_line=-1, etc.)

### vm_load_program()
1. Load bytecode from .abc file
2. Load constants
3. Load string constants
4. Load variable table
5. Load line map
6. Extract DATA items

### vm_run()
Execute main loop until program ends or error

### vm_cleanup()
1. Free all strings in variables and arrays
2. Free arrays
3. Free stacks
4. Close open file channels
5. Free instruction array
6. Free constant pools

---

## Debugging Support

### State Inspection
- **Variables**: Can inspect all 128 slots
- **Stacks**: Can dump numeric and string stack contents
- **PC**: Current instruction address
- **Line Map**: Translate PC to source line number

### Trace Mode
Optional trace output showing:
- Current PC and instruction
- Stack state before/after
- Variable changes
- Control flow transfers

### Breakpoints
Can set breakpoints at:
- Specific PC address
- Specific line number
- Specific instruction opcode

---

## Extensions and Future Work

### Just-In-Time (JIT) Compilation
- **Instruction flags field**: Reserved for type hints
- **Hot path detection**: Identify frequently executed loops
- **Native code generation**: Compile to x86/ARM/etc.

### Optimizations
- **Constant folding**: Already done at compile time
- **Dead code elimination**: Possible with control flow analysis
- **Register allocation**: Convert stack operations to registers

### Advanced Features
- **Debugger integration**: Step, continue, inspect
- **Profiler**: Measure instruction counts, time spent
- **Memory profiler**: Track allocations and leaks

---

## Implementation Files

- **src/vm.h**: VM structure definitions
- **src/vm.c**: Main execution loop and opcode handlers (~2000 lines)
- **src/bytecode.h**: Opcode definitions
- **basset_vm.c**: VM entry point and loader

---

## Summary

The Virtual Machine provides:
- **Dual stack architecture** (numeric and string)
- **128 variable slots** each for numeric and string
- **Dynamic arrays** (1D and 2D, numeric and string)
- **32-level FOR stack** and **64-level GOSUB stack**
- **8 I/O channels** with file operations
- **Line number mapping** for GOTO/GOSUB
- **TRAP error handling**
- **DEG/RAD trigonometric modes**
- **DATA statement support**
- **~140 bytecode opcodes**

The VM faithfully implements classic BASIC semantics while providing a clean, maintainable architecture for execution and future enhancements.
