# Virtual Machine Architecture

## Overview

The Basset BASIC Virtual Machine (VM) is a bytecode interpreter that executes compiled BASIC programs. It implements a **stack-based architecture with tagged values**, variable storage, array support, and control flow mechanisms.

## Design Principles

1. **Stack-Based**: Operations work on a stack rather than registers
2. **Tagged Values**: Each stack entry carries both a value and a type tag (NUMBER or STRING), allowing the single stack to safely hold different data types
3. **Type Safety**: Runtime type checking with clear error messages
4. **Fixed-Width Instructions**: All instructions are 4 bytes
5. **Backward Compatibility**: Implements classic BASIC semantics faithfully
6. **Industry Standard**: Architecture matches modern VMs (JVM, CLR, Lua, Python)

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
    
    /* Expression Stack */
    Value *stack;                /* Stack for numbers and strings */
    int stack_top;               /* Stack pointer */
    int stack_capacity;          /* Stack capacity */
    
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
    int error_code;              /* Last error code (for ERR function) */
    int trap_line;               /* TRAP error handler line (-1 = disabled) */
    int trap_enabled;            /* 1 if TRAP is active */
    int trap_triggered;          /* 1 if trap was just triggered */
    int deg_mode;                /* Trig mode: 1=degrees, 0=radians */
    
} VM;
```

---

## Stack Architecture

### Expression Stack

**Purpose**: Stack that holds both numeric and string values during expression evaluation using tagged values.

**Value Type Definition:**
```c
typedef enum {
    VAL_NUMBER,     /* Numeric value (double) */
    VAL_STRING      /* String value (char*) */
} ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        char *string;
    } data;
} Value;
```

**Operations**:
- **PUSH**: `stack[stack_top++] = value` (with type tag)
- **POP**: `value = stack[--stack_top]` (returns tagged value)
- **PEEK**: `value = stack[stack_top - 1]` (no change to stack)

**Type Safety:**
- Helper functions (`vm_pop_number()`, `vm_pop_string()`) validate types
- Raises "TYPE MISMATCH" error if wrong type extracted
- Memory management: strings are heap-allocated and properly freed

**Growth**: Dynamic allocation, expands as needed

**Usage Examples**:
- Numeric expression: `X + Y * 2`
  ```
  PUSH_NUMBER X     → [{NUM, X}]
  PUSH_NUMBER Y     → [{NUM, X}, {NUM, Y}]
  PUSH_NUMBER 2     → [{NUM, X}, {NUM, Y}, {NUM, 2}]
  MUL               → [{NUM, X}, {NUM, Y*2}]
  ADD               → [{NUM, X+Y*2}]
  ```

- String concatenation: `"HELLO" + " " + "WORLD"`
  ```
  PUSH_STRING "HELLO"  → [{STR, "HELLO"}]
  PUSH_STRING " "      → [{STR, "HELLO"}, {STR, " "}]
  CONCAT               → [{STR, "HELLO "}]
  PUSH_STRING "WORLD"  → [{STR, "HELLO "}, {STR, "WORLD"}]
  CONCAT               → [{STR, "HELLO WORLD"}]
  ```

- Mixed operations (comparison returns number):
  ```
  PUSH_STRING "ABC"    → [{STR, "ABC"}]
  PUSH_STRING "XYZ"    → [{STR, "ABC"}, {STR, "XYZ"}]
  CMP_LT               → [{NUM, 1.0}]  (ABC < XYZ is true)
  ```

**Architecture Benefits:**
- **Industry Standard:** Matches JVM, CLR, Lua, Python, JavaScript V8
- **Simplified:** Single stack pointer, unified memory management
- **Extensible:** Easy to add new value types (arrays, objects, etc.)
- **Type Safe:** Runtime type checking with clear error messages

---

## Variable Storage

### Numeric Variables
- **Slots**: 128 slots (0-127) - **Limit enforced at compile time**
- **Type**: `double` (IEEE 754 double-precision)
- **Initial Value**: 0.0
- **Access**: Direct array indexing
- **Compiler**: Assigns slot numbers during compilation
- **Limit Check**: Compilation fails with clear error if exceeded

### String Variables
- **Slots**: 128 slots (0-127) - **Limit enforced at compile time**
- **Type**: `char*` (dynamically allocated)
- **Initial Value**: Empty string `""`
- **Memory**: Automatic deallocation when reassigned
- **Compiler**: Assigns separate slot numbers from numeric variables
- **Limit Check**: Compilation fails with clear error if exceeded

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
- **Maximum Arrays**: 64 total arrays - **Limit enforced at compile time**
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
3. **Variable Validation**: NEXT checks that variable matches FOR variable, produces descriptive error if mismatch
4. **Overflow**: More than 32 nested FOR loops triggers error

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

The VM maintains an `error_code` field that stores the last error code for use with the ERR function. Error codes follow Microsoft BASIC conventions:

| Code | Constant | Description |
|------|----------|-------------|
| 0 | ERR_NONE | No error |
| 1 | ERR_NEXT_WITHOUT_FOR | NEXT without FOR |
| 3 | ERR_RETURN_WITHOUT_GOSUB | RETURN without GOSUB |
| 4 | ERR_OUT_OF_DATA | Out of DATA |
| 5 | ERR_ILLEGAL_FUNCTION | Illegal function call |
| 6 | ERR_OVERFLOW | Overflow |
| 9 | ERR_SUBSCRIPT_RANGE | Subscript out of range |
| 11 | ERR_DIVISION_BY_ZERO | Division by zero |
| 13 | ERR_TYPE_MISMATCH | Type mismatch |
| 52 | ERR_BAD_FILE_NUMBER | Bad file number |
| 53 | ERR_FILE_NOT_FOUND | File not found |
| 57 | ERR_DEVICE_IO | Device I/O error |

### ERR Function

The `ERR` function returns the error code of the last error that occurred:

```basic
10 TRAP 1000
20 X = 1 / 0
30 END
1000 PRINT "ERROR CODE: "; ERR
1010 IF ERR = 11 THEN PRINT "DIVISION BY ZERO"
1020 END
```

- Returns 0 if no error has occurred
- Stores the error code when any runtime error happens
- Works in conjunction with TRAP for sophisticated error handling
- Error code persists until the next error occurs

### TRAP Mechanism

The TRAP mechanism allows programs to intercept runtime errors and handle them gracefully instead of halting.

#### VM State Fields
```c
int trap_enabled;        /* 1 if TRAP is active, 0 if disabled */
int trap_line;           /* PC offset of trap handler line */
int trap_triggered;      /* 1 if error occurred and trap was invoked */
int error_code;          /* Last error code (for ERR function) */
```

#### Setting Trap Handler
```basic
TRAP line_number
```
- Sets `trap_enabled = 1`
- Sets `trap_line` to PC offset of the target line
- On error, execution jumps to that line instead of halting

#### Disabling Trap
```basic
TRAP 40000
```
- Sets `trap_enabled = 0`
- Sets `trap_line = -1`
- Errors will halt program with error message

#### Trappable Errors
All runtime errors can be trapped:
- **Type mismatch** - Wrong type passed to operation (e.g., `LEN(42)`)
- **Division by zero** - Divide or MOD by zero
- **Array bounds** - Index out of range
- **RETURN without GOSUB** - Return stack underflow
- **NEXT without FOR** - FOR stack underflow
- **FOR/NEXT mismatch** - Variable doesn't match
- **File errors** - File not found, I/O errors
- **DATA errors** - READ beyond available DATA items
- **Invalid operation** - Invalid function arguments

#### Error Handling Flow
```
1. Error occurs during instruction code and error message
3. Error code is stored in vm->error_code (for ERR function)
4. Error message is printed to stderr
5. If trap_enabled && trap_line >= 0:
   a. Set vm->pc = trap_line (jump to handler)
   b. Set vm->trap_triggered = 1 (signal PC was changed)
   c. Set vm->trap_enabled = 0 (disable to prevent infinite loop)
   d. Return to instruction (which checks trap_triggered and breaks)
