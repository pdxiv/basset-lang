/* parser.c - Truly Table-Driven Parser Implementation */
#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include "syntax_tables.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* C89 compatible strdup (POSIX strdup not in C89 standard) */
static char* my_strdup(const char *s) {
    char *d;
    if (!s) return NULL;
    d = malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}

/* Forward declarations */
static ParseNode* parse_nonterminal(Parser *p, NonTerminal nt);
static ParseNode* parse_expression_pratt(Parser *p);
static ParseNode* parse_expression_pratt_prec(Parser *p, int min_prec);
static int get_operator_precedence(unsigned char token, int on_stack);

/* Create parse node */
ParseNode* node_create(NodeType type) {
    ParseNode *node = malloc(sizeof(ParseNode));
    node->type = type;
    node->token = 0;
    node->text = NULL;
    node->value = 0.0;
    node->line_number = 0;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    return node;
}

/* Free parse node */
void node_free(ParseNode *node) {
    int i;
    if (!node) return;
    
    if (node->text) free(node->text);
    
    for (i = 0; i < node->child_count; i++) {
        node_free(node->children[i]);
    }
    
    if (node->children) free(node->children);
    free(node);
}

/* Add child to node */
void node_add_child(ParseNode *parent, ParseNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        int new_cap = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
        parent->children = realloc(parent->children, new_cap * sizeof(ParseNode*));
        parent->child_capacity = new_cap;
    }
    parent->children[parent->child_count++] = child;
}

/* Print parse tree (for debugging) */
void node_print(ParseNode *node, int indent) {
    int i;
    if (!node) return;
    
    for (i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_STATEMENT:
            printf("STMT: %s\n", token_name(node->token));
            break;
        case NODE_EXPRESSION:
            printf("EXPR\n");
            break;
        case NODE_VARIABLE:
            printf("VAR: %s\n", node->text ? node->text : "?");
            break;
        case NODE_CONSTANT:
            if (node->token == TOK_STRING) {
                printf("STRING: \"%s\"\n", node->text ? node->text : "");
            } else {
                printf("CONST: %g\n", node->value);
            }
            break;
        case NODE_OPERATOR:
            printf("OP: %s\n", token_name(node->token));
            break;
        case NODE_FUNCTION_CALL:
            printf("FUNC: %s\n", token_name(node->token));
            break;
        case NODE_ASSIGNMENT:
            printf("ASSIGN\n");
            break;
    }
    
    for (i = 0; i < node->child_count; i++) {
        node_print(node->children[i], indent + 1);
    }
}

/* Create parser */
Parser* parser_create(Tokenizer *tok) {
    Parser *p = malloc(sizeof(Parser));
    p->tokenizer = tok;
    p->root = NULL;
    p->error = 0;
    p->error_msg[0] = '\0';
    p->error_column = -1;
    p->error_line_start = NULL;
    p->error_line_end = NULL;
    p->error_line_number = 0;
    p->error_count = 0;
    p->current_line_number = 0;
    p->previous_line_number = 0;
    p->current_basic_line_start = NULL;
    p->line_numbers = NULL;
    p->line_numbers_count = 0;
    p->line_numbers_capacity = 0;
    p->current_rule = NULL;
    p->rule_index = 0;
    p->op_stack_top = 0;
    p->val_stack_top = 0;
    return p;
}

/* Free parser */
void parser_free(Parser *parser) {
    if (parser->root) {
        node_free(parser->root);
    }
    if (parser->line_numbers) {
        free(parser->line_numbers);
    }
    free(parser);
}

/* Set error message */
static void set_error(Parser *p, const char *msg) {
    Token *tok;
    const char *line_end;
    const char *scan;
    int target_line;
    int line_count;
    
    p->error = 1;
    /* Only capture error info on first error in this statement */
    if (p->error_column < 0) {
        tok = tokenizer_peek(p->tokenizer);
        p->error_column = tok->column;
        p->error_line_number = tok->line;  /* Use token's line number */
        
        /* Use the captured BASIC line start if available */
        if (p->current_basic_line_start) {
            p->error_line_start = p->current_basic_line_start;
        } else {
            /* Fall back to finding the start of the token's line by scanning from the beginning */
            target_line = tok->line;
            line_count = 1;
            p->error_line_start = p->tokenizer->input_start;
            
            /* Scan forward to find the start of the target line */
            scan = p->tokenizer->input_start;
            while (*scan && line_count < target_line) {
                if (*scan == '\n') {
                    line_count++;
                    if (line_count == target_line) {
                        p->error_line_start = scan + 1;
                        break;
                    }
                }
                scan++;
            }
        }
        
        /* Find end of this line */
        line_end = p->error_line_start;
        while (*line_end && *line_end != '\n' && *line_end != '\r') {
            line_end++;
        }
        p->error_line_end = line_end;
    }
    strncpy(p->error_msg, msg, sizeof(p->error_msg) - 1);
    p->error_msg[sizeof(p->error_msg) - 1] = '\0';
}

