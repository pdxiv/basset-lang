# Basset BASIC

A table-driven BASIC compiler and virtual machine implemented in ANSI C89 for Linux.

## Overview

Basset BASIC is a from-scratch implementation of a BASIC language compiler that aims for source compatibility with classic 8-bit BASIC implementations, including Atari BASIC and Microsoft BASIC. The implementation uses a **data-driven architecture** where grammar rules, operator precedence, and function metadata are encoded in data tables rather than hard-coded in parser logic.

### Key Features

- **Enum-Based Pratt Parser**: Expression parsing uses data-driven enum dispatch with zero compiler warnings
- **Function Metadata Table**: All 27 built-in functions with automatic arity validation
- **Table-Driven Grammar**: Statement syntax rules encoded as BML (BASIC Meta-Language) bytecode
- **Bytecode VM**: Fast bytecode interpreter with **industry-standard tagged stack** (matches JVM, CLR, Lua, Python)
- **Comprehensive Tests**: 148 tests covering table validation, functionality, error handling, and tokenization
- **Performance Optimizations**: O(1) keyword lookup, consolidated utilities, modern VM architecture

See [IMPROVEMENTS_COMPLETED.md](IMPROVEMENTS_COMPLETED.md) for details on recent code quality improvements.

## Quick Start

Build everything:

```bash
make
```

Run a BASIC program:

```bash
./basset_compile tests/hello.bas && ./basset_vm tests/hello.abc
```

Run all tests:

```bash
make test
```

## Building

Build all tools:

```bash
make
```

This produces:

- `basset_compile` - Basset BASIC compiler (source → bytecode)
- `basset_vm` - Basset BASIC virtual machine
- `basset_disasm` - Bytecode disassembler
- `basset_asm` - Bytecode assembler
- `basset_tokenize` - Tokenizer debugging tool

Build individual components:

```bash
make basset_compile   # Compiler only
make basset_vm        # VM only
make basset_disasm    # Disassembler only
make basset_asm       # Assembler only
make basset_tokenize  # Tokenizer debugger only
```

## Tools

### Compiler & VM

Compile a BASIC program to bytecode:

```bash
./basset_compile source.bas output.abc
```

Run compiled bytecode:

```bash
./basset_vm output.abc
```

### Tokenizer Debugger

The `basset_tokenize` tool displays the tokenization of a BASIC source file. This is extremely useful for debugging lexer issues, verifying keyword recognition, and understanding how source code is broken into tokens.

**Usage:**

```bash
./basset_tokenize <source.bas>
```

**Example:**

```bash
./basset_tokenize tests/hello.bas
```

**Output format:**

```text
Tokenizing: tests/hello.bas
════════════════════════════════════════════════════════════
Line  Col    Type          Name                  Value/Text
════════════════════════════════════════════════════════════
1     0      0x6B/107     NUMBER                10
1     3      0x20/32      PRINT                 
1     9      0x6C/108     STRING                "Hello, World!"
1     24     0x54/84      CR                    
2     0      0x6B/107     NUMBER                20
2     3      0x15/21      END                   
2     6      0x54/84      CR                    
3     0      TOK_EOF       EOF                 
════════════════════════════════════════════════════════════
Total tokens: 8
```

Each line shows:

- **Line/Col**: Position in source file (1-based line, 0-based column)
- **Type**: Token type as hex/decimal values
- **Name**: Human-readable token name (PRINT, NUMBER, STRING, etc.)
- **Value/Text**: For numbers, the numeric value; for strings/identifiers, the text content

**Use cases:**

- Verify that keywords are recognized correctly (e.g., `TAB` → TOK_TAB)
- Debug tokenization issues (wrong token types, missing tokens)
- Understand how the lexer handles edge cases
- Check token positions for error reporting

### Disassembler

View compiled bytecode in human-readable format:

```bash
./basset_disasm bytecode.abc
```

### Assembler

Assemble bytecode from assembly format:

```bash
./basset_asm input.asm output.abc
```

## Testing

Comprehensive test suite with 148 tests:

```bash
make test                    # Run all test suites
./tests/run_all.sh          # Alternative (same result)

# Individual test suites:
./tests/standard/run.sh     # 127 functional tests
./tests/errors/run.sh       # 15 error detection tests
./tests/tokenizer/run.sh    # 6 tokenizer tests
```

Test organization:

- **Standard tests** (`tests/standard/`): Validate program execution
- **Error tests** (`tests/errors/`): Verify compile-time error detection
- **Tokenizer tests** (`tests/tokenizer/`): Check lexical analysis