6. Else: halt program with error message
```

#### Using ERR with TRAP
```basic
10 TRAP 1000
20 REM Try division by zero
30 X = 1 / 0
40 PRINT "This line never executes"
50 END
1000 REM Error handler
1010 PRINT "Error "; ERR; " occurred"
1020 IF ERR = 11 THEN PRINT "Division by zero"
1030 IF ERR = 13 THEN PRINT "Type mismatch"
1040 IF ERR = 9 THEN PRINT "Array bounds"
1050 END
```= 0 (disable to prevent infinite loop)
   d. Return to instruction
5. Else (no trap):
   a. Set vm->running = 0 (halt program)
   b. Return to instruction
```

#### Instruction Trap Handling
Instructions that can trigger errors must check `trap_triggered` after type-checking operations:

```c
case OP_STR_LEN:
    char *str = vm_pop_string(vm);  // May call vm_error() if wrong type
    if (vm->trap_triggered) break;   // Skip rest if error trapped
    int len = strlen(str);
    vm_push_number(vm, (double)len);
    vm->pc++;
    break;
```

Without this check, the instruction would continue executing and increment PC, skipping the first instruction of the trap handler.

#### Re-enabling TRAP
After handling an error, the trap handler can re-enable TRAP for subsequent errors:

```basic
10 REM Main program
20 TRAP 1000
30 X = LEN(42)          : REM Triggers trap
40 PRINT "After trap"
50 END

1000 REM Error handler
1010 PRINT "Error caught"
1020 TRAP 1000           : REM Re-enable for next error
1030 RETURN              : REM Or continue execution
```

