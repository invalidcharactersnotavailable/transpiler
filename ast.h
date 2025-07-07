#ifndef AST_H
#define AST_H

typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECLARATION,
    NODE_FUNCTION_DECLARATION,
    NODE_RETURN_STATEMENT,
    NODE_EXPRESSION_STATEMENT,
    NODE_BLOCK_STATEMENT,
    NODE_IDENTIFIER,
    NODE_NUMBER_LITERAL,
    NODE_ASCII_LITERAL,
    NODE_STRING_LITERAL,
    NODE_ASSIGN_EXPRESSION,
    NODE_CALL_EXPRESSION,
    NODE_FOR_LOOP,
    NODE_WHILE_LOOP,
    NODE_IMPORT_STATEMENT,
    NODE_BINARY_EXPRESSION,
    NODE_INDEX_EXPRESSION,
} NodeType;

typedef struct ASTNode {
    NodeType type;
    // Common fields for all nodes
    struct ASTNode* next;
} ASTNode;

typedef struct {
    ASTNode base;
    ASTNode* statements;
} Program;

typedef struct {
    ASTNode base;
    char* name;
    ASTNode* size;
    ASTNode* value;
} VarDeclaration;

typedef struct {
    ASTNode base;
    char* name;
    ASTNode* parameters;
    ASTNode* body;
} FunctionDeclaration;

typedef struct {
    ASTNode base;
    ASTNode* return_value;
} ReturnStatement;

typedef struct {
    ASTNode base;
    ASTNode* expression;
} ExpressionStatement;

typedef struct {
    ASTNode base;
    ASTNode* statements;
} BlockStatement;

typedef struct {
    ASTNode base;
    char* value;
} Identifier;

typedef struct {
    ASTNode base;
    char* value;
} NumberLiteral;

typedef struct {
    ASTNode base;
    char* value;
} AsciiLiteral;

typedef struct {
    ASTNode base;
    char* value;
} StringLiteral;

typedef struct {
    ASTNode base;
    ASTNode* name;
    ASTNode* value;
} AssignExpression;

typedef struct {
    ASTNode base;
    ASTNode* function;
    ASTNode* arguments;
} CallExpression;

typedef struct {
    ASTNode base;
    ASTNode* init;
    ASTNode* condition;
    ASTNode* increment;
    ASTNode* body;
} ForLoop;

typedef struct {
    ASTNode base;
    ASTNode* condition;
    ASTNode* body;
} WhileLoop;

typedef enum {
    IMPORT_TYPE_ALIAS,
    IMPORT_TYPE_DESTRUCTURED,
} ImportType;

typedef struct {
    ASTNode base;
    ImportType import_type;
    char* path;
    char* alias;
    ASTNode* imports; // For destructured imports (list of identifiers)
} ImportStatement;

typedef enum {
    BIN_OP_EQ,
    BIN_OP_NEQ,
    BIN_OP_LT,
    BIN_OP_GT,
    BIN_OP_LE,
    BIN_OP_GE,
    BIN_OP_PLUS,
    BIN_OP_MINUS,
    BIN_OP_MULTIPLY,
    BIN_OP_DIVIDE,
    BIN_OP_MODULO,
} BinaryOperator;

typedef struct {
    ASTNode base;
    ASTNode* left;
    BinaryOperator operator;
    ASTNode* right;
} BinaryExpression;


typedef struct {
    ASTNode base;
    ASTNode* array;
    ASTNode* index;
} IndexExpression;

// Function prototypes for AST node creation
ASTNode* ast_node_new(NodeType type);
Program* program_new();
VarDeclaration* var_declaration_new(char* name, ASTNode* size, ASTNode* value);
FunctionDeclaration* function_declaration_new(char* name, ASTNode* parameters, ASTNode* body);
ReturnStatement* return_statement_new(ASTNode* return_value);
ExpressionStatement* expression_statement_new(ASTNode* expression);
BlockStatement* block_statement_new(ASTNode* statements);
Identifier* identifier_new(char* value);
NumberLiteral* number_literal_new(char* value);
AsciiLiteral* ascii_literal_new(char* value);
StringLiteral* string_literal_new(char* value);
AssignExpression* assign_expression_new(ASTNode* name, ASTNode* value);
CallExpression* call_expression_new(ASTNode* function, ASTNode* arguments);
ForLoop* for_loop_new(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body);
WhileLoop* while_loop_new(ASTNode* condition, ASTNode* body);
ImportStatement* import_statement_new(ImportType import_type, char* path, char* alias, ASTNode* imports);
BinaryExpression* binary_expression_new(ASTNode* left, BinaryOperator operator, ASTNode* right);
IndexExpression* index_expression_new(ASTNode* array, ASTNode* index);

void ast_node_free(ASTNode* node);
void ast_node_list_free(ASTNode* node_list);

#endif // AST_H


