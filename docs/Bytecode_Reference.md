# Bytecode Reference - Complete Opcode Catalog

## Overview

This document provides a comprehensive reference for all bytecode opcodes used in the Basset BASIC virtual machine. Each instruction is documented with its hex value, operand requirements, stack effects, and semantic behavior.

## Instruction Format

All bytecode instructions use a fixed 4-byte format:

```c
typedef struct {
    uint8_t  opcode;    /* Operation code (0x00-0x8D) */
    uint8_t  flags;     /* Type hints for future JIT (currently unused) */
    uint16_t operand;   /* Immediate value, offset, or slot number */
} Instruction;
```

## Bytecode Categories

1. **Stack Operations** (0x00-0x06)
2. **Arithmetic** (0x10-0x16)
3. **Comparison** (0x20-0x25)
4. **Logical** (0x26-0x28)
5. **String Operations** (0x30-0x3A)
6. **Array Operations** (0x40-0x49)
7. **Control Flow** (0x50-0x5A)
8. **I/O Operations** (0x60-0x74)
9. **Math Functions** (0x75-0x80)
10. **System** (0x81-0x8D)

---

## Stack Operations (0x00-0x06)

### OP_PUSH_CONST (0x00)
**Push constant to numeric stack**

- **Operand**: Index into constant pool
- **Stack Effect**: `[] → [value]`
- **Description**: Pushes a numeric constant from the constant pool onto the numeric stack

### OP_PUSH_VAR (0x01)
**Push numeric variable value**

- **Operand**: Variable slot number (0-127)
- **Stack Effect**: `[] → [value]`
- **Description**: Reads a numeric variable and pushes its value onto the numeric stack

### OP_POP_VAR (0x02)
**Pop value to numeric variable**

- **Operand**: Variable slot number (0-127)
- **Stack Effect**: `[value] → []`
- **Description**: Pops a value from numeric stack and stores it in the specified variable slot

### OP_DUP (0x03)
**Duplicate top of numeric stack**

- **Operand**: Unused
- **Stack Effect**: `[value] → [value, value]`
- **Description**: Duplicates the top value on the numeric stack

### OP_POP (0x04)
**Discard top of numeric stack**

- **Operand**: Unused
- **Stack Effect**: `[value] → []`
- **Description**: Removes and discards the top value from the numeric stack

### OP_STR_POP_VAR (0x05)
**Pop string to variable**

- **Operand**: Variable slot number (0-127)
- **Stack Effect**: `[string] → []` (string stack)
- **Description**: Pops a string from the string stack and stores it in the specified variable slot

### OP_STR_PUSH_VAR (0x06)
**Push string variable value**

- **Operand**: Variable slot number (0-127)
- **Stack Effect**: `[] → [string]` (string stack)
- **Description**: Reads a string variable and pushes its value onto the string stack

---

## Arithmetic Operations (0x10-0x16)

### OP_ADD (0x10)
**Addition**

- **Operand**: Unused
- **Stack Effect**: `[a, b] → [a+b]`
- **Description**: Pops two values, adds them, pushes result

### OP_SUB (0x11)
**Subtraction**

- **Operand**: Unused
- **Stack Effect**: `[a, b] → [a-b]`
- **Description**: Pops two values (b then a), computes a-b, pushes result

### OP_MUL (0x12)
**Multiplication**

- **Operand**: Unused
- **Stack Effect**: `[a, b] → [a*b]`
- **Description**: Pops two values, multiplies them, pushes result

### OP_DIV (0x13)
**Division**

- **Operand**: Unused
- **Stack Effect**: `[a, b] → [a/b]`
- **Description**: Pops two values (b then a), computes a/b, pushes result
- **Error**: Division by zero triggers error handler or halts

### OP_MOD (0x14)
**Modulo**

- **Operand**: Unused
- **Stack Effect**: `[a, b] → [a mod b]`
- **Description**: Pops two values (b then a), computes remainder of a/b, pushes result

### OP_POW (0x15)
**Exponentiation**

- **Operand**: Unused
- **Stack Effect**: `[base, exponent] → [base^exponent]`
- **Description**: Pops two values (exponent then base), computes base^exponent, pushes result

### OP_NEG (0x16)
**Negation**