#### Multiple TRAP Handlers
Programs can chain different trap handlers for different error scenarios:

```basic
10 TRAP 200
20 A = LEN(42)           : REM Type mismatch → TRAP 200
30 PRINT "Test 1 passed"

200 PRINT "Handler 1"
210 TRAP 300             : REM Set new handler
220 B = "X" + 5          : REM Type mismatch → TRAP 300
230 PRINT "Test 2 passed"

300 PRINT "Handler 2"
310 PRINT "All tests passed"
320 END
```

#### TRAP Behavior Details

**Automatic Disable**: TRAP is automatically disabled after triggering to prevent infinite loops. If the trap handler itself causes an error without re-enabling TRAP, the program halts.

**No Error Code**: Unlike some BASIC implementations, the VM doesn't expose error codes to the BASIC program. Trap handlers must infer the error type from context or error message output.

**Error Messages**: All errors print descriptive messages to stderr before invoking the trap handler, even when trapped. This aids debugging.

**Control Flow**: After handling an error, the trap handler can:
- **END** - Terminate program gracefully
- **GOTO line** - Continue execution elsewhere
- **RETURN** - Only if trap was called from within a GOSUB context

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
  - Stack entry popped and not saved to a variable
  - Array deallocated
  - VM shutdown

### Automatic Cleanup
- **Variables**: Freed on `OP_CLR` or VM shutdown
- **Arrays**: Freed on `OP_CLR` or VM shutdown
- **Stack**: Freed on VM shutdown

### Leak Prevention
- Every `strdup()` has corresponding `free()`
- Stack manages string ownership carefully
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
- **Stack**: Can dump stack contents (shows tagged values)
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
- **Direct address jumps**: GOTO/GOSUB use PC addresses, not line number lookups
- **Address tables**: ON...GOTO/GOSUB store pre-resolved addresses
- **Forward reference resolution**: All line number targets resolved during compilation
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
- **128 variable slots** each for numeric and string (enforced at compile time)
- **64 array slots maximum** (enforced at compile time)
- **Dynamic arrays** (1D and 2D, numeric and string)
- **32-level FOR stack** and **64-level GOSUB stack**
- **8 I/O channels** with file operations
- **Direct address jumps** for GOTO/GOSUB (no runtime line lookups)
- **Optimized ON...GOTO/GOSUB** with pre-built address tables
- **TRAP error handling** with descriptive error messages
- **FOR/NEXT variable mismatch detection**
- **DEG/RAD trigonometric modes**
- **DATA statement support**
- **~140 bytecode opcodes**

The VM faithfully implements classic BASIC semantics while providing a clean, maintainable architecture for execution and future enhancements.
