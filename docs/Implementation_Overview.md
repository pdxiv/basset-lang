# BASIC Compiler Implementation Overview

## Introduction

This document provides a comprehensive technical reference for the BASIC compiler and virtual machine implementation. The system draws inspiration from classic table-driven BASIC interpreters, implementing a modern bytecode compiler and virtual machine with compatibility for programs written in Atari BASIC and Microsoft BASIC.

## System Architecture

The implementation consists of several interconnected subsystems:

### 1. Tokenization (Lexical Analysis)
- **Module**: `src/tokenizer.c/h`
- **Purpose**: Converts source text into token stream
- **Features**:
  - 100+ BASIC keywords (REM, DATA, INPUT, PRINT, FOR, etc.)
  - Microsoft BASIC extensions (CLS, RANDOMIZE, DEFINT, etc.)
  - Operator recognition (arithmetic, comparison, logical)
  - String and numeric constant handling
  - Case-insensitive keyword matching
  - Longest-match algorithm for keyword recognition

### 2. Syntax Tables (Grammar Definition)
- **Modules**: `src/syntax_tables.c/h`
- **Purpose**: Defines BASIC grammar as data tables (not code)
- **Design**: Based on BML (BASIC Meta-Language) from classic table-driven BASIC design
- **Components**:
  - **Keyword Table**: Maps text strings to token IDs
  - **Statement Table**: Maps statement tokens to syntax rules
  - **Syntax Rule Tables**: Grammar productions encoded as byte sequences
  - **Non-terminals**: 80+ grammar symbols (NT_EXP, NT_STATEMENT, etc.)
  - **Syntax Opcodes**: SYN_OR (alternatives), SYN_RTN (return), etc.

### 3. Table-Driven Parser
- **Module**: `src/parser.c/h`
- **Purpose**: Generic parser engine consulting syntax tables
- **Design Philosophy**: No per-statement parsing code - all grammar in tables
- **Algorithm**:
  - Reads syntax rules from tables
  - Matches terminals against token stream
  - Recursively processes non-terminals
  - Builds Abstract Syntax Tree (AST)
  - Pratt-style expression parsing with precedence

### 4. Compiler (Code Generation)
- **Module**: `src/compiler.c/h`
- **Purpose**: Transforms AST into bytecode
- **Outputs**:
  - Bytecode instruction sequence (~140 opcodes)
  - Constant pool (numeric literals)
  - String constant table
  - Variable table (names and slot indices)
  - Line number map (for GOTO/GOSUB)
  
### 5. Virtual Machine (Execution Engine)
- **Module**: `src/vm.c/h`
- **Purpose**: Interprets and executes bytecode
- **Architecture**:
  - Dual stacks (numeric and string)
  - Variable storage (128 slots each for numeric/string)
  - Array support (1D/2D, numeric/string, DIM up to 32767)
  - FOR/NEXT loop stack (32 levels)
  - GOSUB/RETURN call stack (64 levels)
  - 8 I/O channels (0=screen, 1-7=files)
  - Trigonometric functions (DEG/RAD mode switching)
  - String manipulation (LEFT$, RIGHT$, MID$, concatenation)
  - Error handling with TRAP support

### 6. Bytecode File Format
- **Module**: `src/bytecode_file.c/h`
- **Purpose**: .abc file serialization/deserialization
- **Format**:
  - Magic number identification
  - Version information
  - Bytecode section
  - Constant pool section
  - String table section
  - Variable table section
  - Line map section

## Compilation Pipeline

```
Source Text (.bas)
    ↓
[Tokenizer] → Token Stream
    ↓
[Parser] → Abstract Syntax Tree (AST)
    ↓
[Compiler] → Bytecode + Tables
    ↓
[Bytecode File] → .abc File
    ↓
[VM Loader] → Load into Memory
    ↓
[VM Executor] → Execute Program
```

## Design Principles

### 1. Table-Driven Architecture
Following classic table-driven BASIC design, grammar rules are encoded as data rather than code. This makes the parser generic and maintainable - adding new statements requires only table updates, not new parsing functions.

### 2. Compatible Token Mapping
Token IDs are compatible with classic BASIC implementations where applicable, maintaining compatibility with historical architecture while extending it for modern features.

