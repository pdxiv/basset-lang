/* bytecode.h - Bytecode instruction definitions */
#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>

/* Bytecode instruction format: 4 bytes fixed-width */
typedef struct {
    uint8_t opcode;      /* Operation code (0-255) */
    uint8_t flags;       /* Type hints for JIT, future use */
    uint16_t operand;    /* Immediate value, offset, or slot number */
} Instruction;

/* Stack Operations */
#define OP_PUSH_CONST   0x00
#define OP_PUSH_VAR     0x01
#define OP_POP_VAR      0x02
#define OP_DUP          0x03
#define OP_POP          0x04
#define OP_STR_POP_VAR  0x05  /* Pop string from stack to variable */
#define OP_STR_PUSH_VAR 0x06  /* Push string variable to stack */

/* Arithmetic */
#define OP_ADD          0x10
#define OP_SUB          0x11
#define OP_MUL          0x12
#define OP_DIV          0x13
#define OP_MOD          0x14
#define OP_POW          0x15
#define OP_NEG          0x16

/* Comparison */
#define OP_EQ           0x20
#define OP_NE           0x21
#define OP_LT           0x22
#define OP_LE           0x23
#define OP_GT           0x24
#define OP_GE           0x25

/* Logical (AND/OR) */
#define OP_AND          0x26
#define OP_OR           0x27
#define OP_NOT          0x28

/* String Operations */
#define OP_STR_PUSH     0x30
#define OP_STR_CONCAT   0x31
#define OP_STR_LEN      0x32
#define OP_STR_VAL      0x33
#define OP_STR_CHR      0x34
#define OP_STR_STR      0x35
#define OP_STR_ASC      0x36
#define OP_STR_LEFT     0x37
#define OP_STR_RIGHT    0x38
#define OP_STR_MID      0x39
#define OP_STR_MID_2    0x3A  /* MID$ with 2 args (no length) */

/* Array Operations */
#define OP_ARRAY_GET_1D 0x40
#define OP_ARRAY_SET_1D 0x41
#define OP_ARRAY_GET_2D 0x42
#define OP_ARRAY_SET_2D 0x43
#define OP_DIM_1D       0x44
#define OP_DIM_2D       0x45
#define OP_STR_ARRAY_GET_1D 0x46
#define OP_STR_ARRAY_SET_1D 0x47
#define OP_STR_ARRAY_GET_2D 0x48
#define OP_STR_ARRAY_SET_2D 0x49

/* Control Flow */
#define OP_JUMP         0x50
#define OP_JUMP_IF_FALSE 0x51
#define OP_JUMP_IF_TRUE 0x52
#define OP_JUMP_LINE    0x53
#define OP_GOSUB        0x54
#define OP_GOSUB_LINE   0x55
#define OP_RETURN       0x56
#define OP_ON_GOTO      0x57
#define OP_ON_GOSUB     0x58
#define OP_FOR_INIT     0x59
#define OP_FOR_NEXT     0x5A

/* I/O Operations */
#define OP_PRINT_NUM    0x60
#define OP_PRINT_STR    0x61
#define OP_PRINT_NEWLINE 0x62
#define OP_PRINT_SPACE  0x63
#define OP_PRINT_TAB    0x64
#define OP_TAB_FUNC     0x65  /* TAB(n) function - move to column */
#define OP_PRINT_NOSEP  0x66  /* Semicolon - suppress spacing (moved from 0x65) */
#define OP_INPUT_NUM    0x67  /* Shifted from 0x66 */
#define OP_INPUT_STR    0x68  /* Shifted from 0x67 */
#define OP_INPUT_PROMPT 0x69  /* Shifted from 0x68 */
#define OP_OPEN         0x6A  /* Shifted from 0x69 */
#define OP_CLOSE        0x6B  /* Shifted from 0x6A */
#define OP_GET          0x6C  /* Shifted from 0x6B */
#define OP_PUT          0x6D  /* Shifted from 0x6C */
#define OP_NOTE         0x6E  /* Shifted from 0x6D */
#define OP_POINT        0x6F  /* Shifted from 0x6E */
#define OP_STATUS       0x70  /* Shifted from 0x6F */
#define OP_XIO          0x71  /* Shifted from 0x70 */
#define OP_DATA_READ_NUM 0x72  /* Shifted from 0x71 */
#define OP_DATA_READ_STR 0x73  /* Shifted from 0x72 */
#define OP_SET_PRINT_CHANNEL 0x74  /* Shifted from 0x73 */

/* Math Functions (shifted down by 1 to avoid conflict) */
#define OP_FUNC_SIN     0x75  /* Shifted from 0x74 */
#define OP_FUNC_COS     0x76  /* Shifted from 0x75 */
#define OP_FUNC_TAN     0x77  /* Shifted from 0x76 */
#define OP_FUNC_ATN     0x78  /* Shifted from 0x77 */
#define OP_FUNC_EXP     0x79  /* Shifted from 0x78 */
#define OP_FUNC_LOG     0x7A  /* Shifted from 0x79 */
#define OP_FUNC_CLOG    0x7B  /* Shifted from 0x7A - Common logarithm (log10) */
#define OP_FUNC_SQR     0x7C  /* Shifted from 0x7B */
#define OP_FUNC_ABS     0x7D  /* Shifted from 0x7C */
#define OP_FUNC_INT     0x7E  /* Shifted from 0x7D */
#define OP_FUNC_RND     0x7F  /* Shifted from 0x7E */
#define OP_FUNC_SGN     0x80  /* Shifted from 0x7F */

/* System */
#define OP_TRAP         0x81
#define OP_TRAP_DISABLE 0x82
#define OP_END          0x83
#define OP_STOP         0x84
#define OP_RESTORE      0x85
#define OP_RESTORE_LINE 0x86
#define OP_DEG          0x87  /* Switch to degree mode */
#define OP_RAD          0x88  /* Switch to radian mode */
#define OP_RANDOMIZE    0x89  /* Seed RNG with system time */
#define OP_CLR          0x8A  /* Clear all variables */
#define OP_POP_GOSUB    0x8B  /* Pop GOSUB stack without RETURN */
#define OP_NOP          0x8C
#define OP_HALT         0x8D
#define OP_FUNC_PEEK    0x8E  /* PEEK memory function */
#define OP_POKE         0x8F  /* POKE memory statement */

#endif /* BYTECODE_H */
