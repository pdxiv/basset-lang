/* syntax_tables.h - Table-Driven BASIC Syntax Table Definitions */
#ifndef SYNTAX_TABLES_H
#define SYNTAX_TABLES_H

#include "tokens.h"

/* Syntax Table Opcodes (BML Meta-Language Encoding) */
#define SYN_ANTV    0x00  /* Absolute Non-Terminal Vector (+ 2-byte addr-1) */
#define SYN_ESRT    0x01  /* External Subroutine Call (+ 2-byte addr-1) */
#define SYN_OR      0x02  /* Alternative (BNF |) */
#define SYN_RTN     0x03  /* Return (#) */
#define SYN_NULL    0x04  /* Null/Accept (&) */
#define SYN_VEXP    0x0E  /* Special vector for <EXP> */
#define SYN_CHNG    0x0F  /* Change last output token */

/* Token class flags for syntax matching */
#define TC_TERMINAL   0x80  /* High bit = terminal token */
#define TC_NONTERMINAL 0x00 /* Non-terminal reference */

/* Non-terminal symbols (grammar rules) */
typedef enum {
    NT_STATEMENT = 0, /* <STATEMENT> - Generic statement */
    NT_EXP,          /* <EXP> - Expression */
    NT_UNARY,        /* <UNARY> - Unary operator */
    NT_NV,           /* <NV> - Numeric value */
    NT_NOP,          /* <NOP> - Numeric operator */
    NT_OP,           /* <OP> - Operator */
    NT_NVAR,         /* <NVAR> - Numeric variable */
    NT_NMAT,         /* <NMAT> - Numeric matrix/array */
    NT_NMAT2,        /* <NMAT2> - Numeric matrix continuation */
    NT_NFUN,         /* <NFUN> - Numeric function */
    NT_NFUSR,        /* <NFUSR> - USR function */
    NT_NFP,          /* <NFP> - Numeric function parameters */
    NT_SFP,          /* <SFP> - String function parameters */
    NT_STCOMP,       /* <STCOMP> - String comparison */
    NT_STR,          /* <STR> - String expression */
    NT_SFUN,         /* <SFUN> - String function */
    NT_SVAR,         /* <SVAR> - String variable */
    NT_SMAT,         /* <SMAT> - String matrix/array */
    NT_SMAT2,        /* <SMAT2> - String matrix continuation */
    NT_SOP,          /* <SOP> - String operator */
    NT_PUT,          /* <PUT> statement */
    NT_LET,          /* <LET> statement */
    NT_FOR,          /* <FOR> statement */
    NT_FSTEP,        /* <FSTEP> - FOR step clause */
    NT_LOCATE,       /* <LOCATE> statement */
    NT_GET,          /* <GET> statement */
    NT_NEXT,         /* <NEXT> statement */
    NT_NEXTVL,       /* <NEXTVL> - NEXT variable list (comma-separated) */
    NT_RESTORE,      /* <RESTORE> statement */
    NT_INPUT,        /* <INPUT> statement */
    NT_READ,         /* <READ> statement */
    NT_PROMPT,       /* <PROMPT> - Optional INPUT prompt */
    NT_EOS,          /* End of statement */
    NT_PRINT,        /* <PRINT> statement */
    NT_D1,           /* Device number */
    NT_NSVAR,        /* Numeric or string variable */
    NT_NSVRL,        /* Numeric/string variable list */
    NT_NSV2,         /* Variable list continuation */
    NT_XIO,          /* <XIO> statement */
    NT_OPEN,         /* <OPEN> statement */
    NT_CLOSE,        /* <CLOSE> statement */
    NT_RUN,          /* <RUN> statement */
    NT_OPD,          /* Optional device */
    NT_LIST,         /* <LIST> statement */
    NT_STATUS,       /* <STATUS> statement */
    NT_STAT,         /* Status clause */
    NT_FS,           /* File specification */
    NT_TEXP,         /* Two expressions */
    NT_SOUND,        /* <SOUND> statement */
    NT_DIM,          /* <DIM> statement */
    NT_ON,           /* <ON> statement */
    NT_ON1,          /* ON GOTO/GOSUB */
    NT_EXPL,         /* Expression list */
    NT_EXPL1,        /* Expression list continuation */
    NT_EOS2,         /* End of statement (alt) */
    NT_NSMAT,        /* Numeric/string matrix */
    NT_NSML,         /* Numeric/string matrix list */
    NT_NSML2,        /* Matrix list continuation */
    NT_IF,           /* <IF> statement */
    NT_IFA,          /* IF action */
    NT_IFELSE,       /* IF ELSE clause */
    NT_IFBODY,       /* IF body (stops at ELSE) */
    NT_PR1,          /* Print list */
    NT_PR2,          /* Print list continuation */
    NT_PEL,          /* Print element */
    NT_PES,          /* Print element start */
    NT_PELA,         /* Print element list */
    NT_PSL,          /* Print separator list */
    NT_PSLA,         /* Print separator list alt */
    NT_PS,           /* Print separator */
    NT_L1,           /* List start */
    NT_L2,           /* List continuation */
    NT_REM,          /* <REM> statement */
    NT_SDATA,        /* <DATA> statement */
    NT_NFSP,         /* Numeric function (string param) */
    NT_SFNP,         /* String function (numeric param) */
    NT_PUSR,         /* USR parameter list */
    NT_PUSR1,        /* USR parameter continuation */
    NT_NCON,         /* Numeric constant */
    NT_SCON,         /* String constant */
    NT_TNVAR,        /* Token numeric variable */
    NT_TSVAR,        /* Token string variable */
    NT_TNCON,        /* Token numeric constant */
    NT_NFNP,         /* Numeric function (numeric param) */
    NT_EIF,          /* Execute IF */
    NT_OPD2,         /* Optional device 2 */
    NT_D2S,          /* Device 2 string */
    NT_CPND2,        /* Compound device 2 */
    NT_AEXP,         /* Auxiliary expression */
    NT_L1S,          /* List 1 string */
    NT_GOTO_STMT,    /* GOTO statement */
    NT_GOSUB_STMT,   /* GOSUB statement */
    NT_END_STMT,     /* END statement */
    NT_STOP_STMT,    /* STOP statement */
    NT_RETURN_STMT,  /* RETURN statement */
    NT_POKE_STMT,    /* POKE statement */
    NT_GRAPHICS_STMT,/* GRAPHICS statement */
    NT_PLOT_STMT,    /* PLOT statement */
    NT_POSITION_STMT,/* POSITION statement */
    NT_DRAWTO_STMT,  /* DRAWTO statement */
    NT_SETCOLOR_STMT,/* SETCOLOR statement */
    NT_CLR_STMT,     /* CLR statement */
    NT_DEG_STMT,     /* DEG statement */
    NT_RAD_STMT,     /* RAD statement */
    NT_RANDOMIZE_STMT, /* RANDOMIZE statement */
    NT_POP_STMT,     /* POP statement */
    NT_TRAP_STMT,    /* TRAP statement */
    NT_CONT_STMT,    /* CONT statement */
    NT_NOTE_STMT,    /* NOTE statement */
    NT_POINT_STMT,   /* POINT statement */
    NT_BYE_STMT,     /* BYE statement */
    NT_RUN_STMT,     /* RUN statement */
    NT_LIST_STMT,    /* LIST statement */
    NT_SAVE_STMT,    /* SAVE statement */
    NT_CLEAR_STMT,   /* CLEAR statement */
    NT_CLRP1,        /* CLEAR parameter 1 */
    NT_CLRP2,        /* CLEAR parameter 2 */
    NT_DEFINT_STMT,  /* DEFINT statement */
    NT_DEFLNG_STMT,  /* DEFLNG statement */
    NT_DEFSNG_STMT,  /* DEFSNG statement */
    NT_DEFDBL_STMT,  /* DEFDBL statement */
    NT_DEFSTR_STMT,  /* DEFSTR statement */
    NT_CLS_STMT,     /* CLS statement */
    NT_SF2P,         /* String function 2 params (str, num) */
    NT_SF3P,         /* String function 3 params (str, num, num) */
    NT_SFMID,        /* MID$ function */
    NT_MAX_NONTERMINALS
} NonTerminal;