### 3. Separation of Concerns
Each subsystem has a clear responsibility:
- Tokenizer: Text → Tokens
- Parser: Tokens → AST (using syntax tables)
- Compiler: AST → Bytecode
- VM: Bytecode → Execution

### 4. Extensibility
The architecture supports:
- New statements (via syntax table additions)
- New functions (via token and opcode additions)
- I/O extensions (channel-based I/O system)
- Future JIT compilation (bytecode flags field reserved)

## Key Features

### Classic BASIC Compatibility
- All standard BASIC statements (PRINT, INPUT, FOR, IF, etc.)
- Graphics statements (PLOT, DRAWTO, POSITION, GRAPHICS)
- Sound statement (SOUND)
- I/O statements (OPEN, CLOSE, GET, PUT, NOTE, POINT, XIO, STATUS)
- Array support (DIM, 1D/2D arrays)
- String functions (LEFT$, RIGHT$, MID$, STR$, CHR$, ASC, VAL, LEN)
- Math functions (SIN, COS, ATN, EXP, LOG, CLOG, SQR, ABS, INT, RND, SGN)
- Control flow (GOTO, GOSUB, ON...GOTO, ON...GOSUB, TRAP)

### Microsoft BASIC Extensions
- CLS (clear screen)
- RANDOMIZE (seed RNG)
- DEFINT, DEFSNG, DEFDBL, DEFLNG, DEFSTR (type declarations - parsed but not enforced)
- CLEAR (alias for CLR)

### Modern Enhancements
- Bytecode compilation for faster execution
- Structured error handling
- File-based I/O with multiple channels
- Separate compilation and execution phases

## Performance Characteristics

- **Tokenization**: Linear O(n) in source length
- **Parsing**: O(n) with table lookups, backtracking rare
- **Compilation**: Single-pass AST traversal O(n)
- **Execution**: Bytecode dispatch overhead minimal (switch-based)

## Memory Layout

### VM Memory Model
```
Constants Pool:     Fixed at compile time
String Constants:   Fixed at compile time
Variables (0-127):  Numeric and string slots
Arrays:            Dynamic allocation on DIM
FOR Stack:         32 entries (loop contexts)
GOSUB Stack:       64 entries (return addresses)
Numeric Stack:     Dynamic growth
String Stack:      Dynamic growth
I/O Channels:      8 file handles
```

## Error Handling

The system supports classic BASIC-style error trapping:
- `TRAP <line>` - Set error handler
- `TRAP 40000` - Disable trapping (continue on error)
- Runtime errors jump to trap line
- No trap = error message and program halt

## File Format Specifications

### Source Files (.bas)
- Plain text BASIC source code
- Line numbers optional (implicit numbering supported)
- Multiple statements per line (separated by `:`)
- Comments via REM or `'`

### Bytecode Files (.abc)
- Binary format for compiled programs
- Platform-independent (portable)
- Includes all necessary runtime data
- Version tagged for compatibility checking

## Command-Line Tools

### basset_asm
Assembles BASIC source to bytecode:
```
basset_asm input.bas -o output.abc
```

### basset_vm
Executes bytecode files:
```
basset_vm program.abc
```

### basset
Combined compiler and executor:
```
basset program.bas
```

## Testing

Comprehensive test suite with 130+ tests:
- **Standard Tests** (110): Core BASIC functionality
- **Error Tests** (14): Error handling and edge cases
- **Tokenizer Tests** (6): Lexical analysis validation

All tests organized in `tests/` with self-contained runners.

## Documentation Structure

- **README.md**: Quick start guide
- **QUICKSTART.md**: Getting started tutorial
- **STRUCTURE.md**: Project organization
- **Implementation_Overview.md**: This document
- **Token_Reference.md**: Complete token catalog
- **Bytecode_Reference.md**: Complete opcode catalog
- **Syntax_Tables_Reference.md**: Grammar encoding details
- **VM_Architecture.md**: Virtual machine internals

## Next Steps

For detailed technical information, consult:
1. **Token_Reference.md** - All 100+ tokens with IDs and usage
2. **Bytecode_Reference.md** - All ~140 opcodes with hex values and semantics
3. **Syntax_Tables_Reference.md** - Grammar rule encoding and BML opcodes
4. **VM_Architecture.md** - Virtual machine execution model and data structures
