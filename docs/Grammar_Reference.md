# Syntax Tables Reference - Grammar Encoding

## Overview

This document describes the table-driven syntax system that defines the grammar of BASIC. The implementation follows the BML (BASIC Meta-Language) design from classic table-driven BASIC interpreters, where grammar rules are encoded as data tables rather than hard-coded parsing functions.

## Design Philosophy

**Key Principle**: The parser is generic and consults tables to determine syntax. Adding new statements requires only table modifications, not new parsing code.

This design makes the language:
- **Extensible**: New statements via table additions
- **Maintainable**: Grammar centralized in data
- **Compact**: One parser handles all statements
- **Verifiable**: Grammar visible and testable

## Table Components

The syntax system consists of four interconnected tables:

1. **Keyword Table**: Maps text strings to token IDs
2. **Statement Table**: Maps statement tokens to syntax rules
3. **Syntax Rule Table**: Encodes grammar productions
4. **Operator Table**: Defines operator precedence

---

## 1. Keyword Table

Maps BASIC keywords to their token IDs.

### Structure
```c
typedef struct {
    const char *keyword;     /* Keyword text (uppercase) */
    unsigned char token;     /* Token ID (from tokens.h) */
    unsigned char type;      /* 0=statement, 1=keyword, 2=operator, 3=function */
} KeywordEntry;
```

### Example Entries
```c
{"PRINT", TOK_PRINT, 0},   /* Statement */
{"FOR",   TOK_FOR,   0},   /* Statement */
{"TO",    TOK_CTO,   1},   /* Keyword */
{"AND",   TOK_CAND,  2},   /* Operator */
{"SIN",   TOK_CSIN,  3},   /* Function */
```

### Size
- **100+ entries** covering all BASIC keywords
- **Null-terminated** with sentinel `{NULL, 0, 0}`

### Lookup Algorithm
- **Case-insensitive** comparison
- **Longest-match** wins (e.g., "CINT" matches before "INT")
- **Linear search** during tokenization

---

## 2. Statement Table

Maps statement token IDs to their syntax rules (non-terminals).

### Structure
```c
typedef struct {
    unsigned char token;      /* Statement token (e.g., TOK_PRINT) */
    NonTerminal syntax_rule;  /* Corresponding non-terminal (e.g., NT_PRINT) */
} StatementEntry;
```

### Example Entries
```c
{TOK_PRINT,  NT_PRINT},
{TOK_FOR,    NT_FOR},
{TOK_IF,     NT_IF},
{TOK_LET,    NT_LET},
{TOK_GOTO,   NT_GOTO_STMT},
{TOK_END,    NT_END_STMT},
```

### Usage
When the parser encounters a statement token, it looks up the corresponding non-terminal and processes that syntax rule.

---

## 3. Syntax Rule Table

Encodes grammar productions using BML opcodes. This is the heart of the table-driven parser.

### Syntax Opcodes

| Opcode | Hex | Symbol | Meaning |
|--------|-----|--------|---------|
| SYN_ANTV | 0x00 | - | Absolute Non-Terminal Vector (reference to another rule) |
| SYN_ESRT | 0x01 | - | External Subroutine Call (reserved) |
| SYN_OR | 0x02 | \| | Alternative (BNF disjunction) |
| SYN_RTN | 0x03 | # | Return (end of rule) |
| SYN_NULL | 0x04 | & | Null/Accept (optional element) |
| SYN_VEXP | 0x0E | - | Special vector for expression parsing |
| SYN_CHNG | 0x0F | - | Change last output token |

### Terminal vs. Non-Terminal

- **Terminal**: Token from input (has `TC_TERMINAL` flag = 0x80)
- **Non-Terminal**: Reference to another syntax rule

### Syntax Entry Structure
```c
typedef struct {
    unsigned char opcode;      /* Syntax opcode or token (with TC_TERMINAL flag) */
    unsigned char data[3];     /* Additional data (non-terminal ID, etc.) */
} SyntaxEntry;
```

