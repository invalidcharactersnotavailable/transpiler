#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file.manu>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("Error opening input file");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* source = malloc(fsize + 1);
    fread(source, 1, fsize, fp);
    fclose(fp);
    source[fsize] = 0;

    Lexer* lexer = lexer_new(source);
    Parser* parser = parser_new(lexer);
    Program* program = parse_program(parser);

    FILE* output_fp = fopen("output.asm", "w");
    if (output_fp == NULL) {
        perror("Error opening output file");
        lexer_free(lexer);
        parser_free(parser);
        ast_node_free((ASTNode*)program);
        free(source);
        return 1;
    }

    generate_assembly(program, output_fp);

    fclose(output_fp);

    lexer_free(lexer);
    parser_free(parser);
    ast_node_free((ASTNode*)program);
    free(source);

    printf("Transpilation successful! Assembly code written to output.asm\n");

    return 0;
}