- **Operand**: Unused
- **Stack Effect**: `[value] → [-value]`
- **Description**: Pops value, negates it, pushes result

---

## Comparison Operations (0x20-0x25)

All comparison operators pop two values, compare them, and push 1.0 (true) or 0.0 (false).

### OP_EQ (0x20)
**Equal**

- **Stack Effect**: `[a, b] → [1.0 if a==b else 0.0]`

### OP_NE (0x21)
**Not Equal**

- **Stack Effect**: `[a, b] → [1.0 if a!=b else 0.0]`

### OP_LT (0x22)
**Less Than**

- **Stack Effect**: `[a, b] → [1.0 if a<b else 0.0]`

### OP_LE (0x23)
**Less or Equal**

- **Stack Effect**: `[a, b] → [1.0 if a<=b else 0.0]`

### OP_GT (0x24)
**Greater Than**

- **Stack Effect**: `[a, b] → [1.0 if a>b else 0.0]`

### OP_GE (0x25)
**Greater or Equal**

- **Stack Effect**: `[a, b] → [1.0 if a>=b else 0.0]`

---

## Logical Operations (0x26-0x28)

BASIC uses numeric logic: 0 = false, non-zero = true.

### OP_AND (0x26)
**Logical AND**

- **Stack Effect**: `[a, b] → [1.0 if (a!=0 AND b!=0) else 0.0]`
- **Description**: BASIC-style AND (not bitwise)

### OP_OR (0x27)
**Logical OR**

- **Stack Effect**: `[a, b] → [1.0 if (a!=0 OR b!=0) else 0.0]`
- **Description**: BASIC-style OR (not bitwise)

### OP_NOT (0x28)
**Logical NOT**

- **Stack Effect**: `[value] → [1.0 if value==0 else 0.0]`
- **Description**: BASIC-style NOT (not bitwise)

---

## String Operations (0x30-0x3A)

### OP_STR_PUSH (0x30)
**Push string constant**

- **Operand**: Index into string constant table
- **Stack Effect**: `[] → [string]` (string stack)
- **Description**: Pushes a string constant onto the string stack

### OP_STR_CONCAT (0x31)
**String concatenation**

- **Stack Effect**: `[str1, str2] → [str1+str2]` (string stack)
- **Description**: Pops two strings, concatenates them, pushes result

### OP_STR_LEN (0x32)
**String length (LEN)**

- **Stack Effect**: `[string] → [] (string stack), [] → [length] (numeric stack)`
- **Description**: Pops string from string stack, pushes its length to numeric stack

### OP_STR_VAL (0x33)
**String to number (VAL)**

- **Stack Effect**: `[string] → [] (string stack), [] → [value] (numeric stack)`
- **Description**: Pops string, converts to number, pushes to numeric stack

### OP_STR_CHR (0x34)
**Number to character (CHR$)**

- **Stack Effect**: `[code] → [] (numeric stack), [] → [char] (string stack)`
- **Description**: Pops ASCII code, pushes single-character string to string stack

### OP_STR_STR (0x35)
**Number to string (STR$)**

- **Stack Effect**: `[value] → [] (numeric stack), [] → [string] (string stack)`
- **Description**: Pops number, converts to string representation, pushes to string stack

### OP_STR_ASC (0x36)
**Character to ASCII (ASC)**

- **Stack Effect**: `[string] → [] (string stack), [] → [code] (numeric stack)`
- **Description**: Pops string, pushes ASCII code of first character to numeric stack

### OP_STR_LEFT (0x37)
**Left substring (LEFT$)**

- **Stack Effect**: `[string, count] → [substring]`
- **Description**: Pops count from numeric stack, pops string from string stack, pushes leftmost `count` characters to string stack

### OP_STR_RIGHT (0x38)
**Right substring (RIGHT$)**

- **Stack Effect**: `[string, count] → [substring]`
- **Description**: Pops count from numeric stack, pops string from string stack, pushes rightmost `count` characters to string stack

### OP_STR_MID (0x39)
**Mid substring (MID$ with 3 args)**

- **Stack Effect**: `[string, start, length] → [substring]`
- **Description**: Pops length and start from numeric stack, pops string from string stack, pushes substring starting at `start` with `length` characters to string stack