/* Syntax table entry structure */
typedef struct {
    unsigned char opcode;      /* Syntax opcode */
    unsigned char data[3];     /* Additional data (address/token) */
} SyntaxEntry;

/* Operator precedence table entry */
typedef struct {
    unsigned char token;
    unsigned char go_on_stack;  /* Precedence when pushing */
    unsigned char come_off_stack; /* Precedence when popping */
    void (*executor)(void);     /* Execution function pointer */
} OperatorEntry;

/* Keyword table entry */
typedef struct {
    const char *keyword;
    unsigned char token;
    unsigned char type;         /* Statement/function/operator */
} KeywordEntry;

/* Statement dispatch table entry */
typedef struct {
    unsigned char token;      /* Statement token (e.g., TOK_PRINT) */
    NonTerminal syntax_rule;  /* Corresponding syntax rule */
} StatementEntry;

/* External table declarations */
extern const KeywordEntry keyword_table[];
extern const int keyword_table_size;

extern const SyntaxEntry syntax_table[];
extern const int syntax_table_size;

extern const OperatorEntry operator_table[];
extern const int operator_table_size;

extern const StatementEntry statement_table[];
extern const int statement_table_size;

/* Syntax table lookup functions */
void init_syntax_tables(void);
const SyntaxEntry* get_syntax_rule(NonTerminal nt);
NonTerminal get_statement_rule(unsigned char token);
int is_terminal(unsigned char token);
int match_token_class(unsigned char token, unsigned char pattern);

#endif /* SYNTAX_TABLES_H */
