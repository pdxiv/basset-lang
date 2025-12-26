/* tokens.h - Classic BASIC Token Definitions */
#ifndef TOKENS_H
#define TOKENS_H

/* Token values compatible with classic BASIC implementations */

/* General Operators & Keywords ($10-$2C) */
#define TOK_CDQ     0x10  /* " - String delimiter */
#define TOK_CSOE    0x11  /* Start Of Expression (non-printing) */
/* Basic operators - in 0x50+ range to avoid keyword collisions but stay below 0x80 (TC_TERMINAL flag) */
#define TOK_CCOM    0x50  /* , - Comma */
#define TOK_CDOL    0x51  /* $ - Dollar sign (string sigil) */
#define TOK_CEOS    0x52  /* : - End-of-statement colon */
#define TOK_CSC     0x53  /* ; - Semicolon */
#define TOK_CCR     0x54  /* CR - Carriage return / end-of-line */
#define TOK_CGTO    0x55  /* GOTO */
#define TOK_CGS     0x56  /* GOSUB */
#define TOK_CTO     0x57  /* TO */
#define TOK_CSTEP   0x58  /* STEP */
#define TOK_CTHEN   0x59  /* THEN */
#define TOK_CPND    0x5A  /* # - Pound/hash (I/O channel) */
#define TOK_CLE     0x5B  /* <= Less-or-equal */
#define TOK_CNE     0x5C  /* <> Not equal */
#define TOK_CGE     0x5D  /* >= Greater-or-equal */
#define TOK_CLT     0x5E  /* < Less than */
#define TOK_CGT     0x5F  /* > Greater than */
#define TOK_CEQ     0x60  /* = Equal */
#define TOK_CEXP    0x61  /* ^ - Exponentiation */
#define TOK_CMUL    0x62  /* * - Multiplication */
#define TOK_CPLUS   0x63  /* + - Addition */
#define TOK_CMINUS  0x64  /* - - Subtraction */
#define TOK_CDIV    0x65  /* / - Division */
#define TOK_CNOT    0x66  /* NOT */
#define TOK_COR     0x67  /* OR */
#define TOK_CAND    0x68  /* AND */
#define TOK_CLPRN   0x69  /* ( - Left parenthesis */
#define TOK_CRPRN   0x6A  /* ) - Right parenthesis */

/* Special Context Operators ($2D-$3C) */
#define TOK_CAASN   0x2D  /* = - Arithmetic assignment */
#define TOK_CSASN   0x2E  /* = - String assignment */
#define TOK_CSLE    0x2F  /* <= String compare */
#define TOK_CSNE    0x30  /* <> String compare */
#define TOK_CSGE    0x31  /* >= String compare */
#define TOK_CSLT    0x32  /* < String compare */
#define TOK_CSGT    0x33  /* > String compare */
#define TOK_CSEQ    0x34  /* = String compare */
#define TOK_CUPLUS  0x35  /* + Unary plus */
#define TOK_CUMINUS 0x36  /* - Unary minus */
#define TOK_CSLPRN  0x37  /* ( String/substring */
#define TOK_CALPRN  0x38  /* ( Array (non-printing $80) */
#define TOK_CDLPRN  0x39  /* ( DIM (non-printing $80) */
#define TOK_CFLPRN  0x3A  /* ( Function */
#define TOK_CDSLPR  0x3B  /* ( DIM string */
#define TOK_CACOM   0x3C  /* , Array comma */

/* Functions ($3D-$54) */
#define TOK_CSTR    0x3D  /* STR$ */
#define TOK_CCHR    0x3E  /* CHR$ */
#define TOK_CUSR    0x3F  /* USR */
#define TOK_CASC    0x40  /* ASC */
#define TOK_CVAL    0x41  /* VAL */
#define TOK_CLEN    0x42  /* LEN */
#define TOK_CADR    0x43  /* ADR */
#define TOK_CATN    0x44  /* ATN */
#define TOK_CCOS    0x45  /* COS */
#define TOK_CPEEK   0x46  /* PEEK */
#define TOK_CSIN    0x47  /* SIN */
#define TOK_CRND    0x48  /* RND */
#define TOK_CFRE    0x49  /* FRE */
#define TOK_CEXP_F  0x4A  /* EXP (function) */
#define TOK_CLOG    0x4B  /* LOG */
#define TOK_CCLOG   0x4C  /* CLOG */
#define TOK_CSQR    0x4D  /* SQR */
#define TOK_CSGN    0x4E  /* SGN */
#define TOK_CABS    0x4F  /* ABS */
#define TOK_CINT    0x70  /* INT - moved to avoid collision with TOK_CCOM */
#define TOK_CPADD   0x71  /* PADDLE */
#define TOK_CSTIK   0x72  /* STICK */
#define TOK_CPTRG   0x73  /* PTRIG */
#define TOK_CSTRG   0x74  /* STRIG */
#define TOK_CLEFT   0x75  /* LEFT$ */
#define TOK_CRIGHT  0x76  /* RIGHT$ */
#define TOK_CMID    0x77  /* MID$ */
#define TOK_CTAB    0x79  /* TAB - moved to avoid collision with TOK_ELSE (0x78) */

