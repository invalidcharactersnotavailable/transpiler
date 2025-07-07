#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int label_count = 0;

static void generate_expression(ASTNode* node, FILE* output_file);
static void generate_statement(ASTNode* node, FILE* output_file);

static void generate_label(FILE* output_file, const char* prefix) {
    fprintf(output_file, "%s%d:\n", prefix, label_count++);
}

static void generate_var_declaration(VarDeclaration* var_decl, FILE* output_file) {
    fprintf(output_file, "; Variable Declaration: %s\n", var_decl->name);
    // For simplicity, assume all variables are 8-byte (QWORD) for now
    // In a real transpiler, type inference would determine size
    fprintf(output_file, "section .data\n");
    fprintf(output_file, "%s: dq 0\n", var_decl->name);

    if (var_decl->value) {
        fprintf(output_file, "section .text\n");
        fprintf(output_file, "global _start\n"); // For simple programs
        fprintf(output_file, "_start:\n");
        generate_expression(var_decl->value, output_file);
        fprintf(output_file, "  pop rax\n");
        fprintf(output_file, "  mov [rel %s], rax\n", var_decl->name);
    }
}

static void generate_function_declaration(FunctionDeclaration* func_decl, FILE* output_file) {
    fprintf(output_file, "; Function Declaration: %s\n", func_decl->name);
    fprintf(output_file, "section .text\n");
    fprintf(output_file, "global %s\n", func_decl->name);
    fprintf(output_file, "%s:\n", func_decl->name);

    // Function prologue
    fprintf(output_file, "  push rbp\n");
    fprintf(output_file, "  mov rbp, rsp\n");

    // Handle parameters (for now, just a placeholder)
    // In x86_64, parameters are passed in registers (RDI, RSI, RDX, RCX, R8, R9) then stack

    if (func_decl->body) {
        BlockStatement* block = (BlockStatement*)func_decl->body;
        ASTNode* current_stmt = block->statements;
        while (current_stmt) {
            generate_statement(current_stmt, output_file);
            current_stmt = current_stmt->next;
        }
    }

    // Function epilogue (if no explicit return)
    fprintf(output_file, "  mov rsp, rbp\n");
    fprintf(output_file, "  pop rbp\n");
    fprintf(output_file, "  ret\n");
}

static void generate_return_statement(ReturnStatement* ret_stmt, FILE* output_file) {
    fprintf(output_file, "; Return Statement\n");
    if (ret_stmt->return_value) {
        generate_expression(ret_stmt->return_value, output_file);
        fprintf(output_file, "  pop rax\n"); // Return value in RAX
    }
    fprintf(output_file, "  mov rsp, rbp\n");
    fprintf(output_file, "  pop rbp\n");
    fprintf(output_file, "  ret\n");
}

static void generate_expression_statement(ExpressionStatement* expr_stmt, FILE* output_file) {
    fprintf(output_file, "; Expression Statement\n");
    generate_expression(expr_stmt->expression, output_file);
}

static void generate_block_statement(BlockStatement* block_stmt, FILE* output_file) {
    fprintf(output_file, "; Block Statement\n");
    ASTNode* current_stmt = block_stmt->statements;
    while (current_stmt) {
        generate_statement(current_stmt, output_file);
        current_stmt = current_stmt->next;
    }
}

static void generate_identifier(Identifier* ident, FILE* output_file) {
    fprintf(output_file, "; Identifier: %s\n", ident->value);
    // For now, assume identifiers are variables and load their value
    fprintf(output_file, "  push qword [rel %s]\n", ident->value);
}

static void generate_number_literal(NumberLiteral* num_lit, FILE* output_file) {
    fprintf(output_file, "; Number Literal: %s\n", num_lit->value);
    fprintf(output_file, "  push %s\n", num_lit->value);
}

static void generate_ascii_literal(AsciiLiteral* ascii_lit, FILE* output_file) {
    fprintf(output_file, "; ASCII Literal: %s\n", ascii_lit->value);
    char* val_str = strdup(ascii_lit->value);
    val_str[strlen(val_str) - 1] = '\0'; // Remove the 'a'
    int ascii_val = atoi(val_str);
    free(val_str);
    fprintf(output_file, "  push %d\n", ascii_val);
}

static void generate_string_literal(StringLiteral* str_lit, FILE* output_file) {
    fprintf(output_file, "; String Literal: %s\n", str_lit->value);
    // For now, just push the address of the string data
    // In a real transpiler, this would involve defining the string in .data section
    fprintf(output_file, "section .data\n");
    fprintf(output_file, "str_%d: db \"%s\", 0\n", label_count, str_lit->value);
    fprintf(output_file, "section .text\n");
    fprintf(output_file, "  push str_%d\n", label_count++);
}

