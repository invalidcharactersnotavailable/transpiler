#include <string.h> // For strdup
#include "ast.h"
#include <stdlib.h>
// Ensure string.h is definitely at the top or very early

ASTNode* ast_node_new(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->next = NULL;
    return node;
}

Program* program_new() {
    Program* program = (Program*)malloc(sizeof(Program));
    program->base.type = NODE_PROGRAM;
    program->base.next = NULL;
    program->statements = NULL;
    return program;
}

VarDeclaration* var_declaration_new(char* name, ASTNode* size, ASTNode* value) {
    VarDeclaration* var_decl = (VarDeclaration*)malloc(sizeof(VarDeclaration));
    var_decl->base.type = NODE_VAR_DECLARATION;
    var_decl->base.next = NULL;
    var_decl->name = strdup(name);
    var_decl->size = size;
    var_decl->value = value;
    return var_decl;
}

FunctionDeclaration* function_declaration_new(char* name, ASTNode* parameters, ASTNode* body) {
    FunctionDeclaration* func_decl = (FunctionDeclaration*)malloc(sizeof(FunctionDeclaration));
    func_decl->base.type = NODE_FUNCTION_DECLARATION;
    func_decl->base.next = NULL;
    func_decl->name = strdup(name);
    func_decl->parameters = parameters;
    func_decl->body = body;
    return func_decl;
}

ReturnStatement* return_statement_new(ASTNode* return_value) {
    ReturnStatement* ret_stmt = (ReturnStatement*)malloc(sizeof(ReturnStatement));
    ret_stmt->base.type = NODE_RETURN_STATEMENT;
    ret_stmt->base.next = NULL;
    ret_stmt->return_value = return_value;
    return ret_stmt;
}

ExpressionStatement* expression_statement_new(ASTNode* expression) {
    ExpressionStatement* expr_stmt = (ExpressionStatement*)malloc(sizeof(ExpressionStatement));
    expr_stmt->base.type = NODE_EXPRESSION_STATEMENT;
    expr_stmt->base.next = NULL;
    expr_stmt->expression = expression;
    return expr_stmt;
}

BlockStatement* block_statement_new(ASTNode* statements) {
    BlockStatement* block_stmt = (BlockStatement*)malloc(sizeof(BlockStatement));
    block_stmt->base.type = NODE_BLOCK_STATEMENT;
    block_stmt->base.next = NULL;
    block_stmt->statements = statements;
    return block_stmt;
}

Identifier* identifier_new(char* value) {
    Identifier* ident = (Identifier*)malloc(sizeof(Identifier));
    ident->base.type = NODE_IDENTIFIER;
    ident->base.next = NULL;
    ident->value = strdup(value);
    return ident;
}

NumberLiteral* number_literal_new(char* value) {
    NumberLiteral* num_lit = (NumberLiteral*)malloc(sizeof(NumberLiteral));
    num_lit->base.type = NODE_NUMBER_LITERAL;
    num_lit->base.next = NULL;
    num_lit->value = strdup(value);
    return num_lit;
}

AsciiLiteral* ascii_literal_new(char* value) {
    AsciiLiteral* ascii_lit = (AsciiLiteral*)malloc(sizeof(AsciiLiteral));
    ascii_lit->base.type = NODE_ASCII_LITERAL;
    ascii_lit->base.next = NULL;
    ascii_lit->value = strdup(value);
    return ascii_lit;
}

StringLiteral* string_literal_new(char* value) {
    StringLiteral* str_lit = (StringLiteral*)malloc(sizeof(StringLiteral));
    str_lit->base.type = NODE_STRING_LITERAL;
    str_lit->base.next = NULL;
    str_lit->value = strdup(value);
    return str_lit;
}

AssignExpression* assign_expression_new(ASTNode* name, ASTNode* value) {
    AssignExpression* assign_expr = (AssignExpression*)malloc(sizeof(AssignExpression));
    assign_expr->base.type = NODE_ASSIGN_EXPRESSION;
    assign_expr->base.next = NULL;
    assign_expr->name = name;
    assign_expr->value = value;
    return assign_expr;
}

CallExpression* call_expression_new(ASTNode* function, ASTNode* arguments) {
    CallExpression* call_expr = (CallExpression*)malloc(sizeof(CallExpression));
    call_expr->base.type = NODE_CALL_EXPRESSION;
    call_expr->base.next = NULL;
    call_expr->function = function;
    call_expr->arguments = arguments;
    return call_expr;
}

ForLoop* for_loop_new(ASTNode* init, ASTNode* condition, ASTNode* increment, ASTNode* body) {
    ForLoop* for_loop = (ForLoop*)malloc(sizeof(ForLoop));
    for_loop->base.type = NODE_FOR_LOOP;
    for_loop->base.next = NULL;
    for_loop->init = init;
    for_loop->condition = condition;
    for_loop->increment = increment;
    for_loop->body = body;
    return for_loop;
}

WhileLoop* while_loop_new(ASTNode* condition, ASTNode* body) {
    WhileLoop* while_loop = (WhileLoop*)malloc(sizeof(WhileLoop));
    while_loop->base.type = NODE_WHILE_LOOP;
    while_loop->base.next = NULL;
    while_loop->condition = condition;
    while_loop->body = body;
    return while_loop;
}

