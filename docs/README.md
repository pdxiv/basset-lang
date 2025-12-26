# Documentation Index

This directory contains comprehensive documentation for the BASIC compiler and virtual machine implementation.

## Implementation Documentation

### Core Architecture Documents

**[Implementation_Overview.md](Implementation_Overview.md)**
- System architecture and design principles
- Compilation pipeline overview
- Component descriptions (tokenizer, parser, compiler, VM)
- Performance characteristics
- Command-line tools reference

**[Token_Reference.md](Token_Reference.md)** ⭐
- Complete catalog of all 100+ tokens
- Token categories and organization
- Detailed tables with hex values, symbols, and descriptions
- Token recognition algorithm
- Usage examples and precedence tables

**[Bytecode_Reference.md](Bytecode_Reference.md)** ⭐
- Complete catalog of ~140 VM opcodes
- Instruction format specification
- Detailed opcode descriptions with stack effects
- Bytecode generation examples
- Execution model explanation

**[Syntax_Tables_Reference.md](Syntax_Tables_Reference.md)** ⭐
- Grammar encoding using BML (BASIC Meta-Language)
- Syntax opcodes and table structure
- Grammar examples with BNF and encodings
- Non-terminal symbols (80+ defined)
- Operator precedence table
- Parser algorithm description
- Extensibility guide (how to add new statements)

**[VM_Architecture.md](VM_Architecture.md)** ⭐
- Virtual machine internal design
- Memory architecture and stacks (dual stack model)
- Variable and array storage
- Control flow mechanisms (FOR/NEXT, GOSUB/RETURN)
- I/O system and channel management
- Error handling and TRAP mechanism
- Execution loop details
- Performance characteristics
- Linux command-line operation

## Quick Start Guide

**New to the project?** Start here:
1. Read [Implementation_Overview.md](Implementation_Overview.md) for architecture overview
2. Browse [Token_Reference.md](Token_Reference.md) to see all supported keywords and operators
3. Check [Bytecode_Reference.md](Bytecode_Reference.md) to understand VM instructions
4. Dive into [Syntax_Tables_Reference.md](Syntax_Tables_Reference.md) for grammar details
5. Explore [VM_Architecture.md](VM_Architecture.md) for execution internals

**Want to extend the language?**
See the "Extensibility" section in [Syntax_Tables_Reference.md](Syntax_Tables_Reference.md) for step-by-step instructions on adding new statements.

## Documentation Coverage

This documentation covers **every facet** of the implementation:

### Tokenization
- 100+ keyword entries in keyword table
- Token recognition algorithm (longest-match, case-insensitive)
- Multi-character operators (`<=`, `<>`, `>=`)
- Special handling (REM comments, string literals, apostrophe alias)

### Syntax Tables
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
- System operations

### Virtual Machine
- Dual stack architecture (numeric and string)
- 128 variable slots each for numeric and string
- Dynamic arrays (1D/2D, up to 32767 per dimension)
- FOR stack (32 levels) and GOSUB stack (64 levels)
- 8 I/O channels with file operations
- TRAP error handling
- DATA statement support
- DEG/RAD trigonometric modes

## Implementation Files Cross-Reference

Documentation → Source code mapping:
- **Tokens**: [src/tokens.h](../src/tokens.h), [src/tokenizer.c](../src/tokenizer.c)
- **Syntax Tables**: [src/syntax_tables.h](../src/syntax_tables.h), [src/syntax_tables.c](../src/syntax_tables.c)
- **Parser**: [src/parser.c](../src/parser.c), [src/parser.h](../src/parser.h)
- **Bytecode**: [src/bytecode.h](../src/bytecode.h)
- **Compiler**: [src/compiler.c](../src/compiler.c), [src/compiler.h](../src/compiler.h)
- **VM**: [src/vm.c](../src/vm.c), [src/vm.h](../src/vm.h)
- **Bytecode File**: [src/bytecode_file.c](../src/bytecode_file.c)

## Summary

This documentation provides:
- **4 comprehensive technical references** (marked with ⭐)
- **Complete coverage** of tokens, bytecodes, syntax tables, and VM
- **Practical examples** and usage patterns
- **Extensibility guide** for adding new features
- **Historical context** via classic BASIC design patterns

All documentation is written comprehensively, detailed, and technically rigorous.

## Reference Notes

- When documents conflict, the syntax tables are authoritative
- Ambiguities are documented in implementation comments
- Test suite in `../tests/` validates compatibility
