/* parser.h - Table-Driven Classic BASIC Parser */
#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"
#include "syntax_tables.h"

/* Parse tree node types */
typedef enum {
    NODE_STATEMENT,
    NODE_EXPRESSION,
    NODE_VARIABLE,
    NODE_CONSTANT,
    NODE_OPERATOR,
    NODE_FUNCTION_CALL,
    NODE_ASSIGNMENT
} NodeType;

/* Parse tree node */
typedef struct ParseNode {
    NodeType type;
    unsigned char token;         /* Token type for this node */
    char *text;                  /* Text (for identifiers) */
    double value;                /* Value (for constants) */
    int line_number;             /* Line number (if this is a statement) */
    struct ParseNode **children; /* Child nodes */
    int child_count;             /* Number of children */
    int child_capacity;          /* Allocated capacity */
} ParseNode;

/* Parser state */
typedef struct {
    Tokenizer *tokenizer;
    ParseNode *root;             /* Root of parse tree */
    int error;                   /* Error flag */
    char error_msg[256];         /* Error message */
    int error_column;            /* Column where error was detected */
    const char *error_line_start; /* Start of line where error occurred */
    const char *error_line_end;   /* End of line where error occurred */
    int error_line_number;       /* Line number where error occurred */
    int current_line_number;     /* Current line number being parsed */
    int previous_line_number;    /* Previous line number (for sequential check) */
    int error_count;             /* Total number of errors encountered */
    const char *current_basic_line_start; /* Start of current BASIC statement line (for error reporting) */
    
    /* Line number tracking for GOTO/GOSUB validation */
    int *line_numbers;           /* Array of all line numbers seen */
    int line_numbers_count;      /* Number of line numbers */
    int line_numbers_capacity;   /* Capacity of line_numbers array */
    
    /* Syntax parsing state */
    const SyntaxEntry *current_rule;
    int rule_index;
    
    /* Expression parsing state */
    unsigned char op_stack[64];  /* Operator stack */
    int op_stack_top;
    ParseNode *val_stack[64];    /* Value stack */
    int val_stack_top;
} Parser;

/* Parser functions */
Parser* parser_create(Tokenizer *tok);
void parser_free(Parser *parser);
ParseNode* parser_parse_program(Parser *parser);
ParseNode* parser_parse_statement(Parser *parser);
ParseNode* parser_parse_expression(Parser *parser);

/* Parse tree functions */
ParseNode* node_create(NodeType type);
void node_free(ParseNode *node);
void node_add_child(ParseNode *parent, ParseNode *child);
void node_print(ParseNode *node, int indent);

#endif /* PARSER_H */
