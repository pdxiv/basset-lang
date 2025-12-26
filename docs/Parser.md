# Expression Parser Architecture

## Overview

Basset BASIC uses a **data-driven Pratt parser** with **enum-based dispatch** for expression parsing. This design separates parsing logic (the algorithm) from parsing behavior (the data), making the system declarative and maintainable.

## Design Principles

### Data-Driven Architecture

The operator table contains pure declarative data (enum values) rather than executable code (function pointers). The parser algorithm interprets these values through switch-based dispatch, achieving true separation of concerns:

- **Data Layer**: Operator table with enum values describing parse actions
- **Logic Layer**: Switch-based dispatcher executing appropriate parse functions
- **Action Layer**: Static parse functions implementing specific behaviors

### Pratt Parsing

The parser implements Vaughan Pratt's algorithm using the **nud/led pattern**:

- **nud** (Null Denotation): How to parse a token when it appears in prefix/atom position
- **led** (Left Denotation): How to parse a token when it appears in infix position
- **Precedence**: Each operator has `go_on_stack` and `come_off_stack` values for precedence climbing

This enables elegant handling of:
- Prefix operators (`-x`, `NOT x`)
- Infix operators (`x + y`, `x AND y`)
- Postfix operators (none in BASIC, but supported by architecture)
- Atoms (numbers, strings, variables, function calls)

## Architecture

### Parse Action Type Enum
```c
typedef enum {
    PA_NONE = 0,                /* No action */
    PA_NUMBER_LITERAL,          /* Parse number literal */
    PA_STRING_LITERAL,          /* Parse string literal */
    PA_VARIABLE,                /* Parse variable (with optional array subscripts) */
    PA_PARENTHESIZED,           /* Parse parenthesized expression */
    PA_UNARY_PLUS,              /* Parse unary + */
    PA_UNARY_MINUS,             /* Parse unary - */
    PA_UNARY_NOT,               /* Parse unary NOT */
    PA_FUNCTION_CALL,           /* Parse function call */
    PA_BINARY_OP                /* Parse binary operator */
} ParseActionType;
```

### OperatorEntry Structure
```c
typedef struct {
    unsigned char token;          /* Token type (from tokens.h) */
    unsigned char go_on_stack;    /* Precedence for going on stack */
    unsigned char come_off_stack; /* Precedence for coming off stack */
    void (*executor)(void);       /* Execution function pointer (for VM) */
    ParseActionType nud;          /* Null denotation (prefix/atom) */
    ParseActionType led;          /* Left denotation (infix) */
} OperatorEntry;
```

**Fields**:
- `token`: The token type this entry handles (e.g., TOK_CPLUS, TOK_NUMBER)
- `go_on_stack`: Precedence when operator goes on the stack (higher = tighter binding)
- `come_off_stack`: Precedence when operator comes off the stack (enables right-associativity)
- `executor`: VM execution function (used during runtime, not parsing)
- `nud`: How to parse this token in prefix/atom position (PA_* enum value)
- `led`: How to parse this token in infix position (PA_* enum value)

### Parse Actions

### Parse Actions

Nine parse action functions implement all expression parsing behavior:

| Action | Purpose | Complexity |
|--------|---------|------------|
| `parse_number_literal` | Parse numeric literals | 8 lines |
| `parse_string_literal` | Parse string literals | 8 lines |
| `parse_variable` | Parse variables with array subscripts | 28 lines |
| `parse_parenthesized` | Parse parenthesized expressions | 9 lines |
| `parse_unary_plus` | Parse unary + operator | 7 lines |
| `parse_unary_minus` | Parse unary - operator | 10 lines |
| `parse_unary_not` | Parse NOT operator | 10 lines |
| `parse_function_call` | Parse function calls with arity validation | 55 lines |
| `parse_binary_op` | Parse all binary operators | 20 lines |

All functions are `static` to [parser.c](../src/parser.c) - they're called through dispatch, never directly.

### Operator Table

The operator table contains **47 entries** defining parsing behavior for all expression elements:

