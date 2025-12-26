# Contributing to Basset BASIC

Thank you for your interest in contributing to the Basset BASIC compiler! This guide will help you understand the codebase architecture and how to add new features.

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Adding a New Statement](#adding-a-new-statement)
- [Adding a New Function](#adding-a-new-function)
- [Code Style](#code-style)
- [Testing](#testing)

## Architecture Overview

Basset BASIC uses a **table-driven architecture** where behavior is defined by data tables rather than hard-coded logic:

- **Parser**: ~90% table-driven via syntax tables
- **Compiler**: 100% table-driven via dispatch tables
- **VM**: Stack-based bytecode interpreter

## Adding a New Statement

Adding a new BASIC statement requires updates to 5 components. Here's a complete walkthrough using a hypothetical `BEEP` statement as an example.

### Step 1: Add the Token (src/tokens.h)

Add the token definition:

```c
#define TOK_BEEP    0x7A  /* BEEP */
```

### Step 2: Add the Keyword (src/tokenizer.c)

Add to the keyword table (alphabetically sorted):

```c
static const KeywordEntry keywords[] = {
    {"AND", TOK_CAND},
    // ... other keywords ...
    {"BEEP", TOK_BEEP},      /* <-- Add here */
    // ... more keywords ...
};
```

### Step 3: Add Parser Support

#### 3a. Add Non-Terminal (src/syntax_tables.h)

```c
typedef enum {
    // ... existing non-terminals ...
    NT_BEEP_STMT,    /* <BEEP> statement */
    NT_MAX_NONTERMINALS
} NonTerminal;
```

#### 3b. Define Grammar Rule (src/syntax_tables.c)

```c
/* <BEEP> = <EXP> , <EXP> <EOS> # */
static const SyntaxEntry syn_beep[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},      /* Frequency */
    SYN_TOK(TOK_CCOM),          /* Comma */
    {SYN_VEXP, {0, 0, 0}},      /* Duration */
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),             /* No arguments */
    SYN_END
};
```

#### 3c. Register Rule (src/syntax_tables.c in init_syntax_tables())

```c
void init_syntax_tables(void) {
    // ... existing registrations ...
    syntax_rule_table[NT_BEEP_STMT] = syn_beep;
}
```

#### 3d. Map Token to Non-Terminal (src/syntax_tables.c)

Add to statement_table:

```c
const StatementEntry statement_table[] = {
    // ... existing entries ...
    {TOK_BEEP, NT_BEEP_STMT},
    // ... more entries ...
};
```

### Step 4: Add Bytecode Opcode (src/bytecode.h)

```c
#define OP_BEEP         0x70    /* BEEP: Play sound */
```

### Step 5: Add Compiler Function (src/compiler.c)

#### 5a. Write Compiler Function

```c
static void compile_beep_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 2) {
        /* BEEP frequency, duration */
        compile_expression(cs, stmt->children[0]); /* frequency */
        compile_expression(cs, stmt->children[1]); /* duration */
        compiler_emit_no_operand(cs, OP_BEEP);
    } else {
        /* BEEP with no arguments - use defaults */
        compiler_emit(cs, OP_PUSH_CONST, 0);  /* 440 Hz */
        compiler_emit(cs, OP_PUSH_CONST, 1);  /* 100 ms */
        compiler_emit_no_operand(cs, OP_BEEP);
    }
}
```

#### 5b. Add to Dispatch Table

Add entry to compilation_table (alphabetically sorted by token):

```c
static const CompilationEntry compilation_table[] = {
    // ... existing entries ...
    {TOK_BEEP, compile_beep_stmt},
    // ... more entries ...
    {0, NULL}  /* Sentinel */
};
```

### Step 6: Add VM Implementation (src/vm.c)

Add case to vm_execute():

```c
case OP_BEEP: {
    double duration = vm_pop(vm);
    double frequency = vm_pop(vm);
    
    /* Play sound at frequency for duration milliseconds */
    /* Implementation depends on platform */
    printf("BEEP %gHz for %gms\n", frequency, duration);
    break;
}
```

### Step 7: Test

Create test file tests/standard/test_beep.bas:

```basic
10 REM Test BEEP statement
20 BEEP 440, 100
30 BEEP
40 PRINT "BEEP test complete"
```

Create expected output tests/standard/test_beep.expected:

```
BEEP 440Hz for 100ms
BEEP 0Hz for 0ms
BEEP test complete
```

Run tests:

```bash
make test
```

## Adding a New Function

Adding a built-in function like `SQRT()` is similar but requires:

1. **Token**: Add `TOK_SQRT` to tokens.h
2. **Keyword**: Add "SQRT" to tokenizer keyword table
3. **Parser**: Usually functions are already handled generically via NT_NFUN
4. **Compiler**: Add case to compile_expression() for function calls
5. **VM**: Add `OP_SQRT` opcode and implementation in VM
6. **Test**: Create test file with function usage

See existing functions like `SIN`, `COS`, `ABS` for examples.

## Code Style

- **Language**: ANSI C89/C90 (ISO C90) for maximum portability
- **Indentation**: 4 spaces (no tabs)
- **Naming**: snake_case for functions, UPPER_CASE for macros/constants
- **Comments**: Clear, concise explanations of non-obvious logic
- **Line length**: Aim for <100 characters

### Example Function:

```c
/* Compile BEEP statement with frequency and duration */
static void compile_beep_stmt(CompilerState *cs, ParseNode *stmt) {
    if (stmt->child_count >= 2) {
        compile_expression(cs, stmt->children[0]);
        compile_expression(cs, stmt->children[1]);
        compiler_emit_no_operand(cs, OP_BEEP);
    } else {
        compiler_emit(cs, OP_PUSH_CONST, 440);
        compiler_emit(cs, OP_PUSH_CONST, 100);
        compiler_emit_no_operand(cs, OP_BEEP);
    }
}
```

## Testing

All new features must include tests:

1. **Standard Test**: Verify correct behavior
   - File: `tests/standard/test_<feature>.bas`
   - Expected output: `tests/standard/test_<feature>.expected`

2. **Error Test** (if applicable): Verify error handling
   - File: `tests/errors/err_<error>.bas`
   - Expected error: `tests/errors/err_<error>.expected`

Run the test suite:

```bash
make test               # All tests
make test-standard      # Just standard tests
make test-errors        # Just error tests
```

### Writing Good Tests

- Test the happy path (correct usage)
- Test edge cases (empty input, boundary values)
- Test error cases (syntax errors, type mismatches)
- Keep tests focused and simple
- Include comments explaining what's being tested

Example:

```basic
10 REM Test BEEP with different values
20 BEEP 220, 50
30 BEEP 440, 100
40 BEEP 880, 200
50 REM Test BEEP with no arguments
60 BEEP
70 PRINT "All tests passed"
```

## Development Workflow

1. **Branch**: Create a feature branch from main
2. **Implement**: Follow the steps above for your feature
3. **Test**: Ensure all existing tests pass + add new tests
4. **Document**: Update relevant documentation (README.md, etc.)
5. **Commit**: Make clear, atomic commits
6. **Pull Request**: Submit PR with description of changes

## Questions?

- Check the [README.md](README.md) for architecture overview
- Browse existing code for examples
- Look at [docs/](docs/) for detailed references
- Review [REFACTORING_PROGRESS.md](REFACTORING_PROGRESS.md) for recent improvements

## Summary: The Table-Driven Approach

The key insight of Basset BASIC's architecture is that **adding features is declarative, not imperative**:

- **Before** (hard-coded): Modify parser logic + compiler switch statement = fragile, error-prone
- **After** (table-driven): Add table entries + focused functions = clean, maintainable

This makes the codebase:
- ✅ Easier to extend
- ✅ Less prone to bugs
- ✅ More consistent
- ✅ Self-documenting (grammar rules are visible)

Welcome to the project!