### Encoding Macros
```c
#define SYN_TOK(t)  {(t) | TC_TERMINAL, {0, 0, 0}}  /* Terminal token */
#define SYN_NT(nt)  {SYN_ANTV, {(nt) & 0xFF, 0, 0}} /* Non-terminal reference */
#define SYN_OP(op)  {(op), {0, 0, 0}}               /* Syntax opcode */
#define SYN_END     {SYN_RTN, {0, 0, 0}}            /* End of rule */
#define SYN_ALT     {SYN_OR, {0, 0, 0}}             /* Alternative */
#define SYN_EPS     {SYN_NULL, {0, 0, 0}}           /* Epsilon (optional) */
```

---

## Grammar Examples

### Simple Statement: RANDOMIZE

**BNF**: `<RANDOMIZE_STMT> ::= RANDOMIZE <EOS>`

**Encoded**:
```c
static const SyntaxEntry syn_randomize[] = {
    SYN_TOK(TOK_RANDOMIZE),  /* Terminal: RANDOMIZE keyword */
    SYN_NT(NT_EOS),          /* Non-terminal: end of statement */
    SYN_END                  /* Return */
};
```

### Assignment: LET

**BNF**: `<LET> ::= LET? <NVAR> = <EXP> <EOS> | <SVAR> = <STR> <EOS>`

**Encoded**:
```c
static const SyntaxEntry syn_let[] = {
    /* Optional LET keyword */
    SYN_ALT,                  /* Alternative 1: numeric assignment */
    SYN_NT(NT_NVAR),          /* Numeric variable */
    SYN_TOK(TOK_CEQ),         /* = */
    SYN_NT(NT_EXP),           /* Expression */
    SYN_NT(NT_EOS),           /* End of statement */
    SYN_ALT,                  /* Alternative 2: string assignment */
    SYN_NT(NT_SVAR),          /* String variable */
    SYN_TOK(TOK_CEQ),         /* = */
    SYN_NT(NT_STR),           /* String expression */
    SYN_NT(NT_EOS),           /* End of statement */
    SYN_END
};
```

### Conditional: IF

**BNF**: `<IF> ::= IF <EXP> THEN <IFA> <IFELSE>?`

**Encoded**:
```c
static const SyntaxEntry syn_if[] = {
    SYN_TOK(TOK_IF),          /* IF keyword */
    SYN_NT(NT_EXP),           /* Condition expression */
    SYN_TOK(TOK_CTHEN),       /* THEN keyword */
    SYN_NT(NT_IFA),           /* IF action (line number or statement) */
    SYN_NT(NT_IFELSE),        /* Optional ELSE clause */
    SYN_END
};

static const SyntaxEntry syn_ifelse[] = {
    SYN_ALT,                  /* Alternative 1: ELSE present */
    SYN_TOK(TOK_ELSE),        /* ELSE keyword */
    SYN_NT(NT_IFA),           /* ELSE action */
    SYN_ALT,                  /* Alternative 2: no ELSE */
    SYN_EPS,                  /* Epsilon (null) */
    SYN_END
};
```

### Loop: FOR

**BNF**: `<FOR> ::= FOR <NVAR> = <EXP> TO <EXP> <FSTEP>? <EOS>`

**Encoded**:
```c
static const SyntaxEntry syn_for[] = {
    SYN_TOK(TOK_FOR),         /* FOR keyword */
    SYN_NT(NT_NVAR),          /* Loop variable */
    SYN_TOK(TOK_CEQ),         /* = */
    SYN_NT(NT_EXP),           /* Start expression */
    SYN_TOK(TOK_CTO),         /* TO keyword */
    SYN_NT(NT_EXP),           /* Limit expression */
    SYN_NT(NT_FSTEP),         /* Optional STEP clause */
    SYN_NT(NT_EOS),           /* End of statement */
    SYN_END
};

static const SyntaxEntry syn_fstep[] = {
    SYN_ALT,                  /* Alternative 1: STEP present */
    SYN_TOK(TOK_CSTEP),       /* STEP keyword */
    SYN_NT(NT_EXP),           /* Step expression */
    SYN_ALT,                  /* Alternative 2: no STEP (default 1) */
    SYN_EPS,                  /* Epsilon */
    SYN_END
};
```

### Output: PRINT

**BNF**: `<PRINT> ::= PRINT <PR1>`