### OP_STR_MID_2 (0x3A)
**Mid substring (MID$ with 2 args)**

- **Stack Effect**: `[string, start] → [substring]`
- **Description**: Like OP_STR_MID but takes substring from `start` to end of string

---

## Array Operations (0x40-0x49)

### OP_ARRAY_GET_1D (0x40)
**Get 1D numeric array element**

- **Operand**: Variable slot number
- **Stack Effect**: `[index] → [value]`
- **Description**: Pops index, pushes array[index]

### OP_ARRAY_SET_1D (0x41)
**Set 1D numeric array element**

- **Operand**: Variable slot number
- **Stack Effect**: `[index, value] → []`
- **Description**: Pops value and index, stores value in array[index]

### OP_ARRAY_GET_2D (0x42)
**Get 2D numeric array element**

- **Operand**: Variable slot number
- **Stack Effect**: `[row, col] → [value]`
- **Description**: Pops col and row, pushes array[row, col]

### OP_ARRAY_SET_2D (0x43)
**Set 2D numeric array element**

- **Operand**: Variable slot number
- **Stack Effect**: `[row, col, value] → []`
- **Description**: Pops value, col, and row, stores value in array[row, col]

### OP_DIM_1D (0x44)
**Declare 1D numeric array**

- **Operand**: Variable slot number
- **Stack Effect**: `[size] → []`
- **Description**: Pops size, allocates array with `size+1` elements (0 to size)

### OP_DIM_2D (0x45)
**Declare 2D numeric array**

- **Operand**: Variable slot number
- **Stack Effect**: `[rows, cols] → []`
- **Description**: Pops cols and rows, allocates array with (rows+1)×(cols+1) elements

### OP_STR_ARRAY_GET_1D (0x46)
**Get 1D string array element**

- **Operand**: Variable slot number
- **Stack Effect**: `[index] → [string]` (numeric then string stack)
- **Description**: Pops index from numeric stack, pushes array[index] to string stack

### OP_STR_ARRAY_SET_1D (0x47)
**Set 1D string array element**

- **Operand**: Variable slot number
- **Stack Effect**: `[index] (numeric), [string] (string) → []`
- **Description**: Pops string from string stack, pops index from numeric stack, stores string in array[index]

### OP_STR_ARRAY_GET_2D (0x48)
**Get 2D string array element**

- **Operand**: Variable slot number
- **Stack Effect**: `[row, col] → [string]`
- **Description**: Pops col and row from numeric stack, pushes array[row, col] to string stack

### OP_STR_ARRAY_SET_2D (0x49)
**Set 2D string array element**

- **Operand**: Variable slot number
- **Stack Effect**: `[row, col] (numeric), [string] (string) → []`
- **Description**: Pops string from string stack, pops col and row from numeric stack, stores string in array[row, col]

---

## Control Flow (0x50-0x5A)

### OP_JUMP (0x50)
**Unconditional jump**

- **Operand**: Target instruction address
- **Description**: Sets program counter to operand value

### OP_JUMP_IF_FALSE (0x51)
**Conditional jump (if false)**

- **Operand**: Target instruction address
- **Stack Effect**: `[condition] → []`
- **Description**: Pops value; jumps to target if value is 0.0 (false)

### OP_JUMP_IF_TRUE (0x52)
**Conditional jump (if true)**

- **Operand**: Target instruction address
- **Stack Effect**: `[condition] → []`
- **Description**: Pops value; jumps to target if value is non-zero (true)

### OP_JUMP_LINE (0x53)
**Jump to line number (GOTO)**

- **Operand**: Line number
- **Description**: Looks up line number in line map, jumps to corresponding instruction address

### OP_GOSUB (0x54)
**Call subroutine (by address)**

- **Operand**: Target instruction address
- **Description**: Pushes return address to GOSUB stack, jumps to target

### OP_GOSUB_LINE (0x55)
**Call subroutine (by line number)**

- **Operand**: Line number
- **Description**: Looks up line number, pushes return address, jumps to target

### OP_RETURN (0x56)
**Return from subroutine**

- **Description**: Pops return address from GOSUB stack, jumps to that address

### OP_ON_GOTO (0x57)
**Computed GOTO**