ImportStatement* import_statement_new(ImportType import_type, char* path, char* alias, ASTNode* imports) {
    ImportStatement* import_stmt = (ImportStatement*)malloc(sizeof(ImportStatement));
    import_stmt->base.type = NODE_IMPORT_STATEMENT;
    import_stmt->base.next = NULL;
    import_stmt->import_type = import_type;
    import_stmt->path = strdup(path);
    import_stmt->alias = alias ? strdup(alias) : NULL;
    import_stmt->imports = imports;
    return import_stmt;
}

BinaryExpression* binary_expression_new(ASTNode* left, BinaryOperator operator, ASTNode* right) {
    BinaryExpression* bin_expr = (BinaryExpression*)malloc(sizeof(BinaryExpression));
    bin_expr->base.type = NODE_BINARY_EXPRESSION;
    bin_expr->base.next = NULL;
    bin_expr->left = left;
    bin_expr->operator = operator;
    bin_expr->right = right;
    return bin_expr;
}

IndexExpression* index_expression_new(ASTNode* array, ASTNode* index) {
    IndexExpression* index_expr = (IndexExpression*)malloc(sizeof(IndexExpression));
    index_expr->base.type = NODE_INDEX_EXPRESSION;
    index_expr->base.next = NULL;
    index_expr->array = array;
    index_expr->index = index;
    return index_expr;
}

void ast_node_list_free(ASTNode* node_list) {
    ASTNode* current = node_list;
    ASTNode* next;
    while (current != NULL) {
        next = current->next;
        ast_node_free(current); // Free the node itself
        current = next;
    }
}

void ast_node_free(ASTNode* node) {
    if (!node) return;

    // Free members specific to each node type
    // For pointers to other ASTNodes that are NOT part of a list managed by 'next',
    // call ast_node_free.
    // For pointers to ASTNode lists (like statements, parameters),
    // call ast_node_list_free.
    switch (node->type) {
        case NODE_PROGRAM:
            ast_node_list_free(((Program*)node)->statements);
            break;
        case NODE_VAR_DECLARATION:
            free(((VarDeclaration*)node)->name);
            ast_node_free(((VarDeclaration*)node)->size); // size is a single expression
            ast_node_free(((VarDeclaration*)node)->value); // value is a single expression
            break;
        case NODE_FUNCTION_DECLARATION:
            free(((FunctionDeclaration*)node)->name);
            ast_node_list_free(((FunctionDeclaration*)node)->parameters); // parameters is a list
            ast_node_free(((FunctionDeclaration*)node)->body); // body is a single BlockStatement
            break;
        case NODE_RETURN_STATEMENT:
            ast_node_free(((ReturnStatement*)node)->return_value);
            break;
        case NODE_EXPRESSION_STATEMENT:
            ast_node_free(((ExpressionStatement*)node)->expression);
            break;
        case NODE_BLOCK_STATEMENT:
            ast_node_list_free(((BlockStatement*)node)->statements); // statements is a list
            break;
        case NODE_IDENTIFIER:
            free(((Identifier*)node)->value);
            break;
        case NODE_NUMBER_LITERAL:
            free(((NumberLiteral*)node)->value);
            break;
        case NODE_ASCII_LITERAL:
            free(((AsciiLiteral*)node)->value);
            break;
        case NODE_STRING_LITERAL:
            free(((StringLiteral*)node)->value);
            break;
        case NODE_ASSIGN_EXPRESSION:
            // Note: AssignExpression->name could be an Identifier or IndexExpression.
            // It's a single node, not a list.
            ast_node_free(((AssignExpression*)node)->name);
            ast_node_free(((AssignExpression*)node)->value);
            break;
        case NODE_CALL_EXPRESSION:
            ast_node_free(((CallExpression*)node)->function); // function is a single expression (e.g. identifier)
            ast_node_list_free(((CallExpression*)node)->arguments); // arguments is a list of expressions
            break;
        case NODE_FOR_LOOP:
            ast_node_free(((ForLoop*)node)->init);
            ast_node_free(((ForLoop*)node)->condition);
            ast_node_free(((ForLoop*)node)->increment);
            ast_node_free(((ForLoop*)node)->body); // body is a single BlockStatement
            break;
        case NODE_WHILE_LOOP:
            ast_node_free(((WhileLoop*)node)->condition);
            ast_node_free(((WhileLoop*)node)->body); // body is a single BlockStatement
            break;
        case NODE_IMPORT_STATEMENT:
            free(((ImportStatement*)node)->path);
            if (((ImportStatement*)node)->alias) free(((ImportStatement*)node)->alias);
            ast_node_list_free(((ImportStatement*)node)->imports); // imports is a list of identifiers
            break;
        case NODE_BINARY_EXPRESSION:
            ast_node_free(((BinaryExpression*)node)->left);
            ast_node_free(((BinaryExpression*)node)->right);
            break;
        case NODE_INDEX_EXPRESSION:
            ast_node_free(((IndexExpression*)node)->array);
            ast_node_free(((IndexExpression*)node)->index);
            break;
        // No default case needed as all node types should be handled.
        // If a new node type is added, it must be added here.
    }

    // Free the node itself. The 'next' pointer is handled by ast_node_list_free
    // for nodes that are part of a list. Individual nodes not in lists will have 'next' as NULL
    // or it's managed by their parent list structure.
    free(node);
}