static void generate_assign_expression(AssignExpression* assign_expr, FILE* output_file) {
    fprintf(output_file, "; Assignment Expression\n");
    generate_expression(assign_expr->value, output_file);
    fprintf(output_file, "  pop rax\n");
    // Assuming assignment to an identifier (variable)
    if (assign_expr->name->type == NODE_IDENTIFIER) {
        Identifier* ident = (Identifier*)assign_expr->name;
        fprintf(output_file, "  mov [rel %s], rax\n", ident->value);
    } else if (assign_expr->name->type == NODE_INDEX_EXPRESSION) {
        IndexExpression* index_expr = (IndexExpression*)assign_expr->name;
        generate_expression(index_expr->index, output_file);
        fprintf(output_file, "  pop rbx\n"); // index
        // Assuming array is an identifier
        Identifier* array_ident = (Identifier*)index_expr->array;
        fprintf(output_file, "  mov [rel %s + rbx*8], rax\n", array_ident->value); // Assuming 8-byte elements
    }
}

static void generate_call_expression(CallExpression* call_expr, FILE* output_file) {
    fprintf(output_file, "; Call Expression\n");
    // Push arguments onto stack or into registers (x86_64 calling convention)
    // For simplicity, assume no arguments for now, or push them onto stack in reverse order
    // This needs proper argument handling based on x86_64 calling convention

    if (call_expr->function->type == NODE_IDENTIFIER) {
        Identifier* func_ident = (Identifier*)call_expr->function;
        fprintf(output_file, "  call %s\n", func_ident->value);
        fprintf(output_file, "  push rax\n"); // Push return value (RAX) onto stack
    }
}

static void generate_for_loop(ForLoop* for_loop, FILE* output_file) {
    fprintf(output_file, "; For Loop\n");
    int loop_label = label_count++;
    int end_label = label_count++;

    // Initialization
    if (for_loop->init) {
        generate_expression(for_loop->init, output_file);
        fprintf(output_file, "  pop rax\n"); // Consume result of init expression
    }

    fprintf(output_file, "_for_loop_%d:\n", loop_label);

    // Condition
    if (for_loop->condition) {
        generate_expression(for_loop->condition, output_file);
        fprintf(output_file, "  pop rax\n");
        fprintf(output_file, "  cmp rax, 0\n"); // Compare with 0 (false)
        fprintf(output_file, "  je _for_end_%d\n", end_label);
    }

    // Body
    if (for_loop->body) {
        generate_block_statement((BlockStatement*)for_loop->body, output_file);
    }

    // Increment
    if (for_loop->increment) {
        generate_expression(for_loop->increment, output_file);
        fprintf(output_file, "  pop rax\n"); // Consume result of increment expression
    }

    fprintf(output_file, "  jmp _for_loop_%d\n", loop_label);
    fprintf(output_file, "_for_end_%d:\n", end_label);
}

static void generate_while_loop(WhileLoop* while_loop, FILE* output_file) {
    fprintf(output_file, "; While Loop\n");
    int loop_label = label_count++;
    int end_label = label_count++;

    fprintf(output_file, "_while_loop_%d:\n", loop_label);

    // Condition
    if (while_loop->condition) {
        generate_expression(while_loop->condition, output_file);
        fprintf(output_file, "  pop rax\n");
        fprintf(output_file, "  cmp rax, 0\n"); // Compare with 0 (false)
        fprintf(output_file, "  je _while_end_%d\n", end_label);
    }

    // Body
    if (while_loop->body) {
        generate_block_statement((BlockStatement*)while_loop->body, output_file);
    }

    fprintf(output_file, "  jmp _while_loop_%d\n", loop_label);
    fprintf(output_file, "_while_end_%d:\n", end_label);
}

static void generate_import_statement(ImportStatement* import_stmt, FILE* output_file) {
    fprintf(output_file, "; Import Statement: %s\n", import_stmt->path);
    // For now, imports are not directly translated to assembly.
    // In a full transpiler, this would involve linking with external object files
    // or inlining code from imported modules.
}

