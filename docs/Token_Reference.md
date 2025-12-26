# Token Reference - Complete Token Catalog

## Overview

This document provides a comprehensive reference for all tokens used in the BASIC compiler. Tokens are the fundamental units produced by the lexical analyzer (tokenizer) and consumed by the parser. Each token represents a keyword, operator, constant, or identifier.

## Token Organization

Tokens are organized into several categories:

1. **General Operators & Keywords** (0x10-0x6A)
2. **Special Context Operators** (0x2D-0x3C)  
3. **Functions** (0x3D-0x79)
4. **Statements** (0x00-0x3D)
5. **Special Token Types** (0x6B-0x7F)

## Token Definitions

### General Operators & Keywords (0x10-0x6A)

These tokens represent common operators, delimiters, and control keywords.

| Token | Hex | Symbol/Keyword | Description |
|-------|-----|----------------|-------------|
| TOK_CDQ | 0x10 | " | String delimiter (double quote) |
| TOK_CSOE | 0x11 | - | Start of expression (non-printing) |
| TOK_CCOM | 0x50 | , | Comma separator |
| TOK_CDOL | 0x51 | $ | Dollar sign (string variable suffix) |
| TOK_CEOS | 0x52 | : | End-of-statement colon (statement separator) |
| TOK_CSC | 0x53 | ; | Semicolon (print separator) |
| TOK_CCR | 0x54 | CR | Carriage return / end-of-line |
| TOK_CGTO | 0x55 | GOTO | GOTO keyword (expression context) |
| TOK_CGS | 0x56 | GOSUB | GOSUB keyword (expression context) |
| TOK_CTO | 0x57 | TO | TO keyword (FOR loop) |
| TOK_CSTEP | 0x58 | STEP | STEP keyword (FOR loop) |
| TOK_CTHEN | 0x59 | THEN | THEN keyword (IF statement) |
| TOK_CPND | 0x5A | # | Pound/hash (I/O channel specifier) |
| TOK_CLE | 0x5B | <= | Less-than-or-equal comparison |
| TOK_CNE | 0x5C | <> | Not-equal comparison |
| TOK_CGE | 0x5D | >= | Greater-than-or-equal comparison |
| TOK_CLT | 0x5E | < | Less-than comparison |
| TOK_CGT | 0x5F | > | Greater-than comparison |
| TOK_CEQ | 0x60 | = | Equal comparison / assignment |
| TOK_CEXP | 0x61 | ^ | Exponentiation operator |
| TOK_CMUL | 0x62 | * | Multiplication operator |
| TOK_CPLUS | 0x63 | + | Addition operator |
| TOK_CMINUS | 0x64 | - | Subtraction operator |
| TOK_CDIV | 0x65 | / | Division operator |
| TOK_CNOT | 0x66 | NOT | Logical NOT operator |
| TOK_COR | 0x67 | OR | Logical OR operator |
| TOK_CAND | 0x68 | AND | Logical AND operator |
| TOK_CLPRN | 0x69 | ( | Left parenthesis |
| TOK_CRPRN | 0x6A | ) | Right parenthesis |

### Special Context Operators (0x2D-0x3C)

These tokens represent operators whose interpretation depends on context (numeric vs. string, unary vs. binary).

