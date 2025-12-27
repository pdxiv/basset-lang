# Documentation Index

This directory contains comprehensive technical documentation for Basset BASIC.

## Quick Start

**New to the project?** Start here:
1. **[Architecture.md](Architecture.md)** - System overview and compilation pipeline
2. **[Parser.md](Parser.md)** - Enum-based expression parser design
3. **[Grammar_Reference.md](Grammar_Reference.md)** - How grammar is encoded
4. **[Virtual_Machine.md](Virtual_Machine.md)** - VM internals and execution

## Architecture Documentation

**[Architecture.md](Architecture.md)**
- System architecture and design principles  
- Compilation pipeline (tokenize → parse → compile → execute)
- Component descriptions (tokenizer, parser, compiler, VM)
- Performance characteristics
- Command-line tools reference

**[Parser.md](Parser.md)**
- Enum-based data-driven Pratt parser implementation
- Expression parsing architecture (nud/led pattern)
- Comparison: enum dispatch vs function pointers
- Zero-warning compile strategy
- Parse action types and operator table structure

**[Virtual_Machine.md](Virtual_Machine.md)**
- Virtual machine internal design
- Memory architecture and stacks (tagged value stack model)
- Variable and array storage
- Control flow mechanisms (FOR/NEXT, GOSUB/RETURN)
- I/O system and channel management
- Error handling (TRAP mechanism, ERR function, error codes)
- DATA/READ/RESTORE support
- Execution loop details

## Reference Documentation

**[Token_Reference.md](Token_Reference.md)**
- Complete catalog of all 100+ tokens
- Token categories and organization
- Detailed tables with hex values, symbols, and descriptions
- Token recognition algorithm
- Usage examples and precedence tables

**[Bytecode_Reference.md](Bytecode_Reference.md)**
- Complete catalog of ~140 VM opcodes
- Instruction format specification
- Detailed opcode descriptions with stack effects
- Bytecode generation examples
- Execution model explanation

**[Grammar_Reference.md](Grammar_Reference.md)**
- Grammar encoding using BML (BASIC Meta-Language)
- Syntax opcodes and table structure
- Grammar examples with BNF and encodings
- Non-terminal symbols (80+ defined)
- Operator precedence table
- Parser algorithm description
- Extensibility guide (how to add new statements)

## Developer Guides

**[Extending_Guide.md](Extending_Guide.md)**
- Step-by-step guide for adding new language features
- Example: Adding a new statement type
- Token addition, grammar encoding, bytecode generation
- Testing and validation

## Implementation Coverage

This documentation covers **every facet** of the implementation:

### Tokenization
- 100+ keyword entries in keyword table
- Token recognition algorithm (longest-match, case-insensitive)
- Multi-character operators (`<=`, `<>`, `>=`)
- Special handling (REM comments, string literals, apostrophe alias)

### Grammar
- 80+ non-terminal grammar symbols
- BML encoding (SYN_OR, SYN_RTN, SYN_ANTV, etc.)
- Statement-specific syntax rules
- Operator precedence table (16 operators)

### Bytecode
- ~140 opcodes (0x00-0x8D) across 10 categories
- Stack operations, arithmetic, comparison, logical
- String operations, array operations
- Control flow (JUMP, FOR/NEXT, GOSUB/RETURN)
- I/O operations (21 opcodes)
- Math functions (13 opcodes)

### Virtual Machine
- Tagged value stack architecture (matches JVM, CLR, Lua, Python)
- 128 variable slots each for numeric and string
- Dynamic arrays (1D/2D, up to 32767 per dimension)
- FOR stack (32 levels) and GOSUB stack (64 levels)
- 8 I/O channels with file operations
- TRAP error handling with ERR function
- DATA/READ/RESTORE statement support

## Source Code Cross-Reference

Documentation → Source mapping:
- **Tokens**: [../src/tokens.h](../src/tokens.h), [../src/tokenizer.c](../src/tokenizer.c)
- **Grammar**: [../src/syntax_tables.h](../src/syntax_tables.h), [../src/syntax_tables.c](../src/syntax_tables.c)
- **Parser**: [../src/parser.c](../src/parser.c), [../src/parser.h](../src/parser.h)
- **Bytecode**: [../src/bytecode.h](../src/bytecode.h)
- **Compiler**: [../src/compiler.c](../src/compiler.c), [../src/compiler.h](../src/compiler.h)
- **VM**: [../src/vm.c](../src/vm.c), [../src/vm.h](../src/vm.h)