See [tests/README.md](tests/README.md) for detailed test documentation.

### Test Coverage

**Standard Test Suite** (127 tests)

- Statements, expressions, control flow, I/O, functions
- Includes TAB function tests and ERR function tests

**Error Test Suite** (15 tests)

- Compile-time error detection
- Tests undefined labels, syntax errors, invalid constructs

**Tokenizer Test Suite** (6 tests)

- Verifies keyword recognition, operator parsing, function tokens
- Regression tests for tokenizer bugs (e.g., TAB token recognition)

**Run all tests:**

```bash
make test                # All test suites (148 tests total)
make test-standard       # Just standard tests (127 tests)
make test-errors         # Just error tests (15 tests)
make test-tokenizer      # Just tokenizer tests (6 tests)
```

**Current status**: ✅ 148/148 tests passing (100%)

## Usage

Compile and run a BASIC program:

```bash
./basset_compile source.bas
./basset_vm source.abc
```

Or with custom output file:

```bash
./basset_compile source.bas output.abc
./basset_vm output.abc
```

Debug tokenization:

```bash
./basset_tokenize source.bas
```

Example:

```bash
./basset_compile tests/hello.bas
./basset_vm tests/hello.abc
```

## Architecture

This implementation uses a **table-driven architecture** throughout, where behavior is defined by data tables rather than hard-coded logic. Both the parser (~90% table-driven) and compiler (100% table-driven) use dispatch tables to handle different language constructs.

The system consists of several key components:

### 1. Tokenizer (`tokenizer.c`)

Converts source text into a stream of tokens. Implements:

- Keyword recognition using a lookup table
- Multi-character operator detection (`<=`, `<>`, `>=`)
- String literal parsing
- Numeric constant parsing
- Identifier recognition

### 2. Syntax Tables (`syntax_tables.c`)

The heart of the table-driven approach. Contains:

- **Keyword Table**: Maps text strings to token IDs (e.g., "PRINT" → `TOK_PRINT`)
- **Operator Precedence Table**: Defines operator precedence for expression parsing
- **Syntax Rules**: Encoded grammar productions for statements and expressions

The syntax table uses opcodes inspired by table-driven BASIC interpreters:

| Opcode | Name | Description |
|--------|------|-------------|
| `SYN_ANTV` | Absolute Non-Terminal Vector | Branch to another grammar rule |
| `SYN_ESRT` | External Subroutine | Call auxiliary syntax handler |
| `SYN_OR` | Alternative | Choice between productions (BNF `|`) |
| `SYN_RTN` | Return | End of grammar rule (BNF `#`) |
| `SYN_NULL` | Null/Accept | Allow empty production (BNF `&`) |
| `SYN_VEXP` | Vector Expression | Special expression parsing |
| `SYN_CHNG` | Change Token | Modify last token output |

### 3. Parser (`parser.c`)

A table-driven parser engine (~90% table-driven) that:

- Consults syntax tables to validate statement structure
- Uses operator precedence for expression parsing (Pratt-style)
- Builds a parse tree (AST) representation  
- Handles non-terminals by dispatching to appropriate syntax rules
- Uses grammar rules with proper non-terminals (NT_REM_BODY, NT_DATA_LIST, etc.)

**Special cases** (1 of 50+ statements):
- PRINT: Complex alternating expression/separator syntax with trailing separator detection

**Recent improvements**: REM and DATA now use proper grammar rules instead of hard-coded parsing logic, making the parser more consistent and maintainable.

**Key principle**: The parser is generic. To add support for a new statement type, you primarily edit the syntax tables, not the parser code itself.

### 4. Compiler (`compiler.c`)

Table-driven bytecode compiler (100% table-driven dispatch) that:

- Uses a compilation dispatch table to map statement tokens to compiler functions
- Each statement type has a focused, dedicated compiler function
- Generates optimized bytecode for the virtual machine
- Handles forward references for GOTO/GOSUB target resolution
- Manages constant pools, variable symbol tables, and DATA statements

**Recent improvements**: Replaced a 405-line switch statement with a clean dispatch table, making it trivial to add new statements (just add one function + one table entry).

**Key principle**: Adding a new statement requires writing one compiler function and adding one entry to the compilation_table. No modification to core compilation logic needed.

### 5. Virtual Machine (`vm.c`)

Stack-based bytecode interpreter with **tagged values** (matches industry standards like JVM, CLR, Lua, Python) that executes:

