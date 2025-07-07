#include <string.h> // For strdup
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
// Ensure string.h is definitely at the top or very early

static void next_token(Parser* parser) {
    token_free(&parser->current_token);
    parser->current_token = parser->peek_token;
    parser->peek_token = lexer_next_token(parser->lexer);
}

Parser* parser_new(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current_token.value = NULL;
    parser->peek_token.value = NULL;
    next_token(parser);
    next_token(parser);
    return parser;
}

void parser_free(Parser* parser) {
    token_free(&parser->current_token);
    token_free(&parser->peek_token);
    free(parser);
}

static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_expression(Parser* parser, int precedence);

static ASTNode* parse_identifier(Parser* parser) {
    return (ASTNode*)identifier_new(parser->current_token.value);
}

static ASTNode* parse_var_declaration(Parser* parser) {
    next_token(parser); // consume TOKEN_KEYWORD_VAR

    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Expected identifier after 'var' at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }

    char* name = strdup(parser->current_token.value);
    ASTNode* size = NULL;

    next_token(parser); // consume identifier

    if (parser->current_token.type == TOKEN_LBRACKET) {
        next_token(parser); // consume '['
        if (parser->current_token.type == TOKEN_RBRACKET) {
            // Empty size, e.g., x[]
            size = NULL; // Or a specific node type for empty size if needed later
        } else {
            size = parse_expression(parser, 0);
            if (!size) { // Error during size expression parsing
                free(name);
                return NULL; // Error already printed by parse_expression
            }
        }
        if (parser->current_token.type != TOKEN_RBRACKET) {
            fprintf(stderr, "Expected ']' after size in variable declaration at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
            if (size) ast_node_free(size);
            free(name);
            return NULL;
        }
        next_token(parser); // consume ']'
    }

    if (parser->current_token.type != TOKEN_ASSIGN) {
        fprintf(stderr, "Expected '=' in variable declaration at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        free(name);
        return NULL;
    }

    next_token(parser); // consume '='

    ASTNode* value = parse_expression(parser, 0);

    if (parser->current_token.type == TOKEN_SEMICOLON) {
        next_token(parser); // consume semicolon
    }

    return (ASTNode*)var_declaration_new(name, size, value);
}

static ASTNode* parse_expression_statement(Parser* parser) {
    ASTNode* expression = parse_expression(parser, 0);
    ExpressionStatement* stmt = expression_statement_new(expression);

    if (parser->current_token.type == TOKEN_SEMICOLON) {
        next_token(parser); // consume semicolon
    }

    return (ASTNode*)stmt;
}

static ASTNode* parse_block_statement(Parser* parser) {
    BlockStatement* block = block_statement_new(NULL);
    next_token(parser); // consume '{'

    ASTNode* head = NULL;
    ASTNode* current = NULL;

    while (parser->current_token.type != TOKEN_RBRACE && parser->current_token.type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            if (head == NULL) {
                head = stmt;
                current = stmt;
            } else {
                current->next = stmt;
                current = stmt;
            }
        } else {
            // If parse_statement returns NULL, it means an error occurred.
            // parse_statement's default case (if hit) should have consumed the token.
            // If a specific parse_X_statement returned NULL without consuming the problematic token,
            // this next_token call ensures progress.
            fprintf(stderr, "Error in block: Problem parsing statement starting near token '%s' (type %d) at line %d, column %d. Attempting to recover by skipping token.\n", parser->current_token.value, parser->current_token.type, parser->current_token.line, parser->current_token.column);
            next_token(parser); // Ensure progress
        }
    }

    if (parser->current_token.type != TOKEN_RBRACE) {
        fprintf(stderr, "Expected '}' after block statement at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    next_token(parser); // consume '}'

    block->statements = head;
    return (ASTNode*)block;
}

static ASTNode* parse_function_declaration(Parser* parser) {
    next_token(parser); // consume 'func'

    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        fprintf(stderr, "Expected function name after 'func' at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    char* name = strdup(parser->current_token.value);
    next_token(parser); // consume function name

    if (parser->current_token.type != TOKEN_LPAREN) {
        fprintf(stderr, "Expected '(' after function name at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        free(name);
        return NULL;
    }
    next_token(parser); // consume '('

    ASTNode* parameters = NULL;
    ASTNode* head = NULL;
    ASTNode* current = NULL;

    while (parser->current_token.type != TOKEN_RPAREN && parser->current_token.type != TOKEN_EOF) {
        if (parser->current_token.type == TOKEN_IDENTIFIER) {
            ASTNode* param = (ASTNode*)identifier_new(parser->current_token.value);
            if (head == NULL) {
                head = param;
                current = param;
            } else {
                current->next = param;
                current = param;
            }
            next_token(parser); // consume identifier
            if (parser->current_token.type == TOKEN_COMMA) {
                next_token(parser); // consume comma
            }
        } else {
            fprintf(stderr, "Expected identifier or ')' in function parameters at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
            free(name);
            return NULL;
        }
    }
    parameters = head;

    if (parser->current_token.type != TOKEN_RPAREN) {
        fprintf(stderr, "Expected ')' after function parameters at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        free(name);
        return NULL;
    }
    next_token(parser); // consume ')'

    if (parser->current_token.type != TOKEN_LBRACE) {
        fprintf(stderr, "Expected '{' before function body at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        free(name);
        return NULL;
    }

    ASTNode* body = parse_block_statement(parser);

    return (ASTNode*)function_declaration_new(name, parameters, body);
}

static ASTNode* parse_return_statement(Parser* parser) {
    next_token(parser); // consume 'return'

    ASTNode* return_value = parse_expression(parser, 0);

    if (parser->current_token.type == TOKEN_SEMICOLON) {
        next_token(parser); // consume semicolon
    }

    return (ASTNode*)return_statement_new(return_value);
}

static ASTNode* parse_while_loop(Parser* parser) {
    next_token(parser); // consume 'while'

    if (parser->current_token.type != TOKEN_LPAREN) {
        fprintf(stderr, "Expected '(' after 'while' at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    next_token(parser); // consume '('

    ASTNode* condition = parse_expression(parser, 0);

    if (parser->current_token.type != TOKEN_RPAREN) {
        fprintf(stderr, "Expected ')' after while condition at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    next_token(parser); // consume ')'

    if (parser->current_token.type != TOKEN_LBRACE) {
        fprintf(stderr, "Expected '{' after while condition at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }

    ASTNode* body = parse_block_statement(parser);

    return (ASTNode*)while_loop_new(condition, body);
}

static ASTNode* parse_for_loop(Parser* parser) {
    next_token(parser); // consume 'for'

    if (parser->current_token.type != TOKEN_LPAREN) {
        fprintf(stderr, "Expected '(' after 'for' at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    next_token(parser); // consume '('

    ASTNode* init = parse_expression(parser, 0);

    if (parser->current_token.type != TOKEN_COMMA) {
        fprintf(stderr, "Expected ',' after for loop initializer at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    next_token(parser); // consume ','

    ASTNode* condition = parse_expression(parser, 0);

    if (parser->current_token.type != TOKEN_COMMA) {
        fprintf(stderr, "Expected ',' after for loop condition at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    next_token(parser); // consume ','

    ASTNode* increment = parse_expression(parser, 0);

    if (parser->current_token.type != TOKEN_RPAREN) {
        fprintf(stderr, "Expected ')' after for loop incrementer at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    next_token(parser); // consume ')'

    if (parser->current_token.type != TOKEN_LBRACE) {
        fprintf(stderr, "Expected '{' after for loop at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }

    ASTNode* body = parse_block_statement(parser);

    return (ASTNode*)for_loop_new(init, condition, increment, body);
}

static ASTNode* parse_import_statement(Parser* parser) {
    next_token(parser); // consume 'import'

    if (parser->current_token.type == TOKEN_STRING_LITERAL) {
        char* path = strdup(parser->current_token.value);
        next_token(parser); // consume string literal

        if (parser->current_token.type != TOKEN_KEYWORD_AS) {
            fprintf(stderr, "Expected 'as' after import path at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
            free(path);
            return NULL;
        }
        next_token(parser); // consume 'as'

        if (parser->current_token.type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "Expected identifier for alias at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
            free(path);
            return NULL;
        }
        char* alias = strdup(parser->current_token.value);
        next_token(parser); // consume alias

        if (parser->current_token.type == TOKEN_SEMICOLON) {
            next_token(parser); // consume semicolon
        }

        return (ASTNode*)import_statement_new(IMPORT_TYPE_ALIAS, path, alias, NULL);
    } else if (parser->current_token.type == TOKEN_LBRACE) {
        next_token(parser); // consume '{'

        ASTNode* imports = NULL;
        ASTNode* head = NULL;
        ASTNode* current = NULL;

        while (parser->current_token.type != TOKEN_RBRACE && parser->current_token.type != TOKEN_EOF) {
            if (parser->current_token.type == TOKEN_IDENTIFIER) {
                ASTNode* import_item = (ASTNode*)identifier_new(parser->current_token.value);
                if (head == NULL) {
                    head = import_item;
                    current = import_item;
                } else {
                    current->next = import_item;
                    current = import_item;
                }
                next_token(parser); // consume identifier
                if (parser->current_token.type == TOKEN_COMMA) {
                    next_token(parser); // consume comma
                }
            } else {
                fprintf(stderr, "Expected identifier or '}' in destructured imports at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
                return NULL;
            }
        }
        imports = head;

        if (parser->current_token.type != TOKEN_RBRACE) {
            fprintf(stderr, "Expected '}' after destructured imports at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
            return NULL;
        }
        next_token(parser); // consume '}'

        if (parser->current_token.type != TOKEN_KEYWORD_FROM) {
            fprintf(stderr, "Expected 'from' after destructured imports at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
            return NULL;
        }
        next_token(parser); // consume 'from'

        if (parser->current_token.type != TOKEN_STRING_LITERAL) {
            fprintf(stderr, "Expected string literal for path at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
            return NULL;
        }
        char* path = strdup(parser->current_token.value);
        next_token(parser); // consume string literal

        if (parser->current_token.type == TOKEN_SEMICOLON) {
            next_token(parser); // consume semicolon
        }

        return (ASTNode*)import_statement_new(IMPORT_TYPE_DESTRUCTURED, path, NULL, imports);
    } else {
        fprintf(stderr, "Invalid import statement at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
}

static ASTNode* parse_statement(Parser* parser) {
    ASTNode* stmt = NULL;
    switch (parser->current_token.type) {
        case TOKEN_KEYWORD_VAR:
            stmt = parse_var_declaration(parser);
            break;
        case TOKEN_IDENTIFIER:
            stmt = parse_expression_statement(parser);
            break;
        case TOKEN_KEYWORD_RETURN:
            stmt = parse_return_statement(parser);
            break;
        case TOKEN_KEYWORD_WHILE:
            stmt = parse_while_loop(parser);
            break;
        case TOKEN_KEYWORD_FOR:
            stmt = parse_for_loop(parser);
            break;
        case TOKEN_KEYWORD_IMPORT:
            stmt = parse_import_statement(parser);
            break;
        case TOKEN_LBRACE:
            stmt = parse_block_statement(parser);
            break;
        case TOKEN_KEYWORD_FUNC:
            stmt = parse_function_declaration(parser);
            break;
        default:
            fprintf(stderr, "Unexpected token at start of statement: %s (type %d) at line %d, column %d\n", parser->current_token.value, parser->current_token.type, parser->current_token.line, parser->current_token.column);
            // Consume the unexpected token to avoid infinite loop
            next_token(parser);
            return NULL;
    }
    return stmt;
}

static int get_precedence(TokenType type) {
    switch (type) {
        case TOKEN_EQ:
        case TOKEN_NEQ:
            return 1;
        case TOKEN_LT:
        case TOKEN_GT:
        case TOKEN_LE:
        case TOKEN_GE:
            return 2;
        case TOKEN_PLUS:
        case TOKEN_MINUS:
            return 3;
        case TOKEN_MULTIPLY:
        case TOKEN_DIVIDE:
        case TOKEN_MODULO:
            return 4;
        default:
            return 0;
    }
}

static ASTNode* parse_prefix_expression(Parser* parser) {
    ASTNode* node = NULL;
    switch (parser->current_token.type) {
        case TOKEN_IDENTIFIER:
            node = parse_identifier(parser);
            next_token(parser); // Consume identifier
            break;
        case TOKEN_NUMBER:
            node = (ASTNode*)number_literal_new(parser->current_token.value);
            next_token(parser); // Consume number
            break;
        case TOKEN_ASCII_LITERAL:
            node = (ASTNode*)ascii_literal_new(parser->current_token.value);
            next_token(parser); // Consume ASCII literal
            break;
        case TOKEN_STRING_LITERAL:
            node = (ASTNode*)string_literal_new(parser->current_token.value);
            next_token(parser); // Consume string literal
            break;
        case TOKEN_LPAREN:
            next_token(parser); // consume '('
            node = parse_expression(parser, 0);
            if (parser->current_token.type != TOKEN_RPAREN) {
                fprintf(stderr, "Expected ')' at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
                return NULL;
            }
            next_token(parser); // consume ')'
            break;
        default:
            fprintf(stderr, "Unexpected token in expression: %s (type %d) at line %d, column %d\n", parser->current_token.value, parser->current_token.type, parser->current_token.line, parser->current_token.column);
            return NULL;
    }
    return node;
}

static ASTNode* parse_infix_expression(Parser* parser, ASTNode* left) {
    TokenType operator = parser->current_token.type;
    int precedence = get_precedence(operator);
    next_token(parser); // consume operator
    ASTNode* right = parse_expression(parser, precedence);

    BinaryOperator op;
    switch (operator) {
        case TOKEN_PLUS: op = BIN_OP_PLUS; break;
        case TOKEN_MINUS: op = BIN_OP_MINUS; break;
        case TOKEN_MULTIPLY: op = BIN_OP_MULTIPLY; break;
        case TOKEN_DIVIDE: op = BIN_OP_DIVIDE; break;
        case TOKEN_MODULO: op = BIN_OP_MODULO; break;
        case TOKEN_EQ: op = BIN_OP_EQ; break;
        case TOKEN_NEQ: op = BIN_OP_NEQ; break;
        case TOKEN_LT: op = BIN_OP_LT; break;
        case TOKEN_GT: op = BIN_OP_GT; break;
        case TOKEN_LE: op = BIN_OP_LE; break;
        case TOKEN_GE: op = BIN_OP_GE; break;
        default: fprintf(stderr, "Invalid binary operator\n"); exit(1);
    }

    return (ASTNode*)binary_expression_new(left, op, right);
}

static ASTNode* parse_call_expression(Parser* parser, ASTNode* function) {
    next_token(parser); // consume '('

    ASTNode* arguments = NULL;
    ASTNode* head = NULL;
    ASTNode* current = NULL;

    while (parser->current_token.type != TOKEN_RPAREN && parser->current_token.type != TOKEN_EOF) {
        ASTNode* arg = parse_expression(parser, 0);
        if (arg) {
            if (head == NULL) {
                head = arg;
                current = arg;
            } else {
                current->next = arg;
                current = arg;
            }
        }
        if (parser->current_token.type == TOKEN_COMMA) {
            next_token(parser); // consume comma
        }
    }
    arguments = head;

    if (parser->current_token.type != TOKEN_RPAREN) {
        fprintf(stderr, "Expected ')' after call arguments at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    next_token(parser); // consume ')'

    return (ASTNode*)call_expression_new(function, arguments);
}

static ASTNode* parse_index_expression(Parser* parser, ASTNode* array) {
    next_token(parser); // consume '['
    ASTNode* index = parse_expression(parser, 0);

    if (parser->current_token.type != TOKEN_RBRACKET) {
        fprintf(stderr, "Expected ']' after index at line %d, column %d\n", parser->current_token.line, parser->current_token.column);
        return NULL;
    }
    next_token(parser); // consume ']'

    return (ASTNode*)index_expression_new(array, index);
}

static ASTNode* parse_expression(Parser* parser, int precedence) {
    ASTNode* left_expr = parse_prefix_expression(parser);

    while (parser->peek_token.type != TOKEN_EOF && parser->peek_token.type != TOKEN_SEMICOLON && precedence < get_precedence(parser->peek_token.type)) {
        next_token(parser);
        if (parser->current_token.type == TOKEN_LPAREN) {
            left_expr = parse_call_expression(parser, left_expr);
        } else if (parser->current_token.type == TOKEN_LBRACKET) {
            left_expr = parse_index_expression(parser, left_expr);
        } else if (parser->current_token.type == TOKEN_ASSIGN) {
            next_token(parser); // consume '='
            ASTNode* value = parse_expression(parser, 0);
            left_expr = (ASTNode*)assign_expression_new(left_expr, value);
        } else {
            left_expr = parse_infix_expression(parser, left_expr);
        }
    }

    return left_expr;
}

Program* parse_program(Parser* parser) {
    Program* program = program_new();
    ASTNode* head = NULL;
    ASTNode* current = NULL;

    while (parser->current_token.type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement(parser);
        if (stmt) {
            if (head == NULL) {
                head = stmt;
                current = stmt;
            } else {
                current->next = stmt;
                current = stmt;
            }
        } else {
            // If parse_statement returns NULL, it means an error occurred.
            // parse_statement's default case (if hit) should have consumed the token.
            // If a specific parse_X_statement returned NULL without consuming the problematic token,
            // this next_token call ensures progress.
            fprintf(stderr, "Error in program: Problem parsing statement starting near token '%s' (type %d) at line %d, column %d. Attempting to recover by skipping token.\n", parser->current_token.value, parser->current_token.type, parser->current_token.line, parser->current_token.column);
            next_token(parser); // Ensure progress
        }
    }

    program->statements = head;
    return program;
}