**Atoms** (nud only):
```c
{TOK_NUMBER,  0,  0, NULL, PA_NUMBER_LITERAL,  PA_NONE},
{TOK_STRING,  0,  0, NULL, PA_STRING_LITERAL,  PA_NONE},
{TOK_IDENT,   0,  0, NULL, PA_VARIABLE,        PA_NONE},
{TOK_CLPRN,   0,  0, NULL, PA_PARENTHESIZED,   PA_NONE},
```

**Binary Operators** (led only):
```c
{TOK_CPLUS,  12, 12, vm_add,    PA_NONE, PA_BINARY_OP},  // Addition
{TOK_CMINUS, 12, 12, vm_sub,    PA_NONE, PA_BINARY_OP},  // Subtraction
{TOK_CMULT,  13, 13, vm_mult,   PA_NONE, PA_BINARY_OP},  // Multiplication
{TOK_CDIV,   13, 13, vm_div,    PA_NONE, PA_BINARY_OP},  // Division
{TOK_CEXP,   15, 14, vm_exp,    PA_NONE, PA_BINARY_OP},  // Exponentiation (right-assoc)
{TOK_CEQ,    10, 10, vm_eq,     PA_NONE, PA_BINARY_OP},  // Equal
{TOK_CLESS,  10, 10, vm_less,   PA_NONE, PA_BINARY_OP},  // Less than
// ... (11 total binary operators)
```

**Dual-Role Operators** (both nud and led):
```c
{TOK_CPLUS,  12, 12, vm_add,    PA_UNARY_PLUS,  PA_BINARY_OP},  // +x or x+y
{TOK_CMINUS, 12, 12, vm_sub,    PA_UNARY_MINUS, PA_BINARY_OP},  // -x or x-y
```

**Unary Operators** (nud only):
```c
{TOK_CNOT,   16, 16, vm_not,    PA_UNARY_NOT,   PA_NONE},       // NOT x
```

**Functions** (nud only, all use PA_FUNCTION_CALL):
```c
{TOK_SIN,    0,  0, vm_sin,     PA_FUNCTION_CALL, PA_NONE},
{TOK_COS,    0,  0, vm_cos,     PA_FUNCTION_CALL, PA_NONE},
{TOK_ATN,    0,  0, vm_atn,     PA_FUNCTION_CALL, PA_NONE},
// ... (27 total functions)
```

See [syntax_tables.c](../src/syntax_tables.c) for the complete operator table.

## Parser Algorithm

## Parser Algorithm

The core parsing function `parse_expression_pratt_prec()` implements precedence-climbing Pratt parsing in 75 lines:

```c
static ParseNode* parse_expression_pratt_prec(Parser *p, int min_prec) {
    Token *tok;
    ParseNode *left;
    const OperatorEntry *op_entry;
    int prec;
    
    tok = tokenizer_peek(p->tokenizer);
    
    /* === PHASE 1: NULL DENOTATION (nud) - Parse prefix/atom === */
    op_entry = get_operator_entry(tok->type);
    
    if (!op_entry || op_entry->nud == PA_NONE) {
        set_error(p, "Expected expression");
        return NULL;
    }
    
    /* Dispatch to appropriate parse action based on nud enum */
    switch (op_entry->nud) {
        case PA_NUMBER_LITERAL:  left = parse_number_literal(p, NULL); break;
        case PA_STRING_LITERAL:  left = parse_string_literal(p, NULL); break;
        case PA_VARIABLE:        left = parse_variable(p, NULL); break;
        case PA_PARENTHESIZED:   left = parse_parenthesized(p, NULL); break;
        case PA_UNARY_PLUS:      left = parse_unary_plus(p, NULL); break;
        case PA_UNARY_MINUS:     left = parse_unary_minus(p, NULL); break;
        case PA_UNARY_NOT:       left = parse_unary_not(p, NULL); break;
        case PA_FUNCTION_CALL:   left = parse_function_call(p, NULL); break;
        default:
            set_error(p, "Invalid nud action");
            return NULL;
    }
    
    if (!left) return NULL;
    
    /* === PHASE 2: LEFT DENOTATION (led) - Handle infix operators === */
    while (1) {
        tok = tokenizer_peek(p->tokenizer);
        op_entry = get_operator_entry(tok->type);
        
        if (!op_entry || op_entry->led == PA_NONE) {
            break;  /* Not an infix operator */
        }
        
        prec = op_entry->go_on_stack;
        if (prec < min_prec) {
            break;  /* Precedence too low */
        }
        
        /* Dispatch to appropriate parse action based on led enum */
        switch (op_entry->led) {
            case PA_BINARY_OP:
                left = parse_binary_op(p, left);
                break;
            default:
                set_error(p, "Invalid led action");
                return NULL;
        }
        
        if (!left) break;
    }
    
    return left;
}
```