- Arithmetic and logical operations
- Variable access and assignment
- Control flow (jumps, loops, subroutines)
- Built-in functions (SIN, COS, INT, etc.)
- I/O operations

Manages:

- **Expression stack** (tagged values with runtime type checking)
- Variable storage (numeric and string variables, arrays)
- FOR/NEXT loop stack  
- GOSUB/RETURN call stack
- DATA statement read pointer

### 6. Floating Point (`floating_point.c`)

Provides numeric operations. Uses standard C `double` type for simplicity and portability.

## Compatibility

### Supported Features (Current Implementation)

**Core Language:**

- ✓ Tokenization matching classic BASIC conventions
- ✓ Table-driven syntax validation
- ✓ Operator precedence (exponentiation, multiplication, addition, comparison, logical)
- ✓ Numeric expressions and arithmetic operators (`+`, `-`, `*`, `/`, `^`)
- ✓ Comparison operators (`=`, `<`, `>`, `<=`, `>=`, `<>`)
- ✓ Logical operators (`AND`, `OR`, `NOT`)
- ✓ Variable assignments (numeric and string)
- ✓ Comments via REM (and apostrophe alias)

**Statements:**

- ✓ PRINT statement (and `?` shorthand, with TAB, semicolon, comma)
- ✓ INPUT statement (numeric and string variables)
- ✓ LET statement (explicit and implicit assignment)
- ✓ END statement
- ✓ STOP statement
- ✓ NEW, CLR, CLEAR statements
- ✓ DIM statement (1D and 2D arrays, numeric and string)
- ✓ TRAP statement (error handling)
- ✓ POKE statement
- ✓ NOTE, POINT statements (file positioning)

**Control Flow:**

- ✓ FOR/NEXT loops (with optional STEP)
- ✓ IF/THEN/ELSE conditionals (with line numbers or statements)
- ✓ GOTO statement
- ✓ GOSUB/RETURN subroutines
- ✓ ON...GOTO and ON...GOSUB (computed branches)
- ✓ POP statement (stack cleanup)

**File I/O:**

- ✓ OPEN statement (file channels)
- ✓ CLOSE statement
- ✓ PRINT# statement (write to files)
- ✓ INPUT# statement (read from files)
- ✓ PUT and GET statements
- ✓ XIO statement (extended I/O)
- ✓ STATUS function

**Functions:**

- ✓ Math functions: SIN, COS, ATN, LOG, CLOG, SQR, SGN, ABS, INT, EXP
- ✓ Utility functions: RND, FRE, PEEK
- ✓ Joystick functions: PADDLE, STICK, PTRIG, STRIG
- ✓ String functions: STR$, CHR$, ASC, VAL, LEN, ADR
- ✓ Substring functions: LEFT$, RIGHT$, MID$
- ✓ TAB function (print formatting)

**Data Types:**

- ✓ Numeric variables (floating-point)
- ✓ String variables
- ✓ Numeric arrays (1D and 2D)
- ✓ String arrays (1D and 2D)

**Advanced Features:**

- ✓ Multi-statement lines (colon separator)
- ✓ Nested IF statements
- ✓ Array indexing (0-based allocation, 1-based string indexing)
- ✓ File channel management (8 channels)
- ✓ Error trapping
- ✓ DEG/RAD trigonometric modes

### Limitations

1. **Graphics/Sound**: Statements parse but don't produce visual/audio output (GRAPHICS, PLOT, POSITION, DRAWTO, SETCOLOR, SOUND)

2. **Floating-point format**: Uses C doubles. Numeric results may differ slightly from original 8-bit BASIC implementations

3. **Error messages**: Error reporting is simplified compared to full TRAP handling in classic BASIC

4. **Platform-specific functions**: Functions like PADDLE, STICK, PTRIG, STRIG return dummy values

5. **Null DATA items**: Empty values in DATA statements (e.g., `DATA 1,,3`) are not currently supported

6. **DATA identifiers**: Unquoted identifiers in DATA statements (e.g., `DATA ABC, XYZ`) are converted to numeric 0, not stored as strings

7. **READ type conversion**: Reading numeric DATA into string variables converts to string (per Microsoft BASIC spec). Reading string DATA into numeric variables produces TYPE MISMATCH error

## Syntax Table Mapping

The syntax tables in `syntax_tables.c` use a BML (BASIC Meta Language) grammar encoding compatible with classic table-driven BASIC implementations.