/* Display error with source line context */
static void display_error_with_context(Parser *p, const char *msg) {
    const char *line_start;
    const char *line_end;
    int line_num;
    int column;
    int i;
    Token *tok;
    
    /* Use captured error information if available */
    if (p->error_line_start && p->error_line_end) {
        line_start = p->error_line_start;
        line_end = p->error_line_end;
        /* Prefer BASIC line number over physical line number */
        line_num = p->current_line_number > 0 ? p->current_line_number : p->error_line_number;
        column = p->error_column >= 0 ? p->error_column : 0;
    } else {
        /* Use the captured BASIC line start if available, otherwise fall back to tokenizer */
        if (p->current_basic_line_start) {
            line_start = p->current_basic_line_start;
        } else {
            line_start = p->tokenizer->line_start;
        }
        line_end = line_start;
        /* Scan to end of BASIC line, handling strings with embedded newlines */
        while (*line_end && *line_end != '\r') {
            if (*line_end == '"') {
                /* Skip string literal - may contain newlines */
                line_end++;
                while (*line_end && *line_end != '"') {
                    line_end++;
                }
                if (*line_end == '"') line_end++;
            } else if (*line_end == '\n') {
                /* Check if this is truly the end of the BASIC line */
                /* If next char is a digit, it's a new BASIC line */
                if (line_end[1] >= '0' && line_end[1] <= '9') {
                    break;
                }
                /* Otherwise it's embedded in a string or continuation, keep going */
                line_end++;
            } else {
                line_end++;
            }
        }
        line_num = p->current_line_number;
        tok = tokenizer_peek(p->tokenizer);
        column = tok->column;
    }
    
    /* Print the error message */
    fprintf(stderr, "ERROR at line %d: %s\n", line_num, msg);
    
    /* Print the source line */
    fprintf(stderr, "  ");
    fwrite(line_start, 1, line_end - line_start, stderr);
    fprintf(stderr, "\n");
    
    /* Print the position indicator (caret) */
    fprintf(stderr, "  ");
    for (i = 0; i < column; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^\n");
}

/* Record and validate a line number */
static int record_line_number(Parser *p, int line_num) {
    int i;
    char msg[128];
    
    /* Check for valid range (classic BASIC allows 0-32767) */
    if (line_num < 0 || line_num > 32767) {
        sprintf(msg, "Line number %d exceeds maximum (32767)", line_num);
        set_error(p, msg);
        return 0;
    }
    
    /* Check for sequential ordering */
    if (p->previous_line_number > 0 && line_num <= p->previous_line_number) {
        if (line_num == p->previous_line_number) {
            sprintf(msg, "Duplicate line number %d", line_num);
        } else {
            sprintf(msg, "Line number %d must be greater than previous line %d", 
                   line_num, p->previous_line_number);
        }
        set_error(p, msg);
        return 0;
    }
    
    /* Check if we've already seen this line number (shouldn't happen with sequential check) */
    for (i = 0; i < p->line_numbers_count; i++) {
        if (p->line_numbers[i] == line_num) {
            sprintf(msg, "Duplicate line number %d", line_num);
            set_error(p, msg);
            return 0;
        }
    }
    
    /* Add to line numbers array */
    if (p->line_numbers_count >= p->line_numbers_capacity) {
        int new_cap = p->line_numbers_capacity == 0 ? 64 : p->line_numbers_capacity * 2;
        p->line_numbers = realloc(p->line_numbers, new_cap * sizeof(int));
        p->line_numbers_capacity = new_cap;
    }
    p->line_numbers[p->line_numbers_count++] = line_num;
    p->previous_line_number = line_num;
    
    return 1;
}

/* Match and consume a terminal token */
static int match_terminal(Parser *p, unsigned char expected) {
    Token *tok = tokenizer_peek(p->tokenizer);
    
    if (tok->type == expected) {
        tokenizer_next(p->tokenizer);
        return 1;
    }
    
    return 0;
}

/* Get operator precedence from table */
static int get_operator_precedence(unsigned char token, int on_stack) {
    int i;
    
    for (i = 0; i < operator_table_size; i++) {
        if (operator_table[i].token == token) {
            return on_stack ? operator_table[i].come_off_stack : operator_table[i].go_on_stack;
        }
    }
    
    return 0;
}

/* Forward declarations for parse actions */
static ParseNode* parse_expression_pratt_prec(Parser *p, int min_prec);
static ParseNode* parse_expression_pratt(Parser *p);

/* ===== Parse Action Functions (Data-Driven Pratt Parser) ===== */

/* Parse number literal */
static ParseNode* parse_number_literal(Parser *p, ParseNode *left) {
    Token *tok = tokenizer_peek(p->tokenizer);
    ParseNode *node = node_create(NODE_CONSTANT);
    node->value = tok->value;
    node->token = TOK_NUMBER;
    tokenizer_next(p->tokenizer);
    return node;
}

/* Parse string literal */
static ParseNode* parse_string_literal(Parser *p, ParseNode *left) {
    Token *tok = tokenizer_peek(p->tokenizer);
    ParseNode *node = node_create(NODE_CONSTANT);
    node->token = TOK_STRING;
    node->text = my_strdup(tok->text);
    tokenizer_next(p->tokenizer);
    return node;
}

/* Parse variable (with optional array subscripts) */
static ParseNode* parse_variable(Parser *p, ParseNode *left) {
    Token *tok = tokenizer_peek(p->tokenizer);
    ParseNode *var = node_create(NODE_VARIABLE);
    var->text = my_strdup(tok->text);
    var->token = TOK_IDENT;
    tokenizer_next(p->tokenizer);
    
    /* Check for array subscripts */
    if (tokenizer_peek(p->tokenizer)->type == TOK_CLPRN) {
        ParseNode *subscript1;
        ParseNode *subscript2;
        tokenizer_next(p->tokenizer);  /* Consume '(' */
        
        /* Parse first subscript */
        if (tokenizer_peek(p->tokenizer)->type != TOK_CRPRN) {
            subscript1 = parse_expression_pratt(p);
            node_add_child(var, subscript1);
            
            /* Check for second subscript (2D array) */
            if (tokenizer_peek(p->tokenizer)->type == TOK_CCOM) {
                tokenizer_next(p->tokenizer);  /* Consume ',' */
                subscript2 = parse_expression_pratt(p);
                node_add_child(var, subscript2);
            }
        }
        
        if (!match_terminal(p, TOK_CRPRN)) {
            set_error(p, "Expected ')' after array subscript");
            return var;
        }
    }
    
    return var;
}

/* Parse parenthesized expression */
static ParseNode* parse_parenthesized(Parser *p, ParseNode *left) {
    ParseNode *expr;
    tokenizer_next(p->tokenizer);  /* Consume '(' */
    expr = parse_expression_pratt(p);
    if (!match_terminal(p, TOK_CRPRN)) {
        set_error(p, "Expected ')'");
        return expr;
    }
    return expr;
}

/* Parse unary plus */
static ParseNode* parse_unary_plus(Parser *p, ParseNode *left) {
    ParseNode *operand;
    tokenizer_next(p->tokenizer);  /* Consume '+' */
    operand = parse_expression_pratt_prec(p, 7);  /* Unary precedence */
    /* Optimization: +expr is just expr */
    return operand;
}

/* Parse unary minus */
static ParseNode* parse_unary_minus(Parser *p, ParseNode *left) {
    ParseNode *operand, *neg;
    tokenizer_next(p->tokenizer);  /* Consume '-' */
    operand = parse_expression_pratt_prec(p, 7);  /* Unary precedence */
    
    neg = node_create(NODE_OPERATOR);
    neg->token = TOK_CUMINUS;
    node_add_child(neg, operand);
    return neg;
}

/* Parse unary NOT */
static ParseNode* parse_unary_not(Parser *p, ParseNode *left) {
    ParseNode *operand, *not_node;
    tokenizer_next(p->tokenizer);  /* Consume NOT */
    operand = parse_expression_pratt_prec(p, 7);  /* Unary precedence */
    
    not_node = node_create(NODE_OPERATOR);
    not_node->token = TOK_CNOT;
    node_add_child(not_node, operand);
    return not_node;
}

/* Parse function call */
static ParseNode* parse_function_call(Parser *p, ParseNode *left) {
    Token *tok = tokenizer_peek(p->tokenizer);
    const FunctionEntry *func = get_function_metadata(tok->type);
    ParseNode *call;
    ParseNode *arg;
    int arg_count = 0;
    
    if (!func) {
        set_error(p, "Unknown function");
        return NULL;
    }
    
    call = node_create(NODE_FUNCTION_CALL);
    call->token = tok->type;
    tokenizer_next(p->tokenizer);
    
    /* Parse argument list - expect ( expr [, expr]* ) */
    if (tokenizer_peek(p->tokenizer)->type == TOK_CLPRN) {
        tokenizer_next(p->tokenizer);
        if (tokenizer_peek(p->tokenizer)->type != TOK_CRPRN) {
            /* Parse first argument */
            arg = parse_expression_pratt(p);
            node_add_child(call, arg);
            arg_count++;
            
            /* Parse additional arguments separated by commas */
            while (tokenizer_peek(p->tokenizer)->type == TOK_CCOM) {
                tokenizer_next(p->tokenizer);  /* Skip comma */
                arg = parse_expression_pratt(p);
                node_add_child(call, arg);
                arg_count++;
            }
        }
        if (!match_terminal(p, TOK_CRPRN)) {
            set_error(p, "Expected ')' after function argument");
            return call;
        }
    }
    
    /* Validate arity */
    if (arg_count < func->min_arity || 
        (func->max_arity >= 0 && arg_count > func->max_arity)) {
        char msg[128];
        if (func->min_arity == func->max_arity) {
            snprintf(msg, sizeof(msg), "%s expects %d argument%s, got %d",
                     func->name, func->min_arity, 
                     func->min_arity == 1 ? "" : "s", arg_count);
        } else if (func->max_arity < 0) {
            snprintf(msg, sizeof(msg), "%s expects at least %d argument%s, got %d",
                     func->name, func->min_arity,
                     func->min_arity == 1 ? "" : "s", arg_count);
        } else {
            snprintf(msg, sizeof(msg), "%s expects %d-%d arguments, got %d",
                     func->name, func->min_arity, func->max_arity, arg_count);
        }
        set_error(p, msg);
    }
    
    return call;
}

/* Parse binary operator (infix) */
static ParseNode* parse_binary_op(Parser *p, ParseNode *left) {
    Token *tok = tokenizer_peek(p->tokenizer);
    unsigned char op = tok->type;
    int prec = get_operator_precedence(op, 0);
    ParseNode *right, *op_node;
    
    tokenizer_next(p->tokenizer);
    
    /* For right-associative operators (^), use same precedence, otherwise prec+1 */
    if (op == TOK_CEXP) {
        /* Right associative */
        right = parse_expression_pratt_prec(p, prec);
    } else {
        /* Left associative */
        right = parse_expression_pratt_prec(p, prec + 1);
    }
    
    /* Create operator node */
    op_node = node_create(NODE_OPERATOR);
    op_node->token = op;
    node_add_child(op_node, left);
    node_add_child(op_node, right);
    
    return op_node;
}

/* Parse expression using operator precedence (Pratt parsing style) */
static ParseNode* parse_expression_pratt(Parser *p) {
    return parse_expression_pratt_prec(p, 0);
}

/* Parse expression with minimum precedence (ENUM-BASED data-driven Pratt parser) */
static ParseNode* parse_expression_pratt_prec(Parser *p, int min_prec) {
    Token *tok;
    ParseNode *left;
    const OperatorEntry *op_entry;
    int prec;
    
    tok = tokenizer_peek(p->tokenizer);
    
    /* === NULL DENOTATION (nud): Parse prefix/atom === */
    op_entry = get_operator_entry(tok->type);
    
    if (!op_entry || op_entry->nud == PA_NONE) {
        set_error(p, "Expected expression");
        return NULL;
    }
    
    /* Dispatch based on nud action type */
    switch (op_entry->nud) {
        case PA_NUMBER_LITERAL:
            left = parse_number_literal(p, NULL);
            break;
        case PA_STRING_LITERAL:
            left = parse_string_literal(p, NULL);
            break;
        case PA_VARIABLE:
            left = parse_variable(p, NULL);
            break;
        case PA_PARENTHESIZED:
            left = parse_parenthesized(p, NULL);
            break;
        case PA_UNARY_PLUS:
            left = parse_unary_plus(p, NULL);
            break;
        case PA_UNARY_MINUS:
            left = parse_unary_minus(p, NULL);
            break;
        case PA_UNARY_NOT:
            left = parse_unary_not(p, NULL);
            break;
        case PA_FUNCTION_CALL:
            left = parse_function_call(p, NULL);
            break;
        default:
            set_error(p, "Invalid nud action");
            return NULL;
    }
    
    if (!left) {
        return NULL;
    }
    
    /* === LEFT DENOTATION (led): Handle infix/postfix operators === */
    while (1) {
        tok = tokenizer_peek(p->tokenizer);
        op_entry = get_operator_entry(tok->type);
        
        /* No operator entry or no led action? Done. */
        if (!op_entry || op_entry->led == PA_NONE) {
            break;
        }
        
        /* Check precedence */
        prec = op_entry->go_on_stack;
        if (prec < min_prec) {
            break;
        }
        
        /* Dispatch based on led action type */
        switch (op_entry->led) {
            case PA_BINARY_OP:
                left = parse_binary_op(p, left);
                break;
            default:
                set_error(p, "Invalid led action");
                return NULL;
        }
        
        if (!left) {
            break;
        }
    }
    
    return left;
}

/* TABLE-DRIVEN SYNTAX PARSER - the core of the refactored approach */

/* Parse using syntax table rules */
static int recursion_depth = 0;
static ParseNode* parse_nonterminal(Parser *p, NonTerminal nt) {
    const SyntaxEntry *rule;
    ParseNode *node, *child;
    int i, alt_start;
    unsigned char opcode, tok_expected;
    Token *tok;
    Tokenizer saved_state;
    char msg[128];
    
    /* Special case: NT_STATEMENT means parse a complete statement */
    if (nt == NT_STATEMENT) {
        return parser_parse_statement(p);
    }
    
    /* Special case: NT_REM_BODY - consume all tokens until EOS */
    if (nt == NT_REM_BODY) {
        node = node_create(NODE_STATEMENT);
        node->token = TOK_REM;
        
        /* Consume all tokens until end of statement */
        while (1) {
            tok = tokenizer_peek(p->tokenizer);
            if (tok->type == TOK_CEOS || tok->type == TOK_CCR || tok->type == TOK_EOF) {
                break;
            }
            tokenizer_next(p->tokenizer);
        }
        return node;
    }
    
    /* DATA_VAL is now handled by the grammar (supports epsilon for null values) */
    
    /* Special case: NT_IFBODY means parse statement(s) until ELSE or EOS */
    if (nt == NT_IFBODY) {
        Tokenizer saved;
        Token *next;
        
        /* Check if next token is a line number (for THEN linenum syntax) */
        tok = tokenizer_peek(p->tokenizer);
        if (tok->type == TOK_NUMBER) {
            /* THEN linenum - create constant node */
            node = node_create(NODE_CONSTANT);
            node->value = tok->value;
            node->token = TOK_NUMBER;
            tokenizer_next(p->tokenizer);
            return node;
        }
        
        /* Parse statement(s) until we hit ELSE, colon-ELSE, or end-of-statement */
        /* We need to parse statements and check after each one for ELSE */
        node = node_create(NODE_EXPRESSION);
        
        while (1) {
            tok = tokenizer_peek(p->tokenizer);
            
            /* Stop at ELSE, end of line, or EOF */
            if (tok->type == TOK_ELSE || tok->type == TOK_CCR || tok->type == TOK_EOF) {
                break;
            }
            
            /* Stop at colon if followed by ELSE */
            if (tok->type == TOK_CEOS) {
                /* Peek ahead to see if ELSE follows */
                saved = *p->tokenizer;
                tokenizer_next(p->tokenizer);
                next = tokenizer_peek(p->tokenizer);
                *p->tokenizer = saved;
                
                if (next->type == TOK_ELSE) {
                    /* Stop here - don't consume the colon */
                    break;
                }
                
                /* It's a regular colon - consume it and continue */
                tokenizer_next(p->tokenizer);
                continue;
            }
            
            /* Parse one statement */
            child = parser_parse_statement(p);
            if (child) {
                node_add_child(node, child);
            } else if (!p->error) {
                /* No statement but no error - we're done */
                break;
            } else {
                /* Error occurred */
                return NULL;
            }
            
            /* Check what follows */
            tok = tokenizer_peek(p->tokenizer);
            if (tok->type == TOK_CEOS) {
                /* Colon - check if ELSE follows */
                saved = *p->tokenizer;
                tokenizer_next(p->tokenizer);
                next = tokenizer_peek(p->tokenizer);
                *p->tokenizer = saved;
                
                if (next->type == TOK_ELSE) {
                    /* Stop here - ELSE coming up */
                    break;
                }
                
                /* Regular colon - consume and continue */
                tokenizer_next(p->tokenizer);
            } else {
                /* Not a colon - stop */
                break;
            }
        }
        
        return node;
    }
    
    /* Debug: prevent stack overflow */
    recursion_depth++;
    
    if (recursion_depth > 2000) {
        fprintf(stderr, "Recursion depth %d at NT %d, trying to parse token %d\n", 
                recursion_depth, nt, tokenizer_peek(p->tokenizer)->type);
        sprintf(msg, "Recursion depth exceeded at NT %d", nt);
        set_error(p, msg);
        recursion_depth--;
        return NULL;
    }
    
    /* Get syntax rule for this non-terminal */
    rule = get_syntax_rule(nt);
    if (!rule) {
        sprintf(msg, "No syntax rule for NT %d", nt);
        set_error(p, msg);
        recursion_depth--;
        return NULL;
    }
    
    /* Create node for this non-terminal */
    node = node_create(NODE_EXPRESSION);
    
    /* Save tokenizer state for backtracking on alternatives */
    saved_state = *p->tokenizer;
    
    i = 0;
    alt_start = 0;
    
    while (rule[i].opcode != SYN_RTN && i < 1000) {
        opcode = rule[i].opcode;
        
        switch (opcode) {
            case SYN_OR:
                /* Alternative - if we got here from previous alternative, success */
                /* Skip remaining alternatives and exit */
                if (i > alt_start) {
                    /* Previous alternative succeeded - skip all remaining alternatives */
                    while (i < 1000 && rule[i].opcode != SYN_RTN) {
                        i++;
                    }
                    /* Don't increment past RTN - the while loop will exit */
                } else {
                    /* Mark start of this alternative */
                    alt_start = i + 1;
                    i++;
                }
                break;
                
            case SYN_NULL:
                /* Accept empty production - this alternative succeeds with no consumption */
                i++;
                return node;
                
            case SYN_VEXP:
                /* Special expression parser */
                child = parse_expression_pratt(p);
                if (child) {
                    node_add_child(node, child);
                    i++;
                } else {
                    /* Expression parsing failed - check if we can try alternatives */
                    int has_alt = 0;
                    int j;
                    
                    /* Clear error - might be recoverable via alternatives */
                    p->error = 0;
                    
                    /* Look for alternatives */
                    for (j = i + 1; rule[j].opcode != SYN_RTN; j++) {
                        if (rule[j].opcode == SYN_OR) {
                            has_alt = 1;
                            break;
                        }
                    }
                    
                    if (has_alt) {
                        /* Backtrack and try next alternative */
                        *p->tokenizer = saved_state;
                        /* Skip to next SYN_OR */
                        while (i < 1000 && rule[i].opcode != SYN_OR && rule[i].opcode != SYN_RTN) {
                            i++;
                        }
                        if (rule[i].opcode == SYN_OR) {
                            alt_start = i + 1;
                            i++;
                            continue;
                        }
                    }
                    
                    /* No alternatives - this rule fails */
                    recursion_depth--;
                    return NULL;
                }
                break;
                
            case SYN_ANTV:
                /* Non-terminal reference */
                {
                    NonTerminal child_nt = (NonTerminal)(rule[i].data[0] | (rule[i].data[1] << 8));
                    child = parse_nonterminal(p, child_nt);
                    if (child) {
                        node_add_child(node, child);
                        i++;
                    } else {
                        /* Non-terminal failed - try next alternative if available */
                        int has_alt = 0;
                        int j;
                        
                        /* Look for alternatives */
                        for (j = i + 1; rule[j].opcode != SYN_RTN; j++) {
                            if (rule[j].opcode == SYN_OR) {
                                has_alt = 1;
                                break;
                            }
                        }
                        
                        if (has_alt) {
                            /* Clear any error before trying next alternative */
                            p->error = 0;
                            p->error_msg[0] = '\0';
                            
                            /* Backtrack to try next alternative */
                            *p->tokenizer = saved_state;
                            /* Clear any partial nodes */
                            node->child_count = 0;
                            /* Skip to next SYN_OR */
                            while (i < 1000 && rule[i].opcode != SYN_OR && rule[i].opcode != SYN_RTN) {
                                i++;
                            }
                            if (rule[i].opcode == SYN_OR) {
                                alt_start = i + 1;
                                i++;
                                continue;
                            }
                        }
                        
                        /* No alternatives - propagate error if set, otherwise just return NULL */
                        if (!p->error) {
                            /* No explicit error was set - this is just a failed parse attempt */
                        }
                        recursion_depth--;
                        return NULL;
                    }
                }
                break;
                
            default:
                /* Terminal token */
                if (opcode & TC_TERMINAL) {
                    tok_expected = opcode & ~TC_TERMINAL;
                    tok = tokenizer_peek(p->tokenizer);
                    
                    if (tok->type == tok_expected) {
                        /* Match - create token node */
                        if (tok->type == TOK_IDENT) {
                            child = node_create(NODE_VARIABLE);
                            child->text = my_strdup(tok->text);
                            child->token = tok->type;
                        } else if (tok->type == TOK_NUMBER) {
                            child = node_create(NODE_CONSTANT);
                            child->value = tok->value;
                            child->token = tok->type;
                        } else if (tok->type == TOK_STRING) {
                            child = node_create(NODE_CONSTANT);
                            child->text = my_strdup(tok->text);
                            child->token = tok->type;
                        } else {
                            child = node_create(NODE_OPERATOR);
                            child->token = tok->type;
                        }
                        node_add_child(node, child);
                        tokenizer_next(p->tokenizer);
                    } else {
                        /* Syntax error - try next alternative if available */
                        int has_alt;
                        int j;
                        
                        has_alt = 0;
                        for (j = i + 1; rule[j].opcode != SYN_RTN; j++) {
                            if (rule[j].opcode == SYN_OR) {
                                has_alt = 1;
                                break;
                            }
                        }
                        
                        if (has_alt) {
                            /* Backtrack to try next alternative */
                            *p->tokenizer = saved_state;
                            /* Skip to next SYN_OR */
                            while (i < 1000 && rule[i].opcode != SYN_OR && rule[i].opcode != SYN_RTN) {
                                i++;
                            }
                            if (rule[i].opcode == SYN_OR) {
                                alt_start = i + 1;
                                i++;
                                continue;
                            }
                        }
                        
                        /* No alternatives left in this rule - backtrack to parent */
                        recursion_depth--;
                        return NULL;
                    }
                }
                i++;
                break;
        }
    }
    
    recursion_depth--;
    return node;
}

/* TRULY TABLE-DRIVEN STATEMENT PARSER - NO SWITCH! */
ParseNode* parser_parse_statement(Parser *p) {
    Token *tok;
    ParseNode *stmt;
    NonTerminal stmt_rule;
    char msg[128];
    
    tok = tokenizer_peek(p->tokenizer);
    
    /* Empty or end of line */
    if (tok->type == TOK_CCR || tok->type == TOK_EOF || tok->type == TOK_CEOS) {
        return NULL;
    }
    
    /* Line numbers should NOT appear here - they're handled in parse_program */
    if (tok->type == TOK_NUMBER) {
        /* This is an error - line numbers should only appear at the start of a physical line */
        sprintf(msg, "Unexpected line number in statement (line numbers only allowed at start of line)");
        set_error(p, msg);
        return NULL;
    }
    
    /* 
     * Special handling for PRINT - manually parse to track trailing separators
     * 
     * JUSTIFICATION: PRINT is kept as a special case because:
     * 1. Complex syntax with optional channel, multiple alternating expressions/separators
     * 2. Trailing separator detection affects formatting (semicolon = no newline)
     * 3. Expression vs string output context depends on position
     * 4. Table-driven approach would require 10+ new non-terminals for marginal benefit
     * 5. PRINT is performance-critical for output and benefits from direct implementation
     */
    if (tok->type == TOK_PRINT || tok->type == TOK_QUESTION) {
        ParseNode *channel_node = NULL;
        stmt = node_create(NODE_STATEMENT);
        stmt->token = tok->type;
        stmt->line_number = p->current_line_number;
        tokenizer_next(p->tokenizer);
        
        /* Check for PRINT # channel syntax */
        tok = tokenizer_peek(p->tokenizer);
        if (tok->type == TOK_CPND) {
            tokenizer_next(p->tokenizer);  /* Consume # */
            /* Parse channel expression */
            channel_node = parse_expression_pratt(p);
            if (!channel_node) {
                set_error(p, "Expected channel number after #");
                return stmt;
            }
            /* Expect comma after channel */
            tok = tokenizer_peek(p->tokenizer);
            if (tok->type == TOK_CCOM) {
                tokenizer_next(p->tokenizer);  /* Consume comma */
            } else {
                set_error(p, "Expected comma after channel number in PRINT#");
                return stmt;
            }
            /* Add channel node as first child */
            node_add_child(stmt, channel_node);
        }
        
        /* Manually parse PRINT arguments and separators */
        while (1) {
            ParseNode *expr_node;
            ParseNode *sep_node;
            
            tok = tokenizer_peek(p->tokenizer);
            
            /* End of statement or ELSE? */
            if (tok->type == TOK_CCR || tok->type == TOK_EOF || tok->type == TOK_CEOS || tok->type == TOK_ELSE) {
                break;
            }
            
            /* Separator? */
            if (tok->type == TOK_CSC || tok->type == TOK_CCOM) {
                sep_node = node_create(NODE_OPERATOR);  /* Use OPERATOR type for separators */
                sep_node->token = tok->type;
                node_add_child(stmt, sep_node);
                tokenizer_next(p->tokenizer);
                continue;
            }
            
            /* Expression or string */
            expr_node = parse_expression_pratt(p);
            if (expr_node) {
                node_add_child(stmt, expr_node);
            } else {
                break;
            }
        }
        
        /* Don't consume EOS/CR - let parse_program handle it */
        return stmt;
    }
    
    /* TABLE LOOKUP - get syntax rule from statement table */
    stmt_rule = get_statement_rule(tok->type);
    
    if (stmt_rule == (NonTerminal)-1) {
        /* Unknown statement - provide helpful error message */
        if (tok->type == TOK_IDENT) {
            /* This shouldn't happen as IDENT should map to implied LET */
            sprintf(msg, "Unknown statement");
        } else {
            sprintf(msg, "Unknown or misplaced %s", token_name(tok->type));
        }
        set_error(p, msg);
        /* Skip the bad token to allow recovery */
        tokenizer_next(p->tokenizer);
        return NULL;
    }
    
    /* Create statement node */
    stmt = node_create(NODE_STATEMENT);
    stmt->token = tok->type;
    stmt->line_number = p->current_line_number;
    
    /* Save the statement start token for error reporting */
    {
        Token stmt_start_token = *tok;
        
        /* For implied LET (identifier at start), don't consume token yet */
        if (tok->type != TOK_IDENT) {
            tokenizer_next(p->tokenizer);
        }
        
        /* TABLE-DRIVEN PARSE using the syntax rule */
        {
            ParseNode *rule_result = parse_nonterminal(p, stmt_rule);
            int i;
            
            /* Transfer children from rule result to statement node */
            if (rule_result) {
                for (i = 0; i < rule_result->child_count; i++) {
                    node_add_child(stmt, rule_result->children[i]);
                    rule_result->children[i] = NULL;  /* Prevent double-free */
                }
                node_free(rule_result);
            } else if (!p->error) {
                /* Parse failed but no error was set - set a helpful error */
                Token *err_tok = tokenizer_peek(p->tokenizer);
                
                /* For implied LET, provide a more specific message */
                if (stmt->token == TOK_IDENT) {
                    /* Manually set error position to the identifier token */
                    p->error_column = stmt_start_token.column;
                    p->error_line_number = stmt_start_token.line;
                    /* Find the line content */
                    {
                        const char *scan;
                        const char *line_end;
                        int target_line = stmt_start_token.line;
                        int line_count = 1;
                        
                        p->error_line_start = p->tokenizer->input_start;
                        scan = p->tokenizer->input_start;
                        while (*scan && line_count < target_line) {
                            if (*scan == '\n') {
                                line_count++;
                                if (line_count == target_line) {
                                    p->error_line_start = scan + 1;
                                    break;
                                }
                            }
                            scan++;
                        }
                        line_end = p->error_line_start;
                        while (*line_end && *line_end != '\n' && *line_end != '\r') {
                            line_end++;
                        }
                        p->error_line_end = line_end;
                    }
                    
                    /* Check what follows the identifier to provide a better error message */
                    if (err_tok->type == TOK_CLPRN) {
                        /* Looks like a function call - unknown function name */
                        sprintf(msg, "Unknown function '%s'", stmt_start_token.text);
                    } else if (err_tok->type == TOK_CCR || err_tok->type == TOK_EOF || err_tok->type == TOK_CEOS) {
                        /* Identifier alone on a line - unknown statement or missing '=' */
                        sprintf(msg, "Unknown statement '%s' (or missing '=' for assignment)", stmt_start_token.text);
                    } else if (err_tok->type == TOK_CCOM || err_tok->type == TOK_CSC) {
                        /* Statement separator after identifier - probably unknown statement */
                        sprintf(msg, "Unknown statement '%s'", stmt_start_token.text);
                    } else {
                        /* Has something else after it - probably missing '=' */
                        sprintf(msg, "Expected '=' but found %s (for variable '%s' assignment)", 
                                token_name(err_tok->type), stmt_start_token.text);
                    }
                } else {
                    /* Generic syntax error for other statements */
                    sprintf(msg, "Syntax error in %s statement", token_name(stmt->token));
                }
                set_error(p, msg);
                return NULL;
            } else {
                /* Error was already set by parse_nonterminal */
                return NULL;
            }
        }
    }
    
    /* Don't consume EOS here - parse_program handles all EOS/CR consumption */
    
    return stmt;
}

/* Check if a line number exists in the program */
static int line_number_exists(Parser *p, int line_num) {
    int i;
    for (i = 0; i < p->line_numbers_count; i++) {
        if (p->line_numbers[i] == line_num) {
            return 1;
        }
    }
    return 0;
}

/* Recursively validate GOTO/GOSUB line number references */
static void validate_node_line_refs(Parser *p, ParseNode *node) {
    int i;
    int target_line;
    
    if (!node) return;
    
    /* Check if this is a GOTO or GOSUB statement */
    if (node->type == NODE_STATEMENT && 
        (node->token == TOK_GOTO || node->token == TOK_CGTO ||
         node->token == TOK_GOSUB_S || node->token == TOK_CGS)) {
        
        /* First child should be the target line number */
        if (node->child_count > 0 && node->children[0]->type == NODE_CONSTANT) {
            target_line = (int)node->children[0]->value;
            
            /* Check if this line number exists */
            if (!line_number_exists(p, target_line)) {
                /* Report error without source context (simpler for post-parse validation) */
                fprintf(stderr, "ERROR at line %d: Undefined line number %d in %s\n",
                       node->line_number,
                       target_line, 
                       (node->token == TOK_GOTO || node->token == TOK_CGTO) ? "GOTO" : "GOSUB");
                p->error_count++;
            }
        }
    }
    
    /* Check ON GOTO/GOSUB statements */
    if (node->type == NODE_STATEMENT && node->token == TOK_ON) {
        /* ON statements have expression first, then list of line numbers */
        for (i = 1; i < node->child_count; i++) {
            if (node->children[i]->type == NODE_CONSTANT) {
                target_line = (int)node->children[i]->value;
                
                if (!line_number_exists(p, target_line)) {
                    fprintf(stderr, "ERROR at line %d: Undefined line number %d in ON statement\n",
                           node->line_number, target_line);
                    p->error_count++;
                }
            }
        }
    }
    
    /* Recursively check all children */
    for (i = 0; i < node->child_count; i++) {
        validate_node_line_refs(p, node->children[i]);
    }
}

/* Validate all line number references in the program */
static void validate_line_number_references(Parser *p, ParseNode *program) {
    if (!program) return;
    validate_node_line_refs(p, program);
}

/* Parse entire program */
ParseNode* parser_parse_program(Parser *p) {
    ParseNode *program, *stmt;
    Token *tok;
    int line_num;
    
    program = node_create(NODE_STATEMENT);
    program->token = 0; /* Program root */
    
    while (1) {
        tok = tokenizer_peek(p->tokenizer);
        
        if (tok->type == TOK_EOF) {
            break;
        }
        
        /* Skip empty lines (just CR) */
        if (tok->type == TOK_CCR) {
            tokenizer_next(p->tokenizer);
            continue;
        }
        
        /* Expect line number at start of each physical line */
        if (tok->type != TOK_NUMBER) {
            char msg[128];
            sprintf(msg, "Program line must start with a line number (found %s)", token_name(tok->type));
            set_error(p, msg);
            display_error_with_context(p, msg);
            p->error_count++;
            
            /* Skip to end of line */
            while (1) {
                tok = tokenizer_peek(p->tokenizer);
                if (tok->type == TOK_EOF || tok->type == TOK_CCR) {
                    break;
                }
                tokenizer_next(p->tokenizer);
            }
            if (tok->type == TOK_CCR) {
                tokenizer_next(p->tokenizer);
            }
            p->error = 0;
            p->error_column = -1;
            continue;
        }
        
        /* Parse line number */
        line_num = (int)tok->value;
        
        /* Capture the start of this BASIC line for error reporting */
        /* We need to find where this BASIC line number appears in the source */
        /* Scan from the start of the file looking for this line number */
        /* For duplicate line numbers, find the one closest to current position */
        {
            const char *pos = p->tokenizer->input_start;
            const char *line_start_candidate = pos;
            const char *best_match = NULL;
            /* We're currently positioned just after reading the line number token */
            /* So we use the current tokenizer position as our reference */
            const char *current_pos = p->tokenizer->input;
            
            /* Scan forward looking for lines starting with this number */
            /* Keep the match that's closest to (but not after) current_pos */
            while (*pos && pos <= current_pos) {
                /* Skip to start of next line after newlines */
                while (*pos && (*pos == '\r' || *pos == '\n') && pos <= current_pos) {
                    pos++;
                }
                if (pos > current_pos) break;
                
                line_start_candidate = pos;
                
                /* Skip leading spaces/tabs on this line */
                while (*pos && (*pos == ' ' || *pos == '\t') && pos <= current_pos) {
                    pos++;
                }
                if (pos > current_pos) break;
                
                /* Check if this position starts with our line number */
                if (*pos && (*pos >= '0' && *pos <= '9')) {
                    int num = 0;
                    while (*pos >= '0' && *pos <= '9') {
                        num = num * 10 + (*pos - '0');
                        pos++;
                    }
                    
                    /* Check if we found the right line number */
                    if (num == line_num && (*pos == ' ' || *pos == '\t' || *pos == '\r' || *pos == '\n' || *pos == '\0')) {
                        /* This is a match! Keep it as best match so far */
                        best_match = line_start_candidate;
                    }
                    
                    /* Move to next line */
                    while (*pos && *pos != '\n' && *pos != '\r') {
                        pos++;
                    }
                } else if (*pos) {
                    /* Not a digit, skip to next line */
                    while (*pos && *pos != '\n' && *pos != '\r') {
                        pos++;
                    }
                }
            }
            
            /* Use the best match we found */
            if (best_match) {
                p->current_basic_line_start = best_match;
            } else {
                /* Fallback if not found (shouldn't happen) */
                p->current_basic_line_start = p->tokenizer->line_start;
            }
        }
        
        tokenizer_next(p->tokenizer);
        
        /* Store line number */
        p->current_line_number = line_num;
        
        /* Validate and record the line number */
        if (!record_line_number(p, line_num)) {
            display_error_with_context(p, p->error_msg);
            p->error_count++;
            /* Skip to end of line */
            while (1) {
                tok = tokenizer_peek(p->tokenizer);
                if (tok->type == TOK_EOF || tok->type == TOK_CCR) {
                    break;
                }
                tokenizer_next(p->tokenizer);
            }
            if (tok->type == TOK_CCR) {
                tokenizer_next(p->tokenizer);
            }
            p->error = 0;
            p->error_column = -1;
            continue;
        }
        
        /* Now parse all statements on this line (separated by colons) */
        while (1) {
            tok = tokenizer_peek(p->tokenizer);
            
            /* End of line? */
            if (tok->type == TOK_CCR || tok->type == TOK_EOF) {
                if (tok->type == TOK_CCR) {
                    tokenizer_next(p->tokenizer);
                }
                break;
            }
            
            /* Empty statement (consecutive colons or colon at end)? */
            if (tok->type == TOK_CEOS) {
                tokenizer_next(p->tokenizer);
                continue;
            }
            
            /* Parse one statement */
            stmt = parser_parse_statement(p);
            if (stmt) {
                node_add_child(program, stmt);
            }
            
            if (p->error) {
                display_error_with_context(p, p->error_msg);
                p->error_count++;
                
                /* Skip to end of line */
                while (1) {
                    tok = tokenizer_peek(p->tokenizer);
                    if (tok->type == TOK_EOF || tok->type == TOK_CCR) {
                        break;
                    }
                    tokenizer_next(p->tokenizer);
                }
                if (tok->type == TOK_CCR) {
                    tokenizer_next(p->tokenizer);
                }
                
                /* Clear error to continue with next line */
                p->error = 0;
                p->error_column = -1;
                p->error_line_start = NULL;
                p->error_line_end = NULL;
                p->error_line_number = 0;
                break;
            }
            
            /* After parsing a statement, check what follows */
            tok = tokenizer_peek(p->tokenizer);
            if (tok->type == TOK_CEOS) {
                tokenizer_next(p->tokenizer);  /* Consume the colon */
                /* Continue to parse next statement on same line */
            } else if (tok->type == TOK_CCR) {
                tokenizer_next(p->tokenizer);  /* Consume CR */
                break;
            } else if (tok->type == TOK_EOF) {
                break;
            } else {
                /* Unexpected token after statement - treat as error */
                char msg[128];
                sprintf(msg, "Unexpected %s after statement (expected colon or end of line)", token_name(tok->type));
                set_error(p, msg);
                display_error_with_context(p, p->error_msg);
                p->error_count++;
                
                /* Skip to end of line */
                while (1) {
                    tok = tokenizer_peek(p->tokenizer);
                    if (tok->type == TOK_EOF || tok->type == TOK_CCR) {
                        break;
                    }
                    tokenizer_next(p->tokenizer);
                }
                if (tok->type == TOK_CCR) {
                    tokenizer_next(p->tokenizer);
                }
                p->error = 0;
                p->error_column = -1;
                break;
            }
        }
    }
    
    p->root = program;
    
    /* Validate GOTO/GOSUB targets */
    if (p->error_count == 0) {
        validate_line_number_references(p, program);
    }
    
    return program;
}

/* Parse expression (public interface) */
ParseNode* parser_parse_expression(Parser *p) {
    return parse_expression_pratt(p);
}
