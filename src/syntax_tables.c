/* syntax_tables.c - Table-Driven BASIC Syntax Tables Implementation */
#include "syntax_tables.h"
#include "parser.h"
#include <stddef.h>
#include <stdio.h>

/* Keyword table - maps text to tokens */
const KeywordEntry keyword_table[] = {
    /* Statements */
    {"REM", TOK_REM, 0},
    {"DATA", TOK_DATA, 0},
    {"INPUT", TOK_INPUT, 0},
    {"COLOR", TOK_COLOR, 0},
    {"LIST", TOK_LIST, 0},
    {"ENTER", TOK_ENTER, 0},
    {"LET", TOK_LET, 0},
    {"IF", TOK_IF, 0},
    {"FOR", TOK_FOR, 0},
    {"NEXT", TOK_NEXT, 0},
    {"GOTO", TOK_CGTO, 0},
    {"GO", TOK_CGTO, 0},
    {"GOSUB", TOK_CGS, 0},
    {"TRAP", TOK_TRAP, 0},
    {"BYE", TOK_BYE, 0},
    {"CONT", TOK_CONT, 0},
    {"COM", TOK_COM, 0},
    {"CLOSE", TOK_CLOSE, 0},
    {"CLR", TOK_CLR, 0},
    {"DEG", TOK_DEG, 0},
    {"DIM", TOK_DIM, 0},
    {"END", TOK_END, 0},
    {"NEW", TOK_NEW, 0},
    {"OPEN", TOK_OPEN, 0},
    {"LOAD", TOK_LOAD, 0},
    {"SAVE", TOK_SAVE, 0},
    {"STATUS", TOK_STATUS, 0},
    {"NOTE", TOK_NOTE, 0},
    {"POINT", TOK_POINT, 0},
    {"XIO", TOK_XIO, 0},
    {"ON", TOK_ON, 0},
    {"POKE", TOK_POKE, 0},
    {"PRINT", TOK_PRINT, 0},
    {"RAD", TOK_RAD, 0},
    {"READ", TOK_READ, 0},
    {"RESTORE", TOK_RESTORE, 0},
    {"RETURN", TOK_RETURN, 0},
    {"RUN", TOK_RUN, 0},
    {"STOP", TOK_STOP, 0},
    {"POP", TOK_POP, 0},
    {"GET", TOK_GET, 0},
    {"PUT", TOK_PUT, 0},
    {"GRAPHICS", TOK_GRAPHICS, 0},
    {"PLOT", TOK_PLOT, 0},
    {"POSITION", TOK_POSITION, 0},
    {"DOS", TOK_DOS, 0},
    {"DRAWTO", TOK_DRAWTO, 0},
    {"SETCOLOR", TOK_SETCOLOR, 0},
    {"LOCATE", TOK_LOCATE, 0},
    {"SOUND", TOK_SOUND, 0},
    {"LPRINT", TOK_LPRINT, 0},
    {"CSAVE", TOK_CSAVE, 0},
    {"CLOAD", TOK_CLOAD, 0},
    {"RANDOMIZE", TOK_RANDOMIZE, 0},
    {"CLEAR", TOK_CLEAR, 0},
    {"DEFINT", TOK_DEFINT, 0},
    {"DEFLNG", TOK_DEFLNG, 0},
    {"DEFSNG", TOK_DEFSNG, 0},
    {"DEFDBL", TOK_DEFDBL, 0},
    {"DEFSTR", TOK_DEFSTR, 0},
    {"CLS", TOK_CLS, 0},
    
    /* Keywords */
    {"TO", TOK_CTO, 1},
    {"STEP", TOK_CSTEP, 1},
    {"THEN", TOK_CTHEN, 1},
    {"ELSE", TOK_ELSE, 1},
    
    /* Operators */
    {"NOT", TOK_CNOT, 2},
    {"OR", TOK_COR, 2},
    {"AND", TOK_CAND, 2},
    
    /* Functions */
    {"STR$", TOK_CSTR, 3},
    {"CHR$", TOK_CCHR, 3},
    {"USR", TOK_CUSR, 3},
    {"ASC", TOK_CASC, 3},
    {"VAL", TOK_CVAL, 3},
    {"LEN", TOK_CLEN, 3},
    {"ADR", TOK_CADR, 3},
    {"ATN", TOK_CATN, 3},
    {"COS", TOK_CCOS, 3},
    {"PEEK", TOK_CPEEK, 3},
    {"SIN", TOK_CSIN, 3},
    {"RND", TOK_CRND, 3},
    {"FRE", TOK_CFRE, 3},
    {"EXP", TOK_CEXP_F, 3},
    {"LOG", TOK_CLOG, 3},
    {"CLOG", TOK_CCLOG, 3},
    {"SQR", TOK_CSQR, 3},
    {"SGN", TOK_CSGN, 3},
    {"ABS", TOK_CABS, 3},
    {"CINT", TOK_CINT, 3},  /* Microsoft BASIC alias - must come before INT for longest match */
    {"INT", TOK_CINT, 3},
    {"PADDLE", TOK_CPADD, 3},
    {"STICK", TOK_CSTIK, 3},
    {"PTRIG", TOK_CPTRG, 3},
    {"STRIG", TOK_CSTRG, 3},
    {"LEFT$", TOK_CLEFT, 3},
    {"RIGHT$", TOK_CRIGHT, 3},
    {"MID$", TOK_CMID, 3},
    {"TAB", TOK_CTAB, 3},
    
    {NULL, 0, 0}  /* Sentinel */
};

const int keyword_table_size = sizeof(keyword_table) / sizeof(KeywordEntry) - 1;