- **Operand**: Number of target lines
- **Stack Effect**: `[index] → []`
- **Description**: Pops index; reads `operand` line numbers from following instructions; jumps to line[index-1] (1-based)

### OP_ON_GOSUB (0x58)
**Computed GOSUB**

- **Operand**: Number of target lines
- **Stack Effect**: `[index] → []`
- **Description**: Like ON_GOTO but pushes return address first

### OP_FOR_INIT (0x59)
**Initialize FOR loop**

- **Operand**: Variable slot number (loop variable)
- **Stack Effect**: `[start, limit, step] → []`
- **Description**: Pops step, limit, and start values. Initializes loop variable to start. Pushes loop context to FOR stack with limit, step, return address, and variable slot

### OP_FOR_NEXT (0x5A)
**NEXT iteration**

- **Operand**: Variable slot number (loop variable)
- **Description**: Increments loop variable by step. If not past limit, jumps back to start of loop body. Otherwise, pops FOR stack and continues

---

## I/O Operations (0x60-0x74)

### OP_PRINT_NUM (0x60)
**Print numeric value**

- **Stack Effect**: `[value] → []`
- **Description**: Pops value from numeric stack, prints to current output channel (default: screen)

### OP_PRINT_STR (0x61)
**Print string value**

- **Stack Effect**: `[string] → []` (string stack)
- **Description**: Pops string from string stack, prints to current output channel

### OP_PRINT_NEWLINE (0x62)
**Print newline**

- **Description**: Prints newline character to current output channel

### OP_PRINT_SPACE (0x63)
**Print space**

- **Description**: Prints single space to current output channel

### OP_PRINT_TAB (0x64)
**Tab to column**

- **Stack Effect**: `[column] → []`
- **Description**: Pops column number, moves cursor to that column (TAB function in PRINT)

### OP_TAB_FUNC (0x65)
**TAB(n) function**

- **Stack Effect**: `[n] → [n]`
- **Description**: Used internally for TAB function - leaves value on stack for PRINT_TAB

### OP_PRINT_NOSEP (0x66)
**Suppress separator**

- **Description**: Semicolon in PRINT - suppresses spacing between items

### OP_INPUT_NUM (0x67)
**Input numeric value**

- **Operand**: Variable slot number
- **Description**: Prompts for input, reads number, stores in variable

### OP_INPUT_STR (0x68)
**Input string value**

- **Operand**: Variable slot number
- **Description**: Prompts for input, reads string, stores in variable

### OP_INPUT_PROMPT (0x69)
**Display input prompt**

- **Stack Effect**: `[prompt] → []` (string stack)
- **Description**: Pops prompt string, displays it before input

### OP_OPEN (0x6A)
**Open file/device**

- **Stack Effect**: `[channel, mode, aux1, aux2, device] → []`
- **Description**: Opens device string for I/O on channel with specified mode and auxiliary bytes

### OP_CLOSE (0x6B)
**Close file/device**

- **Stack Effect**: `[channel] → []`
- **Description**: Closes the specified I/O channel

### OP_GET (0x6C)
**Read byte from channel**

- **Stack Effect**: `[channel] → []`, stores in variable (operand)
- **Operand**: Variable slot number
- **Description**: Reads one byte from channel, stores as number in variable

### OP_PUT (0x6D)
**Write byte to channel**

- **Stack Effect**: `[channel, value] → []`
- **Description**: Writes one byte (from value) to channel

### OP_NOTE (0x6E)
**Get file position**

- **Stack Effect**: `[channel] → []`, stores in variables
- **Description**: Gets current sector and byte position in file on channel

### OP_POINT (0x6F)
**Set file position**

- **Stack Effect**: `[channel, sector, byte] → []`
- **Description**: Sets file position for channel to specified sector and byte offset

### OP_STATUS (0x70)
**Get device status**

- **Stack Effect**: `[channel] → []`, stores in variable (operand)
- **Operand**: Variable slot number
- **Description**: Reads status of device on channel, stores in variable

### OP_XIO (0x71)
**Extended I/O**

- **Stack Effect**: `[channel, command, aux1, aux2, device] → []`
- **Description**: Performs extended I/O command on device via channel

### OP_DATA_READ_NUM (0x72)
**READ numeric data**

