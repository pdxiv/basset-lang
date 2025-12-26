# Quick Start Guide

## Building

```bash
make all
```

This creates:
- `basset_compile` - Compiler (BASIC → bytecode)
- `basset_vm` - Virtual machine (runs bytecode)
- `basset_disasm` - Disassembler
- `basset_asm` - Assembler
- `basset_tokenize` - Tokenizer debugger

## Running Programs

Compile a BASIC program:
```bash
./basset_compile program.bas
```

Run the compiled bytecode:
```bash
./basset_vm program.abc
```

Example:
```bash
./basset_compile tests/hello.bas
./basset_vm tests/hello.abc
```

## Debugging Tools

### Tokenizer Debugger

View how source code is tokenized:
```bash
./basset_tokenize program.bas
```

Example output:
```
Tokenizing: tests/hello.bas
════════════════════════════════════════════════════════════
Line  Col    Type          Name                  Value/Text
════════════════════════════════════════════════════════════
1     0      0x6B/107     NUMBER                10
1     3      0x20/32      PRINT                 
1     9      0x6C/108     STRING                "Hello, World!"
...
```

This shows:
- Token positions (line/column)
- Token types (hex and decimal)
- Token names (PRINT, NUMBER, STRING, etc.)
- Values for numbers and text content

Useful for:
- Verifying keyword recognition
- Debugging lexer issues
- Understanding token boundaries

### Disassembler

View compiled bytecode:
```bash
./basset_disasm program.abc
```

## Testing

```bash
# Run all 130 tests
make test

# Or run individual test suites
./tests/standard/run.sh     # 110 functional tests
./tests/errors/run.sh       # 14 error tests
./tests/tokenizer/run.sh    # 6 tokenizer tests
```

## Example Programs

### Hello World
```basic
10 PRINT "Hello, BASIC!"
20 END
```

### Variables and Arithmetic
```basic
10 X = 10
20 Y = 20
30 Z = X + Y
40 PRINT Z
50 END
```

### Expressions
```basic
10 A = 5
20 B = 10
30 PRINT A + B
40 PRINT A * B
50 PRINT B / A
60 PRINT B ^ 2
70 END
```

## Supported Features

- Arithmetic: `+`, `-`, `*`, `/`, `^`
- Comparisons: `=`, `<`, `>`, `<=`, `>=`, `<>`
- Logical: `AND`, `OR`, `NOT`
- Parentheses for grouping
- Variables (numeric)
- PRINT statement
- REM comments
- END statement

## Cleaning

```bash
make clean
```

## Running Tests

Run the full test suite:
```bash
make test
```

Current status: **130/130 tests passing (100%)**