/* Operator precedence table (compatible with classic BASIC) */
/* Now with enum-based parse actions (purely data-driven) */
const OperatorEntry operator_table[] = {
    /* Token, Go-on-stack, Come-off-stack, Executor, Nud, Led */
    
    /* Atoms/Primatives (only nud) */
    {TOK_NUMBER,  0, 0, NULL, PA_NUMBER_LITERAL, PA_NONE},
    {TOK_STRING,  0, 0, NULL, PA_STRING_LITERAL, PA_NONE},
    {TOK_IDENT,   0, 0, NULL, PA_VARIABLE, PA_NONE},
    {TOK_CLPRN,   0, 0, NULL, PA_PARENTHESIZED, PA_NONE},
    
    /* Binary operators (only led) */
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
    
    /* Unary-only operators (only nud) */
    {TOK_CNOT,    7, 7, NULL, PA_UNARY_NOT, PA_NONE},   /* NOT logical not */
    {TOK_CUPLUS,  7, 7, NULL, PA_NONE, PA_NONE},   /* unary + token (unused, handled by TOK_CPLUS) */
    {TOK_CUMINUS, 7, 7, NULL, PA_NONE, PA_NONE},   /* unary - token (unused, handled by TOK_CMINUS) */
    
    /* Functions (all use nud for function call parsing) */
    {TOK_CSIN,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CCOS,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CATN,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CEXP_F,  0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CLOG,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CCLOG,   0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CSQR,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CABS,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CINT,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CSGN,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CRND,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CFRE,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CPEEK,   0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CPADD,   0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CSTIK,   0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CPTRG,   0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CSTRG,   0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CASC,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CVAL,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CLEN,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CADR,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CSTR,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CCHR,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CLEFT,   0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CRIGHT,  0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CMID,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    {TOK_CTAB,    0, 0, NULL, PA_FUNCTION_CALL, PA_NONE},
    
    {0, 0, 0, NULL, PA_NONE, PA_NONE}  /* Sentinel */
};

const int operator_table_size = sizeof(operator_table) / sizeof(OperatorEntry) - 1;

/* Function metadata table (for arity validation and error messages) */
const FunctionEntry function_table[] = {
    /* Numeric functions */
    {TOK_CSIN,    "SIN",    1, 1},
    {TOK_CCOS,    "COS",    1, 1},
    {TOK_CATN,    "ATN",    1, 1},
    {TOK_CEXP_F,  "EXP",    1, 1},
    {TOK_CLOG,    "LOG",    1, 1},
    {TOK_CCLOG,   "CLOG",   1, 1},
    {TOK_CSQR,    "SQR",    1, 1},
    {TOK_CABS,    "ABS",    1, 1},
    {TOK_CINT,    "INT",    1, 1},
    {TOK_CSGN,    "SGN",    1, 1},
    {TOK_CRND,    "RND",    1, 1},
    {TOK_CFRE,    "FRE",    1, 1},
    {TOK_CPEEK,   "PEEK",   1, 1},
    {TOK_CPADD,   "PADDLE",  1, 1},
    {TOK_CSTIK,   "STICK",  1, 1},
    {TOK_CPTRG,   "PTRIG",  1, 1},
    {TOK_CSTRG,   "STRIG",  1, 1},
    /* String functions */
    {TOK_CASC,    "ASC",    1, 1},
    {TOK_CVAL,    "VAL",    1, 1},
    {TOK_CLEN,    "LEN",    1, 1},
    {TOK_CADR,    "ADR",    1, 1},
    {TOK_CSTR,    "STR$",   1, 1},
    {TOK_CCHR,    "CHR$",   1, 1},
    {TOK_CLEFT,   "LEFT$",  2, 2},
    {TOK_CRIGHT,  "RIGHT$", 2, 2},
    {TOK_CMID,    "MID$",   2, 3},
    /* Special functions */
    {TOK_CTAB,    "TAB",    1, 1},
    {0, NULL, 0, 0}  /* Sentinel */
};

const int function_table_size = sizeof(function_table) / sizeof(FunctionEntry) - 1;

/* Helper macro for syntax table construction */
#define SYN_TOK(t) {(t) | TC_TERMINAL, {0, 0, 0}}
#define SYN_NT(nt) {SYN_ANTV, {(nt) & 0xFF, ((nt) >> 8) & 0xFF, 0}}
#define SYN_OP(op) {(op), {0, 0, 0}}
#define SYN_END {SYN_RTN, {0, 0, 0}}
#define SYN_ALT {SYN_OR, {0, 0, 0}}
#define SYN_EPS {SYN_NULL, {0, 0, 0}}

/* Terminal handling - match specific token or token class */
#define T_IDENT    (TOK_IDENT | TC_TERMINAL)
#define T_NUMBER   (TOK_NUMBER | TC_TERMINAL)
#define T_STRING   (TOK_STRING | TC_TERMINAL)

/* Simplified syntax table encoding - represents grammar rules
 * This is a compact representation that will be expanded during initialization
 * Format: Each non-terminal starts with its ID, followed by rule bytes, ended by SYN_RTN
 */

/* <TNVAR> = identifier # */
static const SyntaxEntry syn_tnvar[] = {
    SYN_TOK(TOK_IDENT),
    SYN_END
};

/* <NCON> = number # */
static const SyntaxEntry syn_ncon[] = {
    SYN_TOK(TOK_NUMBER),
    SYN_END
};

/* <SCON> = string # */
static const SyntaxEntry syn_scon[] = {
    SYN_TOK(TOK_STRING),
    SYN_END
};

/* <TSVAR> = identifier$ # */
static const SyntaxEntry syn_tsvar[] = {
    SYN_TOK(TOK_IDENT),
    SYN_END
};

/* <SFUN> = (STR$|CHR$) <NFP> | (LEFT$|RIGHT$) <SF2P> | MID$ <SF3P> # */
static const SyntaxEntry syn_sfun[] = {
    SYN_ALT,
    SYN_TOK(TOK_CSTR),
    SYN_NT(NT_NFP),
    SYN_ALT,
    SYN_TOK(TOK_CCHR),
    SYN_NT(NT_NFP),
    SYN_ALT,
    SYN_NT(NT_SFNP),
    SYN_NT(NT_SF2P),
    SYN_ALT,
    SYN_NT(NT_SFMID),
    SYN_NT(NT_SF3P),
    SYN_END
};

/* <SOP> = = | <> | < | > | <= | >= # */
static const SyntaxEntry syn_sop[] = {
    SYN_ALT,
    SYN_TOK(TOK_CEQ),
    SYN_ALT,
    SYN_TOK(TOK_CNE),
    SYN_ALT,
    SYN_TOK(TOK_CLT),
    SYN_ALT,
    SYN_TOK(TOK_CGT),
    SYN_ALT,
    SYN_TOK(TOK_CLE),
    SYN_ALT,
    SYN_TOK(TOK_CGE),
    SYN_END
};

/* <NFNP> = SIN | COS | ATN | LOG | CLOG | SQR | SGN | ABS | INT | EXP | RND | FRE | PEEK | PADDLE | STICK | PTRIG | STRIG # */
static const SyntaxEntry syn_nfnp[] = {
    SYN_ALT,
    SYN_TOK(TOK_CSIN),
    SYN_ALT,
    SYN_TOK(TOK_CCOS),
    SYN_ALT,
    SYN_TOK(TOK_CATN),
    SYN_ALT,
    SYN_TOK(TOK_CLOG),
    SYN_ALT,
    SYN_TOK(TOK_CCLOG),
    SYN_ALT,
    SYN_TOK(TOK_CSQR),
    SYN_ALT,
    SYN_TOK(TOK_CSGN),
    SYN_ALT,
    SYN_TOK(TOK_CABS),
    SYN_ALT,
    SYN_TOK(TOK_CINT),
    SYN_ALT,
    SYN_TOK(TOK_CEXP_F),
    SYN_ALT,
    SYN_TOK(TOK_CTAB),
    SYN_ALT,
    SYN_TOK(TOK_CRND),
    SYN_ALT,
    SYN_TOK(TOK_CFRE),
    SYN_ALT,
    SYN_TOK(TOK_CPEEK),
    SYN_ALT,
    SYN_TOK(TOK_CPADD),
    SYN_ALT,
    SYN_TOK(TOK_CSTIK),
    SYN_ALT,
    SYN_TOK(TOK_CPTRG),
    SYN_ALT,
    SYN_TOK(TOK_CSTRG),
    SYN_END
};

/* <NFP> = ( <EXP> ) # */
static const SyntaxEntry syn_nfp[] = {
    SYN_TOK(TOK_CLPRN),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CRPRN),
    SYN_END
};