**Encoded**:
```c
static const SyntaxEntry syn_print[] = {
    SYN_TOK(TOK_PRINT),       /* PRINT or ? */
    SYN_NT(NT_PR1),           /* Print list */
    SYN_END
};

/* Print list allows multiple items with separators */
static const SyntaxEntry syn_pr1[] = {
    SYN_ALT,                  /* Alternative 1: items present */
    SYN_NT(NT_PEL),           /* Print element */
    SYN_NT(NT_PR2),           /* Continue with more elements */
    SYN_ALT,                  /* Alternative 2: empty print */
    SYN_EPS,                  /* Epsilon */
    SYN_END
};
```

---

## Non-Terminal Symbols

The system defines 80+ non-terminals representing grammar rules. Major categories:

### Expressions
- `NT_EXP` - Numeric expression
- `NT_STR` - String expression
- `NT_UNARY` - Unary operator
- `NT_NOP` - Numeric operator

### Variables
- `NT_NVAR` - Numeric variable
- `NT_SVAR` - String variable
- `NT_NMAT` - Numeric array
- `NT_SMAT` - String array

### Functions
- `NT_NFUN` - Numeric function
- `NT_SFUN` - String function
- `NT_NFP` - Numeric function parameters
- `NT_SFP` - String function parameters

### Statements
- `NT_PRINT` - PRINT statement
- `NT_INPUT` - INPUT statement
- `NT_FOR` - FOR statement
- `NT_IF` - IF statement
- `NT_LET` - Assignment
- `NT_GOTO_STMT` - GOTO statement
- `NT_GOSUB_STMT` - GOSUB statement
- ... (60+ statement non-terminals)

### Structural
- `NT_EOS` - End of statement (`:` or CR)
- `NT_EXPL` - Expression list
- `NT_NSVAR` - Numeric or string variable
- `NT_PROMPT` - Optional prompt string

---

## 4. Operator Table

Defines operator precedence and parse actions for expression parsing.

### Structure
```c
typedef enum {
    PA_NONE = 0,           /* No action */
    PA_NUMBER_LITERAL,     /* Parse number literal */
    PA_STRING_LITERAL,     /* Parse string literal */
    PA_VARIABLE,           /* Parse variable */
    PA_PARENTHESIZED,      /* Parse parenthesized expression */
    PA_UNARY_PLUS,         /* Parse unary + */
    PA_UNARY_MINUS,        /* Parse unary - */
    PA_UNARY_NOT,          /* Parse unary NOT */
    PA_FUNCTION_CALL,      /* Parse function call */
    PA_BINARY_OP           /* Parse binary operator */
} ParseActionType;

typedef struct {
    unsigned char token;           /* Operator token */
    unsigned char go_on_stack;     /* Precedence when pushing */
    unsigned char come_off_stack;  /* Precedence when popping */
    void (*executor)(void);        /* Execution function (reserved) */
    ParseActionType nud;           /* Null denotation (prefix/atom) */
    ParseActionType led;           /* Left denotation (infix) */
} OperatorEntry;
```

### Precedence Levels

| Precedence | Operators | Description |
|------------|-----------|-------------|
| 8 | ^ | Exponentiation (right-associative) |
| 7 | NOT, unary +/- | Logical NOT, unary operators |
| 6 | *, / | Multiplication, division |
| 5 | +, - | Addition, subtraction |
| 4 | (none) | (gap for extensions) |
| 3 | (none) | (gap for extensions) |
| 2 | =, <>, <, <=, >, >= | Comparison operators |
| 1 | AND, OR | Logical operators |