Example grammar rule encoding:

```text
BML: <LET> = <NVAR> = <EXP> <EOS> | <SVAR> = <STR> <EOS> #
```

Becomes:

```c
static const SyntaxEntry syn_let[] = {
    {SYN_OR, 0, 0, 0},
    SYN_NT(NT_NVAR),
    SYN_TOK(TOK_CEQ),
    {SYN_VEXP, 0, 0, 0},
    SYN_NT(NT_EOS),
    {SYN_OR, 0, 0, 0},
    SYN_NT(NT_SVAR),
    SYN_TOK(TOK_CEQ),
    SYN_NT(NT_STR),
    SYN_NT(NT_EOS),
    SYN_END
};
```

- `test_input.bas` - INPUT with FOR loop (using `?` shorthand)
- `test_sum.bas` - INPUT with FOR loop (using PRINT)
- `test_for.bas` - FOR/NEXT loop tests
- `test_gosub.bas` - GOSUB/RETURN subroutine tests
- `test_if.bas` - IF/THEN conditional tests

Run tests:

```bash
./basset tests/hello.bas
./basset tests/arithmetic.bas

# Interactive tests (require input)
echo -e "10\n20\n30\n40\n50" | ./basset tests/test_sum.bas
```

Example working program:

```basic
10 ? "ENTER 5 NUMBERS TO BE SUMMED"
20 FOR N=1 TO 5
30 INPUT X
40 C=C+X
50 NEXT N
60 ? "THE SUM OF THE NUMBERS IS ";C
70 ENDriable assignment and arithmetic
- `expressions.bas` - Multiple operators
- `precedence.bas` - Operator precedence test

Run tests:

```bash
./basset tests/hello.bas
./basset tests/arithmetic.bas
```

## Design Decisions

### Why Table-Driven?

The implementation uses a table-driven architecture inspired by classic BASIC interpreters. This approach offers:

1. **Separation of concerns**: Grammar is data, parsing is logic
2. **Maintainability**: Adding statements requires table updates, not parser rewrites
3. **Historical accuracy**: Matches the original implementation's design
4. **Compactness**: The original fit in 8KB ROM partially due to this design

### C89/C90 Standard

The code follows ANSI C89/C90 (ISO C90) conventions:

- Variables declared at block start (no mixed declarations)
- ANSI function prototypes with parameter types
- Standard library headers (`<stdlib.h>`, `<string.h>`, etc.)
- No C99/C11 features (no `//` comments, no designated initializers, etc.)
- Compiled with `-ansi -pedantic` for strict ISO C90 compliance

### Memory Management

- Dynamic allocation used throughout (no pre-allocated program buffer)
- Parse trees built as programs are parsed
- Variables stored in expandable tables

## Future Work

Potential enhancements:

1. **DATA/READ/RESTORE**: Implement data storage and retrieval
2. **Graphics execution**: Actual screen drawing (GRAPHICS, PLOT, DRAWTO, etc.)
3. **Sound execution**: Audio output implementation
4. **Additional functions**: More string manipulation, additional math functions
5. **Optimization**: Bytecode optimization passes, faster execution
6. **Debugging**: Step-through debugger, breakpoints, variable inspection
7. **Extended I/O**: Additional file operations, device support

## Documentation

Comprehensive documentation is available in the `docs/` directory:

### Architecture & Design
- **[docs/Architecture.md](docs/Architecture.md)** - System architecture, components, and compilation pipeline
- **[docs/Parser.md](docs/Parser.md)** - Enum-based expression parser design and implementation
- **[docs/Grammar_Reference.md](docs/Grammar_Reference.md)** - Grammar encoding, BML meta-language, and extensibility

### Reference Documentation  
- **[docs/Token_Reference.md](docs/Token_Reference.md)** - Complete catalog of 100+ tokens with hex values and descriptions
- **[docs/Bytecode_Reference.md](docs/Bytecode_Reference.md)** - All ~140 VM opcodes with stack effects and examples
- **[docs/Virtual_Machine.md](docs/Virtual_Machine.md)** - Virtual machine internals, memory model, and execution

### Developer Guides
- **[docs/Extending_Guide.md](docs/Extending_Guide.md)** - Step-by-step guide for extending the language
- **[tests/README.md](tests/README.md)** - Test suite organization and running tests

See **[docs/README.md](docs/README.md)** for the complete documentation index.
- Syntax tables reference (BML grammar encoding)
- VM architecture details
- Implementation overview

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
