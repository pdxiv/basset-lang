# Source Code Organization

This directory contains the core library modules for the Classic BASIC compiler and virtual machine.

## Compilation Pipeline Modules

### Frontend (Lexing & Parsing)

**tokenizer.c / tokenizer.h**
- Lexical analysis: converts source text into token stream
- Keyword recognition via lookup table
- Handles operators, numbers, strings, identifiers
- Position tracking for error reporting

**tokens.h**
- Token type definitions (TOK_PRINT, TOK_NUMBER, etc.)
- Compatible with classic BASIC token conventions

**syntax_tables.c / syntax_tables.h**
- Table-driven grammar rules (BML encoding)
- Keyword table mapping text → token IDs
- Operator precedence table
- Syntax rule productions

**parser.c / parser.h**
- Table-driven parser engine
- Builds parse tree (AST) from token stream
- Operator-precedence expression parsing (Pratt-style)
- Dispatches to syntax tables for statement validation

### Backend (Code Generation)

**compiler.c / compiler.h**
- Walks parse tree and generates bytecode
- Line number table generation
- Variable name table generation
- Constant pool management

**bytecode.h**
- Bytecode instruction definitions (opcodes)
- VM instruction set specification

### Runtime (Execution)

**vm.c / vm.h**
- Virtual machine / bytecode interpreter
- Executes compiled bytecode
- Variable storage (numeric and string)
- Control flow stacks (FOR/NEXT, GOSUB/RETURN)
- I/O operations (PRINT, INPUT, file I/O)

### Support Modules

**bytecode_file.c / bytecode_file.h**
- .abc file format reading/writing
- Serialization of bytecode, constants, strings, variables
- File header and validation

**floating_point.c / floating_point.h**
- Numeric operations
- Currently uses C `double` type
- Abstraction layer for potential BCD floating-point

## Module Dependencies

```
tokenizer → parser → compiler → bytecode_file
                                     ↓
                      vm ← bytecode_file
                       ↓
                floating_point
```

All modules use `syntax_tables` for keyword/token mapping.

## Design Principles

1. **Separation of Concerns**: Each module has a single, well-defined responsibility
2. **Table-Driven**: Grammar and token mappings are data, not code
3. **Classic Compatibility**: Token IDs and syntax tables compatible with classic BASIC implementations
4. **K&R C Compliance**: ANSI C (C89) compatible code

## Adding New Features

**New Statement (e.g., WHILE/WEND):**
1. Add tokens to `tokens.h`
2. Add to keyword table in `syntax_tables.c`
3. Add syntax rules in `syntax_tables.c`
4. Add compilation in `compiler.c` (generates opcodes)
5. Add execution in `vm.c` (interprets opcodes)

**New Function (e.g., SQRT):**
1. Add token to `tokens.h`
2. Add to keyword table and function list in `syntax_tables.c`
3. Add function call compilation in `compiler.c`
4. Add function execution in `vm.c`

**New Data Type (e.g., arrays):**
1. Update variable storage in `vm.c`
2. Add array opcodes to `bytecode.h`
3. Update compiler and VM to handle new opcodes
4. Add DIM statement compilation

## File Count

Currently: 11 .c files, 9 .h files (22 total)

As the project grows, consider organizing into subdirectories when file count exceeds ~15-20 files per directory.