### Operator Table Entries
```c
const OperatorEntry operator_table[] = {
    /* Atoms (nud only) */
    {TOK_NUMBER,  0, 0, NULL, PA_NUMBER_LITERAL, PA_NONE},
    {TOK_STRING,  0, 0, NULL, PA_STRING_LITERAL, PA_NONE},
    {TOK_IDENT,   0, 0, NULL, PA_VARIABLE, PA_NONE},
    {TOK_CLPRN,   0, 0, NULL, PA_PARENTHESIZED, PA_NONE},
    
    /* Binary operators (led only) */
    {TOK_CEXP,    8, 1, NULL, PA_NONE, PA_BINARY_OP},   /* ^ exponentiation */
    {TOK_CMUL,    5, 5, NULL, PA_NONE, PA_BINARY_OP},   /* * multiplication */
    {TOK_CDIV,    5, 5, NULL, PA_NONE, PA_BINARY_OP},   /* / division */
    {TOK_CEQ,     2, 2, NULL, PA_NONE, PA_BINARY_OP},   /* = equal */
    {TOK_CLT,     2, 2, NULL, PA_NONE, PA_BINARY_OP},   /* < less than */
    {TOK_CGT,     2, 2, NULL, PA_NONE, PA_BINARY_OP},   /* > greater than */
    {TOK_CLE,     2, 2, NULL, PA_NONE, PA_BINARY_OP},   /* <= less or equal */
    {TOK_CGE,     2, 2, NULL, PA_NONE, PA_BINARY_OP},   /* >= greater or equal */
    {TOK_CNE,     2, 2, NULL, PA_NONE, PA_BINARY_OP},   /* <> not equal */
    {TOK_CAND,    1, 1, NULL, PA_NONE, PA_BINARY_OP},   /* AND logical and */
    {TOK_COR,     1, 1, NULL, PA_NONE, PA_BINARY_OP},   /* OR logical or */
    
    /* Dual-role operators (both nud and led) */
    {TOK_CPLUS,   4, 4, NULL, PA_UNARY_PLUS, PA_BINARY_OP},   /* + addition / unary plus */
    {TOK_CMINUS,  4, 4, NULL, PA_UNARY_MINUS, PA_BINARY_OP},  /* - subtraction / unary minus */
    
    /* Unary operators (nud only) */
    {TOK_CNOT,    7, 7, NULL, PA_UNARY_NOT, PA_NONE},   /* NOT logical not */
    
    /* Functions (nud only) */
    {TOK_CSIN,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CCOS,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    /* ... 25 more functions ... */
    
    {0, 0, 0, NULL, PA_NONE, PA_NONE}  /* Sentinel */
};
```

### Usage in Expression Parsing

The parser uses **enum-based Pratt parsing** for expressions:

1. **Null Denotation (nud)**: Handles prefix operators and atoms (literals, variables, parentheses)
2. **Left Denotation (led)**: Handles infix/postfix operators
3. **Precedence**: Determines parsing order via `go_on_stack` values
4. **Dispatch**: Switch statements interpret ParseActionType enums from table

**Example Flow**:
```c
// Token: TOK_NUMBER
op_entry = get_operator_entry(TOK_NUMBER);
// op_entry->nud == PA_NUMBER_LITERAL
switch (op_entry->nud) {
    case PA_NUMBER_LITERAL:
        left = parse_number_literal(p, NULL);
        break;
}
```

See [Parser.md](Parser.md) for complete details on the enum-based expression parser architecture.

**Algorithm**:
- Push operators to stack while precedence increases
- Pop and emit when precedence decreases or matches
- Right-associative (^) has different go/come precedence

---

## Parser Algorithm

The table-driven parser follows this algorithm:

### Statement Parsing
```
1. Read statement token (e.g., TOK_PRINT)
2. Look up syntax rule in statement_table → NT_PRINT
3. Call parse_nonterminal(NT_PRINT)
4. Process syntax rule for NT_PRINT:
   a. For each entry in rule:
      - Terminal: Match token from input
      - Non-terminal: Recursively parse that rule
      - SYN_OR: Try alternatives until one succeeds
      - SYN_NULL: Optional element (always succeeds)
      - SYN_RTN: Return from rule
5. Build AST nodes during parsing
```

### Expression Parsing (Pratt)
```
1. Parse prefix (number, variable, function, unary operator)
2. Loop while next token is infix operator:
   a. Check precedence against current binding power
   b. If higher precedence, consume operator and parse right side
   c. Build binary operation AST node
3. Return expression AST
```

### Backtracking
The parser supports backtracking for alternatives:
```
1. Save tokenizer state
2. Try first alternative
3. If fails:
   a. Restore tokenizer state
   b. Try next alternative
4. If all alternatives fail, report syntax error
```

---

## Syntax Table Functions

### Core Functions