/* <NFSP> = ASC | VAL | LEN | ADR # */
static const SyntaxEntry syn_nfsp[] = {
    SYN_ALT,
    SYN_TOK(TOK_CASC),
    SYN_ALT,
    SYN_TOK(TOK_CVAL),
    SYN_ALT,
    SYN_TOK(TOK_CLEN),
    SYN_ALT,
    SYN_TOK(TOK_CADR),
    SYN_END
};

/* <SFP> = ( <STR> ) # */
static const SyntaxEntry syn_sfp[] = {
    SYN_TOK(TOK_CLPRN),
    SYN_NT(NT_STR),
    SYN_TOK(TOK_CRPRN),
    SYN_END
};

/* <SFNP> = LEFT$ | RIGHT$ # */
static const SyntaxEntry syn_sfnp[] = {
    SYN_ALT,
    SYN_TOK(TOK_CLEFT),
    SYN_ALT,
    SYN_TOK(TOK_CRIGHT),
    SYN_END
};

/* <SF2P> = ( <STR> , <EXP> ) # */
static const SyntaxEntry syn_sf2p[] = {
    SYN_TOK(TOK_CLPRN),
    SYN_NT(NT_STR),
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CRPRN),
    SYN_END
};

/* <SFMID> = MID$ # */
static const SyntaxEntry syn_sfmid[] = {
    SYN_TOK(TOK_CMID),
    SYN_END
};

/* <SF3P> = ( <STR> , <EXP> , <EXP> ) # */
/* Used for MID$(str$, start, length) */
static const SyntaxEntry syn_sf3p[] = {
    SYN_TOK(TOK_CLPRN),
    SYN_NT(NT_STR),
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CRPRN),
    SYN_END
};

/* <NFUSR> = USR # */
static const SyntaxEntry syn_nfusr[] = {
    SYN_TOK(TOK_CUSR),
    SYN_END
};

/* <TNCON> = number # */
static const SyntaxEntry syn_tncon[] = {
    SYN_TOK(TOK_NUMBER),
    SYN_END
};

/* <NVAR> = <TNVAR> <NMAT> # */
static const SyntaxEntry syn_nvar[] = {
    SYN_NT(NT_TNVAR),
    SYN_NT(NT_NMAT),
    SYN_END
};

/* <NMAT> = ( <EXP> <NMAT2> ) | & # */
static const SyntaxEntry syn_nmat[] = {
    SYN_ALT,
    SYN_TOK(TOK_CLPRN),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_NMAT2),
    SYN_TOK(TOK_CRPRN),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <NMAT2> = , <EXP> | & # */