- **Operand**: Variable slot number
- **Description**: Reads next DATA value, stores as number in variable

### OP_DATA_READ_STR (0x73)
**READ string data**

- **Operand**: Variable slot number
- **Description**: Reads next DATA value, stores as string in variable

### OP_SET_PRINT_CHANNEL (0x74)
**Set output channel for PRINT**

- **Stack Effect**: `[channel] → []`
- **Description**: Sets current output channel for subsequent PRINT operations (PRINT #n, ...)

---

## Math Functions (0x75-0x80)

All math functions pop operands from numeric stack and push results to numeric stack.

### OP_FUNC_SIN (0x75)
**Sine function**

- **Stack Effect**: `[angle] → [sin(angle)]`
- **Description**: Computes sine; angle in degrees or radians depending on DEG/RAD mode

### OP_FUNC_COS (0x76)
**Cosine function**

- **Stack Effect**: `[angle] → [cos(angle)]`
- **Description**: Computes cosine; angle in degrees or radians depending on DEG/RAD mode

### OP_FUNC_TAN (0x77)
**Tangent function**

- **Stack Effect**: `[angle] → [tan(angle)]`
- **Description**: Computes tangent; angle in degrees or radians depending on DEG/RAD mode

### OP_FUNC_ATN (0x78)
**Arctangent function**

- **Stack Effect**: `[value] → [arctan(value)]`
- **Description**: Computes arctangent; result in degrees or radians depending on DEG/RAD mode

### OP_FUNC_EXP (0x79)
**Exponential function**

- **Stack Effect**: `[x] → [e^x]`
- **Description**: Computes e raised to power x

### OP_FUNC_LOG (0x7A)
**Natural logarithm**

- **Stack Effect**: `[x] → [ln(x)]`
- **Description**: Computes natural logarithm (base e)

### OP_FUNC_CLOG (0x7B)
**Common logarithm**

- **Stack Effect**: `[x] → [log10(x)]`
- **Description**: Computes common logarithm (base 10)

### OP_FUNC_SQR (0x7C)
**Square root**

- **Stack Effect**: `[x] → [√x]`
- **Description**: Computes square root

### OP_FUNC_ABS (0x7D)
**Absolute value**

- **Stack Effect**: `[x] → [|x|]`
- **Description**: Computes absolute value

### OP_FUNC_INT (0x7E)
**Integer part**

- **Stack Effect**: `[x] → [floor(x)]`
- **Description**: Computes floor (largest integer ≤ x)

### OP_FUNC_RND (0x7F)
**Random number**

- **Stack Effect**: `[dummy] → [random]`
- **Description**: Generates random number 0.0 ≤ random < 1.0

### OP_FUNC_SGN (0x80)
**Sign function**

- **Stack Effect**: `[x] → [sign(x)]`
- **Description**: Returns -1.0 if x < 0, 0.0 if x == 0, 1.0 if x > 0

---

## System Operations (0x81-0x8D)

### OP_TRAP (0x81)
**Set error trap**

- **Operand**: Line number for error handler
- **Description**: Sets trap line for error handling

### OP_TRAP_DISABLE (0x82)
**Disable error trap**

- **Description**: Disables error trapping (TRAP 40000)

### OP_END (0x83)
**End program**

- **Description**: Terminates program execution normally

### OP_STOP (0x84)
**Stop program**

- **Description**: Pauses program execution (can CONT to resume)

### OP_RESTORE (0x85)
**Reset DATA pointer**

- **Description**: Resets DATA read pointer to beginning of DATA statements

### OP_RESTORE_LINE (0x86)
**Reset DATA pointer to line**

- **Operand**: Line number
- **Description**: Resets DATA read pointer to specified line number

### OP_DEG (0x87)
**Set degree mode**

- **Description**: Sets trigonometric functions to use degrees

### OP_RAD (0x88)
**Set radian mode**

- **Description**: Sets trigonometric functions to use radians

### OP_RANDOMIZE (0x89)
**Seed random number generator**

- **Description**: Seeds RNG with current system time

### OP_CLR (0x8A)
**Clear variables**

- **Description**: Clears all variables and arrays, resets DATA pointer

### OP_POP_GOSUB (0x8B)
**Pop GOSUB stack**

- **Description**: Removes top entry from GOSUB stack (POP statement)

### OP_NOP (0x8C)
**No operation**

- **Description**: Does nothing; placeholder instruction

### OP_HALT (0x8D)
**Halt execution**

- **Description**: Stops VM execution immediately

### OP_FUNC_PEEK (0x8E)
**PEEK function**

- **Stack Effect**: `[address] → [value]`
- **Description**: Reads byte from memory address. For safety, always returns 0 in this implementation.

---

## Execution Model

### Stack Architecture

The VM uses **dual stacks**:

1. **Numeric Stack**: For numeric values (double precision floating-point)
2. **String Stack**: For string values (dynamically allocated char*)

Most operations work on one stack or the other. Some operations (like STR$, VAL, etc.) transfer values between stacks.

### Variable Storage

- **128 numeric variable slots** (0-127) - Limit enforced at compile time
- **128 string variable slots** (0-127) - Limit enforced at compile time
- **64 array slots maximum** - Limit enforced at compile time
- Variables are allocated by compiler during AST analysis
- Arrays stored separately with DIM allocation
- Exceeding limits produces clear compile-time error messages

### Control Flow Stacks

1. **FOR Stack** (32 entries): Stores loop contexts (variable, limit, step, return PC)
2. **GOSUB Stack** (64 entries): Stores subroutine return addresses

### Program Counter

- **PC (uint32_t)**: Current instruction index
- Modified by: JUMP, JUMP_IF_*, GOSUB, RETURN, FOR_NEXT, ON_GOTO/GOSUB

### I/O Channels

- **8 channels** (0-7)
- Channel 0: Console (screen/keyboard)
- Channels 1-7: Files or devices
- Current print channel selectable via PRINT #

### Trigonometric Mode

- **deg_mode (bool)**: false = radians, true = degrees
- Affects: SIN, COS, TAN, ATN
- Set by: DEG statement (OP_DEG), RAD statement (OP_RAD)

### Error Handling

- **trap_line (int)**: Line number for error handler (or -1 if disabled)
- Set by: TRAP statement (OP_TRAP)
- On error: Jump to trap_line if set, else halt with error message

---

## Bytecode Generation Examples

### Assignment: `X = 10`
```
OP_PUSH_CONST 0      ; Push constant 10.0
OP_POP_VAR    0      ; Pop to variable X (slot 0)
```

### Expression: `Y = X + 5`
```
OP_PUSH_VAR   0      ; Push X
OP_PUSH_CONST 1      ; Push 5.0
OP_ADD               ; Add
OP_POP_VAR    1      ; Pop to Y
```

### IF Statement: `IF X > 0 THEN PRINT "POSITIVE"`
```
OP_PUSH_VAR     0    ; Push X
OP_PUSH_CONST   0    ; Push 0.0
OP_GT                ; Compare X > 0
OP_JUMP_IF_FALSE 10  ; Jump to end if false
OP_STR_PUSH     0    ; Push "POSITIVE"
OP_PRINT_STR         ; Print it
OP_PRINT_NEWLINE     ; Print newline
; Address 10: continue
```

### FOR Loop: `FOR I=1 TO 10: PRINT I: NEXT I`
```
OP_PUSH_CONST 0      ; Push 1.0 (start)
OP_PUSH_CONST 1      ; Push 10.0 (limit)
OP_PUSH_CONST 2      ; Push 1.0 (step)
OP_FOR_INIT   0      ; Initialize loop (I = slot 0)
; Address 4: loop body start
OP_PUSH_VAR   0      ; Push I
OP_PRINT_NUM         ; Print it
OP_PRINT_NEWLINE     ; Print newline
OP_FOR_NEXT   0      ; Check loop condition, jump back if needed
; Continue after loop
```

---

## Summary Statistics

- **Total Opcodes**: ~140 (0x00 through 0x8D)
- **Stack Operations**: 7
- **Arithmetic**: 7
- **Comparison**: 6
- **Logical**: 3
- **String**: 11
- **Array**: 10
- **Control Flow**: 11
- **I/O**: 21
- **Math Functions**: 13
- **System**: 13

All opcodes are defined in [src/bytecode.h](../src/bytecode.h) and executed by the VM in [src/vm.c](../src/vm.c).