#### `get_syntax_rule(NonTerminal nt)`
Returns pointer to syntax rule for given non-terminal.

#### `get_statement_rule(unsigned char token)`
Returns non-terminal for given statement token.

#### `is_terminal(unsigned char token)`
Checks if token has `TC_TERMINAL` flag set.

#### `match_token_class(unsigned char token, unsigned char pattern)`
Matches token against pattern (handles wildcards).

### Parser Functions (in parser.c)

#### `parse_nonterminal(NonTerminal nt)`
Generic recursive parser for any non-terminal.

#### `parse_expression(int min_precedence)`
Pratt parser for expressions with precedence.

#### `parse_statement()`
Top-level statement parser.

---

## Grammar Coverage

### Statements Supported
- **Assignments**: LET (numeric and string)
- **I/O**: PRINT, INPUT, GET, PUT, OPEN, CLOSE, XIO, STATUS, NOTE, POINT
- **Control Flow**: IF/THEN/ELSE, FOR/NEXT, GOTO, GOSUB, RETURN, ON...GOTO/GOSUB
- **Data**: READ, DATA, RESTORE, DIM
- **Graphics**: GRAPHICS, PLOT, DRAWTO, POSITION, LOCATE, SETCOLOR, COLOR
- **Sound**: SOUND
- **System**: END, STOP, RUN, LIST, SAVE, LOAD, CLR, NEW, BYE, DOS, TRAP, POP
- **Math Mode**: DEG, RAD, RANDOMIZE
- **Microsoft BASIC**: CLS, RANDOMIZE, CLEAR, DEFINT/LNG/SNG/DBL/STR

### Functions Supported
- **Numeric**: SIN, COS, TAN, ATN, EXP, LOG, CLOG, SQR, ABS, INT, RND, SGN, PEEK, PADDLE, STICK, PTRIG, STRIG
- **String**: STR$, CHR$, LEFT$, RIGHT$, MID$, ASC, VAL, LEN, ADR
- **Special**: USR, TAB

### Operators Supported
- **Arithmetic**: +, -, *, /, ^
- **Comparison**: =, <>, <, <=, >, >=
- **Logical**: AND, OR, NOT
- **String**: + (concatenation), comparisons

---

## Extensibility

### Adding a New Statement

1. **Define Token** in `tokens.h`:
   ```c
   #define TOK_MYNEW 0x3E
   ```

2. **Add Keyword** to `keyword_table`:
   ```c
   {"MYNEW", TOK_MYNEW, 0},
   ```

3. **Define Non-Terminal** in `syntax_tables.h`:
   ```c
   NT_MYNEW_STMT,
   ```

4. **Create Syntax Rule** in `syntax_tables.c`:
   ```c
   static const SyntaxEntry syn_mynew[] = {
       SYN_TOK(TOK_MYNEW),
       SYN_NT(NT_EXP),   /* Example: takes an expression */
       SYN_NT(NT_EOS),
       SYN_END
   };
   ```

5. **Add to Statement Table**:
   ```c
   {TOK_MYNEW, NT_MYNEW_STMT},
   ```

6. **Implement Compiler** in `compiler.c`:
   ```c
   case NT_MYNEW_STMT:
       compile_mynew(comp, node);
       break;
   ```

7. **Define Bytecode** in `bytecode.h`:
   ```c
   #define OP_MYNEW 0x8E
   ```

8. **Implement VM Handler** in `vm.c`:
   ```c
   case OP_MYNEW:
       /* Execute MYNEW statement */
       break;
   ```

No changes to the parser itself are required - it automatically handles the new statement via the tables.

---

## Implementation Files

- **src/syntax_tables.h**: Opcode and structure definitions
- **src/syntax_tables.c**: Table data (1400+ lines)
- **src/parser.c**: Generic table-driven parser
- **src/tokens.h**: Token definitions

---

## Summary

The syntax table system provides:
- **100+ keywords** in keyword table
- **80+ non-terminals** defining grammar rules
- **60+ statements** with encoded syntax
- **16 operators** with precedence
- **Generic parser** - no per-statement code
- **Extensible design** - add statements via tables

This architecture follows classic table-driven BASIC design while extending it for compilation to bytecode and modern execution.
