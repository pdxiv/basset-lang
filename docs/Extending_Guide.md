# Adding New Language Features

This guide explains how to extend Basset BASIC with new functions, operators, and statements.

## Adding a New Built-in Function

### 1. Define the Token (tokens.h)
```c
#define TOK_CTAN    0xNN  /* TAN function */
```

### 2. Add to Keyword Table (syntax_tables.c)
```c
const KeywordEntry keyword_table[] = {
    // ... existing entries ...
    {"TAN",    TOK_CTAN,    0},  /* Numeric function */
    // ...
};
```

### 3. Add to Function Metadata Table (syntax_tables.c)
```c
const FunctionEntry function_table[] = {
    // ... existing entries ...
    {TOK_CTAN,    "TAN",    1, 1},  /* TAN(x) - one argument */
    // ...
};
```

### 4. Implement Compiler Handler (compiler.c)
```c
static void compile_tan(CompilerState *cs, ParseNode *node) {
    /* Compile argument */
    compile_expression(cs, node->children[0]);
    
    /* Emit TAN opcode */
    compiler_emit_no_operand(cs, OP_FUNC_TAN);
}
```

### 5. Add to Compilation Table (compiler.c)
```c
const CompilationEntry compilation_table[] = {
    // ... existing entries ...
    {TOK_CTAN, compile_tan},
    // ...
};
```

### 6. Implement VM Opcode (vm.c)
```c
case OP_FUNC_TAN: {
    double x = vm_pop(vm);
    vm_push(vm, tan(x));  /* Standard library */
    break;
}
```

### 7. Add Test Cases
Create `tests/standard/functions/test_tan.bas`:
```basic
10 REM Test TAN function
20 X = TAN(0)
30 PRINT X
40 Y = TAN(0.7854)  ' π/4
50 PRINT Y
```

The parser will automatically:
- Recognize TAN as a function token (keyword table lookup)
- Parse the function call with argument validation (function metadata table)
- Ensure exactly 1 argument (arity check from metadata table)

## Adding a New Operator

Operators are already defined in the operator precedence table. To add a new operator:

### 1. Define the Token (tokens.h)
```c
#define TOK_CMOD    0xNN  /* MOD modulo operator */
```

### 2. Add to Operator Table (syntax_tables.c)
```c
const OperatorEntry operator_table[] = {
    // ... existing entries ...
    {TOK_CMOD,    5, 5, NULL},   /* MOD - same precedence as * / */
    // ...
};
```

### 3. Add to Compilation Table (compiler.c)
```c
static void compile_mod(CompilerState *cs, ParseNode *node) {
    compile_expression(cs, node->children[0]);  /* Left operand */
    compile_expression(cs, node->children[1]);  /* Right operand */
    compiler_emit_no_operand(cs, OP_MOD);
}
```

### 4. Implement VM Opcode (vm.c)
```c
case OP_MOD: {
    double b = vm_pop(vm);
    double a = vm_pop(vm);
    vm_push(vm, fmod(a, b));
    break;
}
```

The parser will automatically handle the operator with correct precedence.

## Adding a New Statement

Statements are defined in the statement dispatch table and have corresponding grammar rules.

### Example: Adding SWAP Statement

#### 1. Define Token and Grammar (syntax_tables.c)
```c
/* Grammar: SWAP <VAR>, <VAR> */
static const SyntaxEntry syn_swap[] = {
    SYN_NT(NT_NVAR),     /* First variable */
    SYN_TOK(TOK_CCOM),   /* Comma */
    SYN_NT(NT_NVAR),     /* Second variable */
    SYN_NT(NT_EOS),      /* End of statement */
    SYN_RTN
};
```

#### 2. Add to Statement Table (syntax_tables.c)
```c
const StatementEntry statement_table[] = {
    // ... existing entries ...
    {TOK_SWAP, NT_SWAP},
    // ...
};
```

#### 3. Implement Compiler (compiler.c)
```c
static void compile_swap(CompilerState *cs, ParseNode *node) {
    /* Get variables */
    ParseNode *var1 = node->children[0];
    ParseNode *var2 = node->children[1];
    
    /* Load var1 */
    compile_variable_load(cs, var1);
    /* Load var2 */
    compile_variable_load(cs, var2);
    /* Store to var1 */
    compile_variable_store(cs, var1);
    /* Store to var2 */
    compile_variable_store(cs, var2);
}
```

The parser will automatically route SWAP through the grammar table.

## Key Advantages of Table-Driven Design

1. **No Parser Changes**: Adding functions requires ZERO changes to parser.c
2. **Automatic Validation**: Arity checking happens automatically
3. **Better Error Messages**: "SIN expects 1 argument, got 2" without custom code
4. **Easier Testing**: Can validate table completeness with `make test-validation`
5. **Lower Maintenance**: New features isolated to table entries + compiler/VM

## What's Still Hard-Coded?

Some aspects remain hard-coded by design:

- **Expression Atoms**: Numbers, strings, variables (fundamental primitives)
- **Array Subscripts**: Tightly coupled with variable parsing
- **Unary Operators**: Require context-dependent disambiguation
- **PRINT Statement**: Complex separator semantics (justified special case)
- **IF/ELSE**: Control flow terminator detection (complex grammar)

These are pragmatic engineering decisions where table-driven design would add complexity without benefit.

## Validation

After adding new features, validate the implementation:

```bash
make test                     # Run all tests
make test-validation          # Validate table coverage
```

## Related Documentation

- **[Grammar_Reference.md](Grammar_Reference.md)** - Table-driven syntax system design
- **[Architecture.md](Architecture.md)** - Overall system architecture
- **[Parser.md](Parser.md)** - Expression parsing implementation
