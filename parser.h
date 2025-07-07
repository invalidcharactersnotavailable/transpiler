#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token current_token;
    Token peek_token;
} Parser;

Parser* parser_new(Lexer* lexer);
void parser_free(Parser* parser);
Program* parse_program(Parser* parser);

#endif // PARSER_H