| Token | Hex | Symbol | Context |
|-------|-----|--------|---------|
| TOK_CAASN | 0x2D | = | Arithmetic assignment |
| TOK_CSASN | 0x2E | = | String assignment |
| TOK_CSLE | 0x2F | <= | String less-than-or-equal |
| TOK_CSNE | 0x30 | <> | String not-equal |
| TOK_CSGE | 0x31 | >= | String greater-than-or-equal |
| TOK_CSLT | 0x32 | < | String less-than |
| TOK_CSGT | 0x33 | > | String greater-than |
| TOK_CSEQ | 0x34 | = | String equal |
| TOK_CUPLUS | 0x35 | + | Unary plus |
| TOK_CUMINUS | 0x36 | - | Unary minus |
| TOK_CSLPRN | 0x37 | ( | String/substring context |
| TOK_CALPRN | 0x38 | ( | Array subscript context |
| TOK_CDLPRN | 0x39 | ( | DIM statement context |
| TOK_CFLPRN | 0x3A | ( | Function call context |
| TOK_CDSLPR | 0x3B | ( | DIM string array context |
| TOK_CACOM | 0x3C | , | Array subscript comma |

### Functions (0x3D-0x79)

Function tokens represent built-in functions that return values.

#### String Functions

| Token | Hex | Function | Description |
|-------|-----|----------|-------------|
| TOK_CSTR | 0x3D | STR$ | Convert number to string |
| TOK_CCHR | 0x3E | CHR$ | Convert ASCII code to character |
| TOK_CLEFT | 0x75 | LEFT$ | Extract leftmost characters |
| TOK_CRIGHT | 0x76 | RIGHT$ | Extract rightmost characters |
| TOK_CMID | 0x77 | MID$ | Extract substring |

#### Numeric Functions (String Arguments)

| Token | Hex | Function | Description |
|-------|-----|----------|-------------|
| TOK_CASC | 0x40 | ASC | Get ASCII code of first character |
| TOK_CVAL | 0x41 | VAL | Convert string to number |
| TOK_CLEN | 0x42 | LEN | Get string length |
| TOK_CADR | 0x43 | ADR | Get string address (implementation-specific) |

#### Mathematical Functions

| Token | Hex | Function | Description |
|-------|-----|----------|-------------|
| TOK_CATN | 0x44 | ATN | Arctangent (inverse tangent) |
| TOK_CCOS | 0x45 | COS | Cosine (DEG/RAD mode) |
| TOK_CSIN | 0x47 | SIN | Sine (DEG/RAD mode) |
| TOK_CRND | 0x48 | RND | Random number (0 to 1) |
| TOK_CEXP_F | 0x4A | EXP | Exponential (e^x) |
| TOK_CLOG | 0x4B | LOG | Natural logarithm (ln) |
| TOK_CCLOG | 0x4C | CLOG | Common logarithm (log10) |
| TOK_CSQR | 0x4D | SQR | Square root |
| TOK_CSGN | 0x4E | SGN | Sign (-1, 0, or 1) |
| TOK_CABS | 0x4F | ABS | Absolute value |
| TOK_CINT | 0x70 | INT | Integer part (floor) |

#### System Functions

| Token | Hex | Function | Description |
|-------|-----|----------|-------------|
| TOK_CUSR | 0x3F | USR | Call machine language routine |
| TOK_CPEEK | 0x46 | PEEK | Read memory byte |
| TOK_CFRE | 0x49 | FRE | Free memory (unimplemented) |
| TOK_CPADD | 0x71 | PADDLE | Read paddle controller |
| TOK_CSTIK | 0x72 | STICK | Read joystick position |
| TOK_CPTRG | 0x73 | PTRIG | Paddle trigger status |
| TOK_CSTRG | 0x74 | STRIG | Joystick trigger status |
| TOK_CTAB | 0x79 | TAB | Tab to column (PRINT context) |

### Statements (0x00-0x3D)

Statement tokens begin executable statements.

#### Core Statements

| Token | Hex | Statement | Description |
|-------|-----|-----------|-------------|
| TOK_REM | 0x00 | REM | Remark (comment) - rest of line ignored |
| TOK_DATA | 0x01 | DATA | Define data for READ statements |
| TOK_INPUT | 0x02 | INPUT | Get user input |
| TOK_PRINT | 0x20 | PRINT | Output to screen/device |
| TOK_QUESTION | 0x28 | ? | Alias for PRINT |
| TOK_LET | 0x06 | LET | Variable assignment (optional keyword) |
| TOK_END | 0x15 | END | Terminate program |
| TOK_STOP | 0x26 | STOP | Pause program execution |

#### Control Flow

| Token | Hex | Statement | Description |
|-------|-----|-----------|-------------|
| TOK_IF | 0x07 | IF | Conditional execution |
| TOK_ELSE | 0x78 | ELSE | Alternative branch for IF |
| TOK_FOR | 0x08 | FOR | Begin FOR loop |
| TOK_NEXT | 0x09 | NEXT | End FOR loop |
| TOK_GOTO | 0x0A | GOTO | Jump to line number |
| TOK_GO_TO | 0x0B | GO TO | Two-word GOTO |
| TOK_GOSUB_S | 0x0C | GOSUB | Call subroutine |
| TOK_RETURN | 0x24 | RETURN | Return from subroutine |
| TOK_ON | 0x1E | ON | Computed GOTO/GOSUB |
| TOK_TRAP | 0x0D | TRAP | Set error trap handler |
| TOK_POP | 0x27 | POP | Remove GOSUB return address |

#### Data Management

| Token | Hex | Statement | Description |
|-------|-----|-----------|-------------|
| TOK_READ | 0x22 | READ | Read DATA values into variables |
| TOK_RESTORE | 0x23 | RESTORE | Reset DATA pointer |
| TOK_DIM | 0x14 | DIM | Declare array dimensions |
| TOK_CLR | 0x12 | CLR | Clear all variables |
| TOK_CLEAR | 0x37 | CLEAR | Alias for CLR (Microsoft BASIC) |

#### File I/O

| Token | Hex | Statement | Description |
|-------|-----|-----------|-------------|
| TOK_OPEN | 0x17 | OPEN | Open file/device |
| TOK_CLOSE | 0x11 | CLOSE | Close file/device |
| TOK_GET | 0x29 | GET | Read single byte from device |
| TOK_PUT | 0x2A | PUT | Write single byte to device |
| TOK_NOTE | 0x1B | NOTE | Get current file position |
| TOK_POINT | 0x1C | POINT | Set file position |
| TOK_STATUS | 0x1A | STATUS | Get device status |
| TOK_XIO | 0x1D | XIO | Extended I/O command |

#### Graphics & Sound

| Token | Hex | Statement | Description |
|-------|-----|-----------|-------------|
| TOK_GRAPHICS | 0x2B | GRAPHICS | Set graphics mode |
| TOK_PLOT | 0x2C | PLOT | Plot pixel |
| TOK_DRAWTO | 0x2F | DRAWTO | Draw line |
| TOK_POSITION | 0x2D | POSITION | Position cursor |
| TOK_LOCATE | 0x31 | LOCATE | Read pixel color |
| TOK_SETCOLOR | 0x30 | SETCOLOR | Set color register |
| TOK_COLOR | 0x03 | COLOR | Set drawing color |
| TOK_SOUND | 0x32 | SOUND | Generate sound |

#### System Commands

| Token | Hex | Statement | Description |
|-------|-----|-----------|-------------|
| TOK_RUN | 0x25 | RUN | Execute program |
| TOK_LIST | 0x04 | LIST | Display program listing |
| TOK_ENTER | 0x05 | ENTER | Merge program from device |
| TOK_LOAD | 0x18 | LOAD | Load program from disk |
| TOK_SAVE | 0x19 | SAVE | Save program to disk |
| TOK_CSAVE | 0x34 | CSAVE | Save to cassette |
| TOK_CLOAD | 0x35 | CLOAD | Load from cassette |
| TOK_NEW | 0x16 | NEW | Clear program memory |
| TOK_BYE | 0x0E | BYE | Exit to DOS |
| TOK_DOS | 0x2E | DOS | Enter DOS |
| TOK_CONT | 0x0F | CONT | Continue after STOP |
| TOK_COM | 0x10 | COM | Communication (unused) |
| TOK_POKE | 0x1F | POKE | Write byte to memory |
| TOK_CLS | 0x3D | CLS | Clear screen (Microsoft BASIC) |
| TOK_LPRINT | 0x33 | LPRINT | Print to printer |

#### Mathematical Mode

| Token | Hex | Statement | Description |
|-------|-----|-----------|-------------|
| TOK_DEG | 0x13 | DEG | Set degree mode for trig functions |
| TOK_RAD | 0x21 | RAD | Set radian mode for trig functions |
| TOK_RANDOMIZE | 0x36 | RANDOMIZE | Seed random number generator |

#### Type Declarations (Microsoft BASIC)

These statements are parsed but not enforced in the current implementation:

| Token | Hex | Statement | Description |
|-------|-----|-----------|-------------|
| TOK_DEFINT | 0x38 | DEFINT | Declare integer variables by letter range |
| TOK_DEFLNG | 0x39 | DEFLNG | Declare long integer variables |
| TOK_DEFSNG | 0x3A | DEFSNG | Declare single-precision variables |
| TOK_DEFDBL | 0x3B | DEFDBL | Declare double-precision variables |
| TOK_DEFSTR | 0x3C | DEFSTR | Declare string variables |

### Special Token Types (0x6B-0x7F)

These tokens represent special elements that don't correspond to keywords.

| Token | Hex | Type | Description |
|-------|-----|------|-------------|
| TOK_NUMBER | 0x6B | Constant | Numeric literal (integer or floating-point) |
| TOK_STRING | 0x6C | Constant | String literal (enclosed in quotes) |
| TOK_IDENT | 0x6D | Identifier | Variable or array name |
| TOK_EOF | 0x7F | Marker | End of file/input |

## Token Recognition Algorithm

The tokenizer uses a **longest-match algorithm** for keyword recognition:

1. **Whitespace Handling**: Spaces and tabs are skipped (but not newlines)
2. **String Literals**: Quoted text `"..."` → TOK_STRING
3. **Numeric Literals**: Digits or `.digit` → TOK_NUMBER (parsed as floating-point)
4. **Multi-Character Operators**: `<=`, `<>`, `>=` recognized before single-character operators
5. **Single-Character Operators**: `,`, `:`, `;`, `#`, `=`, `^`, `*`, `+`, `-`, `/`, `(`, `)`, etc.
6. **Keywords and Identifiers**:
   - Scan alphabetic character sequences
   - Try matching against keyword table (case-insensitive)
   - Longest matching keyword wins
   - Non-matching text becomes TOK_IDENT
   - Variables ending in `$` are string variables
7. **Special Cases**:
   - `'` (apostrophe) → TOK_REM (rest of line is comment)
   - `?` → TOK_QUESTION (alias for PRINT)
   - `REM` statements consume rest of line

## Microsoft BASIC Compatibility

The tokenizer supports Microsoft BASIC's convention of allowing keywords to immediately follow variable names without spaces:

- `IFFPRINT` → `IF F PRINT` (IF + variable F + PRINT)
- `FORX=1TO10` → `FOR X=1 TO 10`
- `GOTOLINE` → `GOTO LINE` (GOTO + variable LINE)

This is handled by checking if a keyword begins at each position after consuming identifier characters.

## Token Usage in Parser

Tokens are classified for parsing:

- **Terminal Tokens**: Direct matches in syntax (have TC_TERMINAL flag = 0x80)
- **Non-Terminal Tokens**: Trigger syntax rule lookups
- **Operators**: Pratt parsing with precedence levels
- **Delimiters**: Statement separators (`:`, CR) and expression delimiters (`,`, `;`, `)`)

## Token Precedence (Expression Parsing)

Operator tokens have precedence levels for expression parsing:

| Precedence | Operators | Associativity |
|------------|-----------|---------------|
| 1 (lowest) | OR | Left |
| 2 | AND | Left |
| 3 | NOT | Right (unary) |
| 4 | =, <>, <, <=, >, >= | Left |
| 5 | +, - (binary) | Left |
| 6 | *, / | Left |
| 7 | MOD | Left |
| 8 | ^  | Right |
| 9 (highest) | +, - (unary) | Right |

## Token Storage

Each token structure contains:
- `type` (uint8_t): Token type code
- `line` (int): Source line number
- `column` (int): Column position  
- `text` (char*): Original text (for identifiers, strings)
- `value` (double): Numeric value (for TOK_NUMBER)

## Implementation Notes

1. **Case Insensitivity**: All keyword matching is case-insensitive
2. **Memory Management**: Token text is dynamically allocated and must be freed
3. **Lookahead**: Tokenizer maintains 1-token lookahead for parser
4. **Backtracking**: Parser can save/restore tokenizer state for backtracking
5. **Line Tracking**: Line and column information preserved for error messages

## Summary

The token system provides 100+ distinct tokens covering:
- 60+ statement keywords
- 30+ function keywords  
- 25+ operator symbols
- Type and context-specific variants
- Special markers (EOF, constants, identifiers)

All tokens are defined in [src/tokens.h](../src/tokens.h) and recognized by the tokenizer in [src/tokenizer.c](../src/tokenizer.c).