### Algorithm Flow

1. **Lookup operator entry** for current token
2. **Phase 1 (nud)**: Dispatch based on `nud` enum value to parse prefix/atom
3. **Phase 2 (led)**: Loop while infix operators with sufficient precedence:
   - Check if token has `led` action
   - Check if precedence >= minimum required
   - Dispatch based on `led` enum value
4. **Return** the complete expression tree

### Precedence Handling

Precedence values control operator binding strength:

| Precedence | Operators | Associativity |
|------------|-----------|---------------|
| 16 | `NOT` | Left |
| 15/14 | `^` | Right (15=go, 14=come) |
| 13 | `*`, `/` | Left |
| 12 | `+`, `-` | Left |
| 10 | `=`, `<`, `>`, `<=`, `>=`, `<>` | Left |
| 9 | `AND` | Left |
| 8 | `OR` | Left |

**Right-associativity** is achieved by having different `go_on_stack` (15) and `come_off_stack` (14) values for `^`, making it pop before recursing.

## Design Benefits

### Type Safety

- **Zero compiler warnings**: Pure data (enums) vs function pointers that caused 49 warnings
- **Compile-time validation**: Invalid enum values caught immediately
- **No casting required**: All types match exactly

### Performance

- **Switch optimization**: Compilers generate jump tables or binary searches
- **No indirect calls**: Switch dispatch faster than function pointers
- **Better branch prediction**: Modern CPUs predict switch cases well
- **Reduced overhead**: No function pointer dereference cost

### Maintainability

- **Centralized dispatch**: All parsing logic in one 75-line function
- **Clear control flow**: Explicit switch cases, no indirection
- **Simplified debugging**: Breakpoints in switch cases show exact execution path
- **Isolated concerns**: Parse actions are independent, testable units

### Extensibility

Adding a new operator requires:
1. Define enum value in `ParseActionType` (if new action type needed)
2. Implement parse action function in `parser.c`
3. Add entry to operator table in `syntax_tables.c`
4. Add switch case to dispatcher (if new action type)

No header changes needed for existing action types (e.g., new binary operators just reuse `PA_BINARY_OP`).

## Implementation Details

## Implementation Details

### Source Files

- **[syntax_tables.h](../src/syntax_tables.h)**: Defines `ParseActionType` enum and `OperatorEntry` structure
- **[syntax_tables.c](../src/syntax_tables.c)**: Contains 47-entry operator table with enum values
- **[parser.c](../src/parser.c)**: Implements switch-based dispatcher (75 lines) and 9 parse action functions (155 lines)

### Enum vs Function Pointer Approach

This implementation uses **enum-based dispatch** instead of function pointers. Comparison:

| Aspect | Enum-Based (Current) | Function Pointers |
|--------|---------------------|-------------------|
| **Data-driven** | ✅ Pure data (enums) | ⚠️ Contains code pointers |
| **Type safety** | ✅ Zero warnings | ❌ 49 type warnings (circular deps) |
| **Performance** | ✅ Fast switch dispatch | ⚠️ Indirect function calls |
| **Debugging** | ✅ Clear control flow | ⚠️ More difficult to trace indirection |
| **Extensibility** | ⚠️ Add switch case | ✅ Add table entry only |
| **Encapsulation** | ✅ Parse actions are static | ⚠️ Must expose in headers |

The enum approach was chosen for BASIC because:
- The grammar is stable (compile-time extension sufficient)
- Type safety eliminates 49 circular dependency warnings
- Better performance and debuggability
- True separation of data (table) and logic (dispatcher)

### Parse Node Structure

Parse actions return `ParseNode*` structures representing the abstract syntax tree:

```c
typedef struct ParseNode {
    NodeType type;           /* NODE_LITERAL, NODE_VARIABLE, NODE_BINARY_OP, etc. */
    union {
        double number;       /* For number literals */
        char *string;        /* For string literals */
        struct {             /* For binary operations */
            struct ParseNode *left;
            struct ParseNode *right;
            unsigned char op;
        } binary_op;
        /* ... other node types ... */
    } data;
} ParseNode;
```

The parse tree is consumed by the compiler to generate bytecode.

## Examples

### Simple Expression: `3 + 4`

1. **Token stream**: `TOK_NUMBER(3)`, `TOK_CPLUS`, `TOK_NUMBER(4)`
2. **Phase 1 (nud)**: 
   - Lookup `TOK_NUMBER` → `nud = PA_NUMBER_LITERAL`
   - Dispatch to `parse_number_literal()` → creates node for `3`
3. **Phase 2 (led)**:
   - Peek `TOK_CPLUS` → `led = PA_BINARY_OP`, precedence = 12
   - Dispatch to `parse_binary_op()`:
     - Consume `TOK_CPLUS`
     - Recursively parse right side: `4` (precedence climb)
     - Create binary op node: `3 + 4`
4. **Result**: Binary op node with left=3, op=PLUS, right=4

### Complex Expression: `-5 * (X + 2)`

1. **Token stream**: `TOK_CMINUS`, `TOK_NUMBER(5)`, `TOK_CMULT`, `TOK_CLPRN`, `TOK_IDENT(X)`, ...
2. **Phase 1 (nud)**:
   - Lookup `TOK_CMINUS` → `nud = PA_UNARY_MINUS`
   - Dispatch to `parse_unary_minus()` → recursively parse `5`, create unary minus node
3. **Phase 2 (led)**:
   - Peek `TOK_CMULT` → precedence = 13
   - Dispatch to `parse_binary_op()`:
     - Consume `TOK_CMULT`
     - Recursively parse `(X + 2)`:
       - `TOK_CLPRN` → `nud = PA_PARENTHESIZED` → parse inner expression
       - Inner: `X + 2` parsed as binary op
     - Create binary op node: `(-5) * (X + 2)`
4. **Result**: Binary multiply with left=unary_minus(5), right=binary_plus(X, 2)

### Function Call: `SIN(X * 3.14)`

1. **Token stream**: `TOK_SIN`, `TOK_CLPRN`, `TOK_IDENT(X)`, `TOK_CMULT`, ...
2. **Phase 1 (nud)**:
   - Lookup `TOK_SIN` → `nud = PA_FUNCTION_CALL`
   - Dispatch to `parse_function_call()`:
     - Expect `(`
     - Parse argument expression: `X * 3.14`
     - Validate arity (SIN requires 1 argument)
     - Expect `)`
     - Create function call node
3. **Result**: Function call node with func=SIN, arg=binary_mult(X, 3.14)

## Related Documentation

- **[Grammar_Reference.md](Grammar_Reference.md)**: Complete grammar encoding and BML syntax
- **[Token_Reference.md](Token_Reference.md)**: All token types and their values
- **[Architecture.md](Architecture.md)**: Overall system design and compilation pipeline
- **[Bytecode_Reference.md](Bytecode_Reference.md)**: Bytecode generation from parse trees

The enum approach was chosen for BASIC because:
- The grammar is stable (compile-time extension sufficient)
- Type safety eliminates 49 circular dependency warnings
- Better performance and debuggability
- True separation of data (table) and logic (dispatcher)

### Parse Node Structure

Parse actions return `ParseNode*` structures representing the abstract syntax tree:

```c
typedef struct ParseNode {
    NodeType type;           /* NODE_LITERAL, NODE_VARIABLE, NODE_BINARY_OP, etc. */
    union {
        double number;       /* For number literals */
        char *string;        /* For string literals */
        struct {             /* For binary operations */
            struct ParseNode *left;
            struct ParseNode *right;
            unsigned char op;
        } binary_op;
        /* ... other node types ... */
    } data;
} ParseNode;
```

The parse tree is consumed by the compiler to generate bytecode.

## Examples