/* Statements (0x55-) */
#define TOK_REM     0x00  /* REM - special handling */
#define TOK_DATA    0x01  /* DATA - special handling */
#define TOK_INPUT   0x02  /* INPUT */
#define TOK_COLOR   0x03  /* COLOR */
#define TOK_LIST    0x04  /* LIST */
#define TOK_ENTER   0x05  /* ENTER */
#define TOK_LET     0x06  /* LET */
#define TOK_IF      0x07  /* IF */
#define TOK_FOR     0x08  /* FOR */
#define TOK_NEXT    0x09  /* NEXT */
#define TOK_GOTO    0x0A  /* GOTO (statement form) */
#define TOK_GO_TO   0x0B  /* GO TO */
#define TOK_GOSUB_S 0x0C  /* GOSUB (statement form) */
#define TOK_TRAP    0x0D  /* TRAP */
#define TOK_BYE     0x0E  /* BYE */
#define TOK_CONT    0x0F  /* CONT */
#define TOK_COM     0x10  /* COM */
#define TOK_CLOSE   0x11  /* CLOSE */
#define TOK_CLR     0x12  /* CLR */
#define TOK_DEG     0x13  /* DEG */
#define TOK_DIM     0x14  /* DIM */
#define TOK_END     0x15  /* END */
#define TOK_NEW     0x16  /* NEW */
#define TOK_OPEN    0x17  /* OPEN */
#define TOK_LOAD    0x18  /* LOAD */
#define TOK_SAVE    0x19  /* SAVE */
#define TOK_STATUS  0x1A  /* STATUS */
#define TOK_NOTE    0x1B  /* NOTE */
#define TOK_POINT   0x1C  /* POINT */
#define TOK_XIO     0x1D  /* XIO */
#define TOK_ON      0x1E  /* ON */
#define TOK_POKE    0x1F  /* POKE */
#define TOK_PRINT   0x20  /* PRINT */
#define TOK_RAD     0x21  /* RAD */
#define TOK_READ    0x22  /* READ */
#define TOK_RESTORE 0x23  /* RESTORE */
#define TOK_RETURN  0x24  /* RETURN */
#define TOK_RUN     0x25  /* RUN */
#define TOK_STOP    0x26  /* STOP */
#define TOK_POP     0x27  /* POP */
#define TOK_QUESTION 0x28 /* ? (alias for PRINT) */
#define TOK_GET     0x29  /* GET */
#define TOK_PUT     0x2A  /* PUT */
#define TOK_GRAPHICS 0x2B /* GRAPHICS */
#define TOK_PLOT    0x2C  /* PLOT */
#define TOK_POSITION 0x2D /* POSITION */
#define TOK_DOS     0x2E  /* DOS */
#define TOK_DRAWTO  0x2F  /* DRAWTO */
#define TOK_SETCOLOR 0x30 /* SETCOLOR */
#define TOK_LOCATE  0x31  /* LOCATE */
#define TOK_SOUND   0x32  /* SOUND */
#define TOK_LPRINT  0x33  /* LPRINT */
#define TOK_CSAVE   0x34  /* CSAVE */
#define TOK_CLOAD   0x35  /* CLOAD */
#define TOK_RANDOMIZE 0x36 /* RANDOMIZE */
#define TOK_CLEAR   0x37  /* CLEAR (Microsoft BASIC) */
#define TOK_DEFINT  0x38  /* DEFINT (Microsoft BASIC) */
#define TOK_DEFLNG  0x39  /* DEFLNG (Microsoft BASIC) */
#define TOK_DEFSNG  0x3A  /* DEFSNG (Microsoft BASIC) */
#define TOK_DEFDBL  0x3B  /* DEFDBL (Microsoft BASIC) */
#define TOK_DEFSTR  0x3C  /* DEFSTR (Microsoft BASIC) */
#define TOK_CLS     0x3D  /* CLS (Clear Screen) */
#define TOK_ELSE    0x78  /* ELSE - moved to avoid collision with TOK_CCHR */

/* Special token types - must be < 0x80 to work with TC_TERMINAL flag */
#define TOK_NUMBER  0x6B  /* Numeric constant */
#define TOK_STRING  0x6C  /* String constant */
#define TOK_IDENT   0x6D  /* Variable identifier */
#define TOK_EOF     0x7F  /* End of file/input */

#endif /* TOKENS_H */