static const SyntaxEntry syn_nmat2[] = {
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <EXP> = ( <EXP> ) <NOP> | <UNARY> <EXP> | <NV> <NOP> # */
static const SyntaxEntry syn_exp[] = {
    SYN_ALT,
    SYN_TOK(TOK_CLPRN),
    {SYN_VEXP, {0, 0, 0}},  /* Recursive <EXP> */
    SYN_TOK(TOK_CRPRN),
    SYN_NT(NT_NOP),
    SYN_ALT,
    SYN_NT(NT_UNARY),
    {SYN_VEXP, {0, 0, 0}},  /* <EXP> */
    SYN_ALT,
    SYN_NT(NT_NV),
    SYN_NT(NT_NOP),
    SYN_END
};

/* <UNARY> = + | - | NOT # */
static const SyntaxEntry syn_unary[] = {
    SYN_ALT,
    SYN_TOK(TOK_CPLUS),
    SYN_ALT,
    SYN_TOK(TOK_CMINUS),
    SYN_ALT,
    SYN_TOK(TOK_CNOT),
    SYN_END
};

/* <NV> = <NFUN> | <NVAR> | <NCON> | <STCOMP> # */
static const SyntaxEntry syn_nv[] = {
    SYN_ALT,
    SYN_NT(NT_NFUN),
    SYN_ALT,
    SYN_NT(NT_NVAR),
    SYN_ALT,
    SYN_NT(NT_NCON),
    SYN_ALT,
    SYN_NT(NT_STCOMP),
    SYN_END
};

/* <NOP> = <OP> <EXP> | & # */
static const SyntaxEntry syn_nop[] = {
    SYN_ALT,
    SYN_NT(NT_OP),
    {SYN_VEXP, {0, 0, 0}},
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <OP> = ^ | * | / | <= | >= | <> | < | > | = | AND | OR # */
static const SyntaxEntry syn_op[] = {
    SYN_ALT,
    SYN_TOK(TOK_CEXP),
    SYN_ALT,
    SYN_TOK(TOK_CMUL),
    SYN_ALT,
    SYN_TOK(TOK_CDIV),
    SYN_ALT,
    SYN_TOK(TOK_CLE),
    SYN_ALT,
    SYN_TOK(TOK_CGE),
    SYN_ALT,
    SYN_TOK(TOK_CNE),
    SYN_ALT,
    SYN_TOK(TOK_CLT),
    SYN_ALT,
    SYN_TOK(TOK_CGT),
    SYN_ALT,
    SYN_TOK(TOK_CEQ),
    SYN_ALT,
    SYN_TOK(TOK_CAND),
    SYN_ALT,
    SYN_TOK(TOK_COR),
    SYN_END
};

/* <NFUN> = <NFNP> <NFP> | <NFSP> <SFP> | <NFUSR> # */
static const SyntaxEntry syn_nfun[] = {
    SYN_ALT,
    SYN_NT(NT_NFNP),
    SYN_NT(NT_NFP),
    SYN_ALT,
    SYN_NT(NT_NFSP),
    SYN_NT(NT_SFP),
    SYN_ALT,
    SYN_NT(NT_NFUSR),
    SYN_END
};

/* <STCOMP> = <STR> <SOP> <STR> # */
static const SyntaxEntry syn_stcomp[] = {
    SYN_NT(NT_STR),
    SYN_NT(NT_SOP),
    SYN_NT(NT_STR),
    SYN_END
};

/* <STR> = <SFUN> | <SVAR> | <SCON> # */
static const SyntaxEntry syn_str[] = {
    SYN_ALT,
    SYN_NT(NT_SFUN),
    SYN_ALT,
    SYN_NT(NT_SVAR),
    SYN_ALT,
    SYN_NT(NT_SCON),
    SYN_END
};

/* <SVAR> = <TSVAR> <SMAT> # */
static const SyntaxEntry syn_svar[] = {
    SYN_NT(NT_TSVAR),
    SYN_NT(NT_SMAT),
    SYN_END
};

/* <SMAT> = ( <EXP> <SMAT2> ) | & # */
static const SyntaxEntry syn_smat[] = {
    SYN_ALT,
    SYN_TOK(TOK_CLPRN),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_SMAT2),
    SYN_TOK(TOK_CRPRN),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <SMAT2> = , <EXP> | & # */
static const SyntaxEntry syn_smat2[] = {
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <EOS> = epsilon # */
/* EOS is now handled by parse_program, not by syntax rules */
/* This just provides an epsilon (empty) production */
static const SyntaxEntry syn_eos[] = {
    SYN_EPS,
    SYN_END
};

/* ===== STATEMENT SYNTAX RULES ===== */

/* <REM> = <REM_BODY> <EOS> # */
static const SyntaxEntry syn_rem[] = {
    SYN_NT(NT_REM_BODY),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <REM_BODY> = (special handling - consumes until EOS) # */
static const SyntaxEntry syn_rem_body[] = {
    SYN_EPS,
    SYN_END
};

/* <LET> = <NVAR> = <EXP> <EOS> | <SVAR> = <STR> <EOS> # */
static const SyntaxEntry syn_let[] = {
    SYN_ALT,
    SYN_NT(NT_NVAR),
    SYN_TOK(TOK_CEQ),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_SVAR),
    SYN_TOK(TOK_CEQ),
    SYN_NT(NT_STR),
    SYN_NT(NT_EOS),
    SYN_END
};

/* Simplified PRINT grammar without left recursion - for proof of concept */
/* <PRINT> = # <EXP> , <PR1> <EOS> | <PR1> <EOS> # Support PRINT #channel or PRINT */
static const SyntaxEntry syn_print[] = {
    SYN_ALT,
    SYN_TOK(TOK_CPND),      /* # */
    SYN_NT(NT_EXP),          /* channel expression */
    SYN_TOK(TOK_CCOM),       /* , */
    SYN_NT(NT_PR1),          /* print list */
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_PR1),          /* print list without channel */
    SYN_NT(NT_EOS),
    SYN_END
};

/* <PR1> = <PES> <PR2> | & # Print list: first element and tail, or empty */
static const SyntaxEntry syn_pr1[] = {
    SYN_ALT,
    SYN_NT(NT_PES),
    SYN_NT(NT_PR2),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <PR2> = <PS> <PES> <PR2> | <PS> | & # Print tail: sep+element+tail, or trailing sep, or empty */
static const SyntaxEntry syn_pr2[] = {
    SYN_ALT,
    SYN_NT(NT_PS),
    SYN_NT(NT_PES),
    SYN_NT(NT_PR2),
    SYN_ALT,
    SYN_NT(NT_PS),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <PEL> = (simplified - not used) # */
static const SyntaxEntry syn_pel[] = {
    SYN_EPS,
    SYN_END
};

/* <PES> = <EXP> | <STR> # */
static const SyntaxEntry syn_pes[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_ALT,
    SYN_NT(NT_STR),
    SYN_END
};

/* <PELA> = (simplified - not used) # */
static const SyntaxEntry syn_pela[] = {
    SYN_EPS,
    SYN_END
};

/* <PSL> = (simplified - not used) # */
static const SyntaxEntry syn_psl[] = {
    SYN_EPS,
    SYN_END
};

/* <PSLA> = (simplified - not used) # */
static const SyntaxEntry syn_psla[] = {
    SYN_EPS,
    SYN_END
};

/* <PS> = , | ; # */
static const SyntaxEntry syn_ps[] = {
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    SYN_ALT,
    SYN_TOK(TOK_CSC),
    SYN_END
};

/* <GOTO> = <EXP> <EOS> # */
static const SyntaxEntry syn_goto[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <GOSUB> = <EXP> <EOS> # */
static const SyntaxEntry syn_gosub[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <END> = <EOS> # */
static const SyntaxEntry syn_end[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <STOP> = <EOS> # */
static const SyntaxEntry syn_stop[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <RETURN> = <EOS> # */
static const SyntaxEntry syn_return[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <FOR> = <TNVAR> = <EXP> TO <EXP> <FSTEP> <EOS> # */
static const SyntaxEntry syn_for[] = {
    SYN_NT(NT_TNVAR),
    SYN_TOK(TOK_CEQ),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CTO),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_FSTEP),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <FSTEP> = STEP <EXP> | & # */
static const SyntaxEntry syn_fstep[] = {
    SYN_ALT,
    SYN_TOK(TOK_CSTEP),
    {SYN_VEXP, {0, 0, 0}},
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <NEXT> = <TNVAR> <EOS> | <EOS> # */
/* Variable is optional - NEXT without variable closes most recent FOR loop */
/* <NEXTVL> = <TNVAR> | <TNVAR> , <NEXTVL> # */
static const SyntaxEntry syn_nextvl[] = {
    SYN_ALT,
    SYN_NT(NT_TNVAR),
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_NEXTVL),
    SYN_ALT,
    SYN_NT(NT_TNVAR),
    SYN_END
};

/* <NEXT> = <NEXTVL> <EOS> | <EOS> # */
static const SyntaxEntry syn_next[] = {
    SYN_ALT,
    SYN_NT(NT_NEXTVL),
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <IF> = <EXP> THEN <IFBODY> <IFELSE> <EOS> | <EXP> <IFBODY> <IFELSE> <EOS> # */
/* Two alternatives: with THEN and without THEN (compact syntax) */
/* <IFELSE> is optional ELSE clause */
static const SyntaxEntry syn_if[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CTHEN),
    SYN_NT(NT_IFBODY),
    SYN_NT(NT_IFELSE),
    SYN_NT(NT_EOS),
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_IFBODY),
    SYN_NT(NT_IFELSE),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <IFBODY> = <TNCON> | <STATEMENT> # */
/* This will be handled specially in parser to stop at ELSE */
static const SyntaxEntry syn_ifbody[] = {
    SYN_ALT,
    SYN_NT(NT_TNCON),
    SYN_ALT,
    SYN_NT(NT_STATEMENT),
    SYN_END
};

/* <IFA> = <TNCON> | <STATEMENT> # */
static const SyntaxEntry syn_ifa[] = {
    SYN_ALT,
    SYN_NT(NT_TNCON),
    SYN_ALT,
    SYN_NT(NT_STATEMENT),
    SYN_END
};

/* <IFELSE> = ELSE <IFA> | epsilon # */
static const SyntaxEntry syn_ifelse[] = {
    SYN_ALT,
    SYN_TOK(TOK_ELSE),
    SYN_NT(NT_IFA),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <POKE> = <EXP> , <EXP> <EOS> # */
static const SyntaxEntry syn_poke[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <GRAPHICS> = <EXP> <EOS> # */
static const SyntaxEntry syn_graphics[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <PLOT> = <EXP> , <EXP> <EOS> # */
static const SyntaxEntry syn_plot[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <POSITION> = <EXP> , <EXP> <EOS> # */
static const SyntaxEntry syn_position[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <DRAWTO> = <EXP> , <EXP> <EOS> # */
static const SyntaxEntry syn_drawto[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <SETCOLOR> = <EXP> , <EXP> , <EXP> <EOS> # */
static const SyntaxEntry syn_setcolor[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <SOUND> = <EXP> , <EXP> , <EXP> , <EXP> <EOS> # */
static const SyntaxEntry syn_sound[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <INPUT> = <OPD> <PROMPT> <READ> # */
static const SyntaxEntry syn_input[] = {
    SYN_NT(NT_OPD),
    SYN_NT(NT_PROMPT),
    SYN_NT(NT_READ),
    SYN_END
};

/* <PROMPT> = <STR> <PS> | & # Optional prompt with separator */
static const SyntaxEntry syn_prompt[] = {
    SYN_ALT,
    SYN_NT(NT_STR),
    SYN_NT(NT_PS),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <READ> = <NSVRL> <EOS> # */
static const SyntaxEntry syn_read[] = {
    SYN_NT(NT_NSVRL),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <NSVAR> = <NVAR> | <SVAR> # */
static const SyntaxEntry syn_nsvar[] = {
    SYN_ALT,
    SYN_NT(NT_NVAR),
    SYN_ALT,
    SYN_NT(NT_SVAR),
    SYN_END
};

/* <NSVRL> = <NSVAR> <NSV2> # */
static const SyntaxEntry syn_nsvrl[] = {
    SYN_NT(NT_NSVAR),
    SYN_NT(NT_NSV2),
    SYN_END
};

/* <NSV2> = , <NSVRL> | & # */
static const SyntaxEntry syn_nsv2[] = {
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_NSVRL),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <OPD> = # <D1> , | & # */
static const SyntaxEntry syn_opd[] = {
    SYN_ALT,
    SYN_TOK(TOK_CPND),
    SYN_NT(NT_D1),
    SYN_TOK(TOK_CCOM),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <D1> = <EXP> # */
static const SyntaxEntry syn_d1[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_END
};

/* <DATA> = <DATA_LIST> <EOS> # */
static const SyntaxEntry syn_data[] = {
    SYN_NT(NT_DATA_LIST),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <DATA_LIST> = <DATA_VAL> <DATA_TAIL> # */
static const SyntaxEntry syn_data_list[] = {
    SYN_NT(NT_DATA_VAL),
    SYN_NT(NT_DATA_TAIL),
    SYN_END
};

/* <DATA_TAIL> = , <DATA_VAL> <DATA_TAIL> | , <DATA_TAIL> | ε # */
static const SyntaxEntry syn_data_tail[] = {
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_DATA_VAL),
    SYN_NT(NT_DATA_TAIL),
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_DATA_TAIL),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <DATA_VAL> = NUMBER | STRING | IDENT | - | + # */
static const SyntaxEntry syn_data_val[] = {
    SYN_ALT,
    SYN_TOK(TOK_NUMBER),
    SYN_ALT,
    SYN_TOK(TOK_STRING),
    SYN_ALT,
    SYN_TOK(TOK_IDENT),
    SYN_ALT,
    SYN_TOK(TOK_CMINUS),
    SYN_ALT,
    SYN_TOK(TOK_CPLUS),
    SYN_END
};

/* <RESTORE> = <EXP> <EOS> | <EOS> # */
static const SyntaxEntry syn_restore[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <DIM> = <NSML> <EOS> # */
static const SyntaxEntry syn_dim[] = {
    SYN_NT(NT_NSML),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <NSMAT> = <TNVAR> ( <EXP> <NMAT2> ) # */
static const SyntaxEntry syn_nsmat[] = {
    SYN_NT(NT_TNVAR),
    SYN_TOK(TOK_CLPRN),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_NMAT2),
    SYN_TOK(TOK_CRPRN),
    SYN_END
};

/* <NSML> = <NSMAT> <NSML2> # */
static const SyntaxEntry syn_nsml[] = {
    SYN_NT(NT_NSMAT),
    SYN_NT(NT_NSML2),
    SYN_END
};

/* <NSML2> = , <NSML> | & # */
static const SyntaxEntry syn_nsml2[] = {
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_NSML),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <ON> = <EXP> <ON1> <EXPL> <EOS> # */
static const SyntaxEntry syn_on[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_ON1),
    SYN_NT(NT_EXPL),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <ON1> = GOTO | GOSUB # */
static const SyntaxEntry syn_on1[] = {
    SYN_ALT,
    SYN_TOK(TOK_CGTO),
    SYN_ALT,
    SYN_TOK(TOK_CGS),
    SYN_END
};

/* <EXPL> = <EXP> <EXPL1> # */
static const SyntaxEntry syn_expl[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EXPL1),
    SYN_END
};

/* <EXPL1> = , <EXPL> | & # */
static const SyntaxEntry syn_expl1[] = {
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_EXPL),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <CLR> = <EOS> # */
static const SyntaxEntry syn_clr[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <DEG> = <EOS> # */
static const SyntaxEntry syn_deg[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <RAD> = <EOS> # */
static const SyntaxEntry syn_rad[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <RANDOMIZE> = <EXP> <EOS> | <EOS> # */
static const SyntaxEntry syn_randomize[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <POP> = <EOS> # */
static const SyntaxEntry syn_pop[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <TRAP> = <EXP> <EOS> | <EOS> # */
static const SyntaxEntry syn_trap[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <CONT> = <EOS> # */
static const SyntaxEntry syn_cont[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <BYE> = <EOS> # */
static const SyntaxEntry syn_bye[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <RUN> = <EOS> # */
static const SyntaxEntry syn_run[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <LIST> = <EOS> # */
static const SyntaxEntry syn_list[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <SAVE> = <EOS> # */
static const SyntaxEntry syn_save[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* <CLEAR> = <EXP> <EOS> | , <CLRP1> <EOS> | <EOS> # */
/* Accepts: CLEAR | CLEAR expr | CLEAR, | CLEAR,expr | CLEAR,,expr | CLEAR,expr,expr */
static const SyntaxEntry syn_clear[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_CLRP1),
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <CLRP1> = <EXP> <CLRP2> | <CLRP2> | & # First param (optional) and continuation */
static const SyntaxEntry syn_clrp1[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_CLRP2),
    SYN_ALT,
    SYN_NT(NT_CLRP2),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <CLRP2> = , <EXP> | , | & # Second param (optional comma and optional expr) */
static const SyntaxEntry syn_clrp2[] = {
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_ALT,
    SYN_TOK(TOK_CCOM),
    SYN_ALT,
    SYN_EPS,
    SYN_END
};

/* <DEFINT> = <EXP> <EOS> | <EOS> # */
/* Accepts letter ranges like A-Z or just DEFINT alone */
static const SyntaxEntry syn_defint[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <DEFLNG> = <EXP> <EOS> | <EOS> # */
static const SyntaxEntry syn_deflng[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <DEFSNG> = <EXP> <EOS> | <EOS> # */
static const SyntaxEntry syn_defsng[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <DEFDBL> = <EXP> <EOS> | <EOS> # */
static const SyntaxEntry syn_defdbl[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <DEFSTR> = <EXP> <EOS> | <EOS> # */
static const SyntaxEntry syn_defstr[] = {
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <CLS> = <EOS> # */
static const SyntaxEntry syn_cls[] = {
    SYN_NT(NT_EOS),
    SYN_END
};

/* File I/O Syntax Rules */

/* <GET> = # <D1> , <TNVAR> <EOS> # */
static const SyntaxEntry syn_get[] = {
    SYN_TOK(TOK_CPND),
    SYN_NT(NT_D1),
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_TNVAR),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <PUT> = # <D1> , <EXP> <EOS> # */
static const SyntaxEntry syn_put[] = {
    SYN_TOK(TOK_CPND),
    SYN_NT(NT_D1),
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* <OPEN> = # <D1> , <EXP> , <EXP> , <STR> <EOS> | <EXP> , <EXP> , <STR> <EOS> # */
/* Supports both: OPEN #ch,mode,dev,"file" and OPEN mode,ch,"file" */
static const SyntaxEntry syn_open[] = {
    SYN_ALT,
    SYN_TOK(TOK_CPND),
    SYN_NT(NT_D1),
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_STR),
    SYN_NT(NT_EOS),
    SYN_ALT,
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_STR),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <CLOSE> = # <D1> <EOS> | <EOS> # */
static const SyntaxEntry syn_close[] = {
    SYN_ALT,
    SYN_TOK(TOK_CPND),
    SYN_NT(NT_D1),
    SYN_NT(NT_EOS),
    SYN_ALT,
    SYN_NT(NT_EOS),
    SYN_END
};

/* <XIO> = <EXP> , # <D1> , <EXP> , <EXP> , <STR> <EOS> # */
static const SyntaxEntry syn_xio[] = {
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    SYN_TOK(TOK_CPND),
    SYN_NT(NT_D1),
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_STR),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <STATUS> = # <D1> , <TNVAR> <EOS> # */
static const SyntaxEntry syn_status[] = {
    SYN_TOK(TOK_CPND),
    SYN_NT(NT_D1),
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_TNVAR),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <NOTE> = # <D1> , <TNVAR> , <TNVAR> <EOS> # */
static const SyntaxEntry syn_note[] = {
    SYN_TOK(TOK_CPND),
    SYN_NT(NT_D1),
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_TNVAR),
    SYN_TOK(TOK_CCOM),
    SYN_NT(NT_TNVAR),
    SYN_NT(NT_EOS),
    SYN_END
};

/* <POINT> = # <D1> , <EXP> , <EXP> <EOS> # */
static const SyntaxEntry syn_point[] = {
    SYN_TOK(TOK_CPND),
    SYN_NT(NT_D1),
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_TOK(TOK_CCOM),
    {SYN_VEXP, {0, 0, 0}},
    SYN_NT(NT_EOS),
    SYN_END
};

/* Main syntax table - maps non-terminals to their rule arrays */
static const SyntaxEntry* syntax_rule_table[NT_MAX_NONTERMINALS];

/* Statement dispatch table - maps statement tokens to syntax rules */
const StatementEntry statement_table[] = {
    {TOK_REM, NT_REM},
    {TOK_LET, NT_LET},
    {TOK_PRINT, NT_PRINT},
    {TOK_QUESTION, NT_PRINT},  /* ? is alias for PRINT */
    {TOK_CGTO, NT_GOTO_STMT},
    {TOK_CGS, NT_GOSUB_STMT},
    {TOK_END, NT_END_STMT},
    {TOK_STOP, NT_STOP_STMT},
    {TOK_RETURN, NT_RETURN_STMT},
    {TOK_FOR, NT_FOR},
    {TOK_NEXT, NT_NEXT},
    {TOK_IF, NT_IF},
    {TOK_INPUT, NT_INPUT},
    {TOK_READ, NT_READ},
    {TOK_DATA, NT_SDATA},
    {TOK_RESTORE, NT_RESTORE},
    {TOK_DIM, NT_DIM},
    {TOK_POKE, NT_POKE_STMT},
    {TOK_GRAPHICS, NT_GRAPHICS_STMT},
    {TOK_PLOT, NT_PLOT_STMT},
    {TOK_POSITION, NT_POSITION_STMT},
    {TOK_DRAWTO, NT_DRAWTO_STMT},
    {TOK_SETCOLOR, NT_SETCOLOR_STMT},
    {TOK_SOUND, NT_SOUND},
    {TOK_ON, NT_ON},
    {TOK_CLR, NT_CLR_STMT},
    {TOK_DEG, NT_DEG_STMT},
    {TOK_RAD, NT_RAD_STMT},
    {TOK_RANDOMIZE, NT_RANDOMIZE_STMT},
    {TOK_POP, NT_POP_STMT},
    {TOK_TRAP, NT_TRAP_STMT},
    {TOK_CONT, NT_CONT_STMT},
    {TOK_BYE, NT_BYE_STMT},
    {TOK_RUN, NT_RUN_STMT},
    {TOK_LIST, NT_LIST_STMT},
    {TOK_SAVE, NT_SAVE_STMT},
    {TOK_CLEAR, NT_CLEAR_STMT},
    {TOK_DEFINT, NT_DEFINT_STMT},
    {TOK_DEFLNG, NT_DEFLNG_STMT},
    {TOK_DEFSNG, NT_DEFSNG_STMT},
    {TOK_DEFDBL, NT_DEFDBL_STMT},
    {TOK_DEFSTR, NT_DEFSTR_STMT},
    {TOK_CLS, NT_CLS_STMT},
    {TOK_GET, NT_GET},
    {TOK_PUT, NT_PUT},
    {TOK_OPEN, NT_OPEN},
    {TOK_CLOSE, NT_CLOSE},
    {TOK_XIO, NT_XIO},
    {TOK_STATUS, NT_STATUS},
    {TOK_NOTE, NT_NOTE_STMT},
    {TOK_POINT, NT_POINT_STMT},
    {TOK_IDENT, NT_LET},  /* Implied LET */
    {0, 0}  /* Sentinel */
};

const int statement_table_size = sizeof(statement_table) / sizeof(StatementEntry) - 1;

/* Initialize syntax table pointers */
void init_syntax_tables(void) {
    int i;
    
    /* Clear table */
    for (i = 0; i < NT_MAX_NONTERMINALS; i++) {
        syntax_rule_table[i] = NULL;
    }
    
    /* Map non-terminals to their syntax rules */
    syntax_rule_table[NT_EXP] = syn_exp;
    syntax_rule_table[NT_UNARY] = syn_unary;
    syntax_rule_table[NT_NV] = syn_nv;
    syntax_rule_table[NT_NOP] = syn_nop;
    syntax_rule_table[NT_OP] = syn_op;
    syntax_rule_table[NT_NVAR] = syn_nvar;
    syntax_rule_table[NT_NMAT] = syn_nmat;
    syntax_rule_table[NT_NMAT2] = syn_nmat2;
    syntax_rule_table[NT_NFUN] = syn_nfun;
    syntax_rule_table[NT_STCOMP] = syn_stcomp;
    syntax_rule_table[NT_STR] = syn_str;
    syntax_rule_table[NT_SVAR] = syn_svar;
    syntax_rule_table[NT_SMAT] = syn_smat;
    syntax_rule_table[NT_SMAT2] = syn_smat2;
    syntax_rule_table[NT_TNVAR] = syn_tnvar;
    syntax_rule_table[NT_NCON] = syn_ncon;
    syntax_rule_table[NT_SCON] = syn_scon;
    syntax_rule_table[NT_EOS] = syn_eos;
    syntax_rule_table[NT_TSVAR] = syn_tsvar;
    syntax_rule_table[NT_SFUN] = syn_sfun;
    syntax_rule_table[NT_SOP] = syn_sop;
    syntax_rule_table[NT_NFNP] = syn_nfnp;
    syntax_rule_table[NT_NFP] = syn_nfp;
    syntax_rule_table[NT_NFSP] = syn_nfsp;
    syntax_rule_table[NT_SFP] = syn_sfp;
    syntax_rule_table[NT_SFNP] = syn_sfnp;
    syntax_rule_table[NT_SF2P] = syn_sf2p;
    syntax_rule_table[NT_SFMID] = syn_sfmid;
    syntax_rule_table[NT_SF3P] = syn_sf3p;
    syntax_rule_table[NT_NFUSR] = syn_nfusr;
    syntax_rule_table[NT_TNCON] = syn_tncon;
    
    /* Statement rules */
    syntax_rule_table[NT_REM] = syn_rem;
    syntax_rule_table[NT_REM_BODY] = syn_rem_body;
    syntax_rule_table[NT_LET] = syn_let;
    syntax_rule_table[NT_PRINT] = syn_print;
    syntax_rule_table[NT_PR1] = syn_pr1;
    syntax_rule_table[NT_PR2] = syn_pr2;
    syntax_rule_table[NT_PEL] = syn_pel;
    syntax_rule_table[NT_PES] = syn_pes;
    syntax_rule_table[NT_PELA] = syn_pela;
    syntax_rule_table[NT_PSL] = syn_psl;
    syntax_rule_table[NT_PSLA] = syn_psla;
    syntax_rule_table[NT_PS] = syn_ps;
    syntax_rule_table[NT_GOTO_STMT] = syn_goto;
    syntax_rule_table[NT_GOSUB_STMT] = syn_gosub;
    syntax_rule_table[NT_END_STMT] = syn_end;
    syntax_rule_table[NT_STOP_STMT] = syn_stop;
    syntax_rule_table[NT_RETURN_STMT] = syn_return;
    syntax_rule_table[NT_FOR] = syn_for;
    syntax_rule_table[NT_FSTEP] = syn_fstep;
    syntax_rule_table[NT_NEXT] = syn_next;
    syntax_rule_table[NT_NEXTVL] = syn_nextvl;
    syntax_rule_table[NT_IF] = syn_if;
    syntax_rule_table[NT_IFA] = syn_ifa;
    syntax_rule_table[NT_IFELSE] = syn_ifelse;
    syntax_rule_table[NT_IFBODY] = syn_ifbody;
    syntax_rule_table[NT_POKE_STMT] = syn_poke;
    syntax_rule_table[NT_GRAPHICS_STMT] = syn_graphics;
    syntax_rule_table[NT_PLOT_STMT] = syn_plot;
    syntax_rule_table[NT_POSITION_STMT] = syn_position;
    syntax_rule_table[NT_DRAWTO_STMT] = syn_drawto;
    syntax_rule_table[NT_SETCOLOR_STMT] = syn_setcolor;
    syntax_rule_table[NT_SOUND] = syn_sound;
    syntax_rule_table[NT_INPUT] = syn_input;
    syntax_rule_table[NT_READ] = syn_read;
    syntax_rule_table[NT_PROMPT] = syn_prompt;
    syntax_rule_table[NT_NSVAR] = syn_nsvar;
    syntax_rule_table[NT_NSVRL] = syn_nsvrl;
    syntax_rule_table[NT_NSV2] = syn_nsv2;
    syntax_rule_table[NT_OPD] = syn_opd;
    syntax_rule_table[NT_D1] = syn_d1;
    syntax_rule_table[NT_SDATA] = syn_data;
    syntax_rule_table[NT_DATA_LIST] = syn_data_list;
    syntax_rule_table[NT_DATA_TAIL] = syn_data_tail;
    syntax_rule_table[NT_DATA_VAL] = syn_data_val;
    syntax_rule_table[NT_RESTORE] = syn_restore;
    syntax_rule_table[NT_DIM] = syn_dim;
    syntax_rule_table[NT_NSMAT] = syn_nsmat;
    syntax_rule_table[NT_NSML] = syn_nsml;
    syntax_rule_table[NT_NSML2] = syn_nsml2;
    syntax_rule_table[NT_ON] = syn_on;
    syntax_rule_table[NT_ON1] = syn_on1;
    syntax_rule_table[NT_EXPL] = syn_expl;
    syntax_rule_table[NT_EXPL1] = syn_expl1;
    syntax_rule_table[NT_CLR_STMT] = syn_clr;
    syntax_rule_table[NT_DEG_STMT] = syn_deg;
    syntax_rule_table[NT_RAD_STMT] = syn_rad;
    syntax_rule_table[NT_RANDOMIZE_STMT] = syn_randomize;
    syntax_rule_table[NT_POP_STMT] = syn_pop;
    syntax_rule_table[NT_TRAP_STMT] = syn_trap;
    syntax_rule_table[NT_CONT_STMT] = syn_cont;
    syntax_rule_table[NT_BYE_STMT] = syn_bye;
    syntax_rule_table[NT_RUN_STMT] = syn_run;
    syntax_rule_table[NT_LIST_STMT] = syn_list;
    syntax_rule_table[NT_SAVE_STMT] = syn_save;
    syntax_rule_table[NT_CLEAR_STMT] = syn_clear;
    syntax_rule_table[NT_CLRP1] = syn_clrp1;
    syntax_rule_table[NT_CLRP2] = syn_clrp2;
    syntax_rule_table[NT_DEFINT_STMT] = syn_defint;
    syntax_rule_table[NT_DEFLNG_STMT] = syn_deflng;
    syntax_rule_table[NT_DEFSNG_STMT] = syn_defsng;
    syntax_rule_table[NT_DEFDBL_STMT] = syn_defdbl;
    syntax_rule_table[NT_DEFSTR_STMT] = syn_defstr;
    syntax_rule_table[NT_CLS_STMT] = syn_cls;
    syntax_rule_table[NT_GET] = syn_get;
    syntax_rule_table[NT_PUT] = syn_put;
    syntax_rule_table[NT_OPEN] = syn_open;
    syntax_rule_table[NT_CLOSE] = syn_close;
    syntax_rule_table[NT_XIO] = syn_xio;
    syntax_rule_table[NT_STATUS] = syn_status;
    syntax_rule_table[NT_NOTE_STMT] = syn_note;
    syntax_rule_table[NT_POINT_STMT] = syn_point;
}

/* Get syntax rule for a non-terminal */
const SyntaxEntry* get_syntax_rule(NonTerminal nt) {
    if (nt >= 0 && nt < NT_MAX_NONTERMINALS) {
        return syntax_rule_table[nt];
    }
    return NULL;
}

/* Get statement syntax rule from token */
NonTerminal get_statement_rule(unsigned char token) {
    int i;
    
    for (i = 0; i < statement_table_size; i++) {
        if (statement_table[i].token == token) {
            return statement_table[i].syntax_rule;
        }
    }
    
    return (NonTerminal)-1;  /* Not found */
}

/* Get function metadata from token */
const FunctionEntry* get_function_metadata(unsigned char token) {
    int i;
    
    for (i = 0; i < function_table_size; i++) {
        if (function_table[i].token == token) {
            return &function_table[i];
        }
    }
    
    return NULL;  /* Not found */
}

/* Get operator entry from token */
const OperatorEntry* get_operator_entry(unsigned char token) {
    int i;
    
    for (i = 0; i < operator_table_size; i++) {
        if (operator_table[i].token == token) {
            return &operator_table[i];
        }
    }
    
    return NULL;  /* Not found */
}

/* Check if token is terminal */
int is_terminal(unsigned char token) {
    return (token & TC_TERMINAL) != 0;
}

/* Match token against pattern */
int match_token_class(unsigned char token, unsigned char pattern) {
    /* Terminal match - exact comparison */
    if (pattern & TC_TERMINAL) {
        return token == (pattern & ~TC_TERMINAL);
    }
    /* Non-terminal - handled by parser */
    return 0;
}