static void generate_binary_expression(BinaryExpression* bin_expr, FILE* output_file) {
    fprintf(output_file, "; Binary Expression\n");
    generate_expression(bin_expr->left, output_file);
    generate_expression(bin_expr->right, output_file);

    fprintf(output_file, "  pop rbx\n"); // Right operand
    fprintf(output_file, "  pop rax\n"); // Left operand

    switch (bin_expr->operator) {
        case BIN_OP_PLUS:
            fprintf(output_file, "  add rax, rbx\n");
            break;
        case BIN_OP_MINUS:
            fprintf(output_file, "  sub rax, rbx\n");
            break;
        case BIN_OP_MULTIPLY:
            fprintf(output_file, "  imul rax, rbx\n");
            break;
        case BIN_OP_DIVIDE:
            fprintf(output_file, "  xor rdx, rdx\n"); // Clear RDX for division
            fprintf(output_file, "  idiv rbx\n");
            break;
        case BIN_OP_MODULO:
            fprintf(output_file, "  xor rdx, rdx\n"); // Clear RDX for division
            fprintf(output_file, "  idiv rbx\n");
            fprintf(output_file, "  mov rax, rdx\n"); // Remainder is in RDX
            break;
        case BIN_OP_EQ:
            fprintf(output_file, "  cmp rax, rbx\n");
            fprintf(output_file, "  sete al\n"); // Set AL to 1 if equal, 0 otherwise
            fprintf(output_file, "  movzx rax, al\n"); // Zero-extend AL to RAX
            break;
        case BIN_OP_NEQ:
            fprintf(output_file, "  cmp rax, rbx\n");
            fprintf(output_file, "  setne al\n");
            fprintf(output_file, "  movzx rax, al\n");
            break;
        case BIN_OP_LT:
            fprintf(output_file, "  cmp rax, rbx\n");
            fprintf(output_file, "  setl al\n");
            fprintf(output_file, "  movzx rax, al\n");
            break;
        case BIN_OP_GT:
            fprintf(output_file, "  cmp rax, rbx\n");
            fprintf(output_file, "  setg al\n");
            fprintf(output_file, "  movzx rax, al\n");
            break;
        case BIN_OP_LE:
            fprintf(output_file, "  cmp rax, rbx\n");
            fprintf(output_file, "  setle al\n");
            fprintf(output_file, "  movzx rax, al\n");
            break;
        case BIN_OP_GE:
            fprintf(output_file, "  cmp rax, rbx\n");
            fprintf(output_file, "  setge al\n");
            fprintf(output_file, "  movzx rax, al\n");
            break;
    }
    fprintf(output_file, "  push rax\n");
}

static void generate_index_expression(IndexExpression* index_expr, FILE* output_file) {
    fprintf(output_file, "; Index Expression\n");
    generate_expression(index_expr->index, output_file);
    fprintf(output_file, "  pop rbx\n"); // index
    // Assuming array is an identifier
    Identifier* array_ident = (Identifier*)index_expr->array;
    fprintf(output_file, "  push qword [rel %s + rbx*8]\n", array_ident->value); // Assuming 8-byte elements
}

static void generate_expression(ASTNode* node, FILE* output_file) {
    if (!node) return;

    switch (node->type) {
        case NODE_IDENTIFIER:
            generate_identifier((Identifier*)node, output_file);
            break;
        case NODE_NUMBER_LITERAL:
            generate_number_literal((NumberLiteral*)node, output_file);
            break;
        case NODE_ASCII_LITERAL:
            generate_ascii_literal((AsciiLiteral*)node, output_file);
            break;
        case NODE_STRING_LITERAL:
            generate_string_literal((StringLiteral*)node, output_file);
            break;
        case NODE_ASSIGN_EXPRESSION:
            generate_assign_expression((AssignExpression*)node, output_file);
            break;
        case NODE_CALL_EXPRESSION:
            generate_call_expression((CallExpression*)node, output_file);
            break;
        case NODE_BINARY_EXPRESSION:
            generate_binary_expression((BinaryExpression*)node, output_file);
            break;
        case NODE_INDEX_EXPRESSION:
            generate_index_expression((IndexExpression*)node, output_file);
            break;
        default:
            fprintf(stderr, "Error: Unknown expression node type %d\n", node->type);
            exit(1);
    }
}

static void generate_statement(ASTNode* node, FILE* output_file) {
    if (!node) return;

    switch (node->type) {
        case NODE_VAR_DECLARATION:
            generate_var_declaration((VarDeclaration*)node, output_file);
            break;
        case NODE_FUNCTION_DECLARATION:
            generate_function_declaration((FunctionDeclaration*)node, output_file);
            break;
        case NODE_RETURN_STATEMENT:
            generate_return_statement((ReturnStatement*)node, output_file);
            break;
        case NODE_EXPRESSION_STATEMENT:
            generate_expression_statement((ExpressionStatement*)node, output_file);
            break;
        case NODE_BLOCK_STATEMENT:
            generate_block_statement((BlockStatement*)node, output_file);
            break;
        case NODE_FOR_LOOP:
            generate_for_loop((ForLoop*)node, output_file);
            break;
        case NODE_WHILE_LOOP:
            generate_while_loop((WhileLoop*)node, output_file);
            break;
        case NODE_IMPORT_STATEMENT:
            generate_import_statement((ImportStatement*)node, output_file);
            break;
        default:
            fprintf(stderr, "Error: Unknown statement node type %d\n", node->type);
            exit(1);
    }
}

void generate_assembly(Program* program, FILE* output_file) {
    fprintf(output_file, "; Transpiled Assembly Code\n");
    fprintf(output_file, "section .text\n");
    fprintf(output_file, "global _start\n");
    fprintf(output_file, "_start:\n");

    ASTNode* current_stmt = program->statements;
    while (current_stmt) {
        generate_statement(current_stmt, output_file);
        current_stmt = current_stmt->next;
    }

    // Exit system call (for simple programs)
    fprintf(output_file, "  mov rax, 60  ; syscall number for exit\n");
    fprintf(output_file, "  xor rdi, rdi ; exit code 0\n");
    fprintf(output_file, "  syscall\n");
}


