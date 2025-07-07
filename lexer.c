#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static Token create_token(TokenType type, const char* value, int length, int line, int column) {
    Token token;
    token.type = type;
    token.value = (char*)malloc(length + 1);
    strncpy(token.value, value, length);
    token.value[length] = '\0';
    token.line = line;
    token.column = column;
    return token;
}

static void advance(Lexer* lexer) {
    if (lexer->source[lexer->position] == '\n') {
        lexer->line++;
        lexer->column = 0;
    } else {
        lexer->column++;
    }
    lexer->position++;
}

static char peek(Lexer* lexer) {
    return lexer->source[lexer->position];
}

static char peek_next(Lexer* lexer) {
    if (lexer->position + 1 < strlen(lexer->source)) {
        return lexer->source[lexer->position + 1];
    }
    return '\0';
}

static void skip_whitespace(Lexer* lexer) {
    while (isspace(peek(lexer))) {
        advance(lexer);
    }
}

Lexer* lexer_new(const char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 0;
    lexer->current_token.type = TOKEN_EOF; // Initialize to EOF
    lexer->current_token.value = NULL;
    return lexer;
}

void lexer_free(Lexer* lexer) {
    if (lexer->current_token.value) {
        free(lexer->current_token.value);
    }
    free(lexer);
}

void token_free(Token* token) {
    if (token->value) {
        free(token->value);
        token->value = NULL;
    }
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace(lexer);

    if (peek(lexer) == '\0') {
        return create_token(TOKEN_EOF, "", 0, lexer->line, lexer->column);
    }

    int start_pos = lexer->position;
    int start_column = lexer->column;

    char current_char = peek(lexer);

    if (isalpha(current_char) || current_char == '_') {
        while (isalnum(peek(lexer)) || peek(lexer) == '_') {
            advance(lexer);
        }
        int length = lexer->position - start_pos;
        const char* value = lexer->source + start_pos;
        if (strncmp(value, "return", length) == 0) return create_token(TOKEN_KEYWORD_RETURN, value, length, lexer->line, start_column);
        if (strncmp(value, "for", length) == 0) return create_token(TOKEN_KEYWORD_FOR, value, length, lexer->line, start_column);
        if (strncmp(value, "while", length) == 0) return create_token(TOKEN_KEYWORD_WHILE, value, length, lexer->line, start_column);
        if (strncmp(value, "import", length) == 0) return create_token(TOKEN_KEYWORD_IMPORT, value, length, lexer->line, start_column);
        if (strncmp(value, "as", length) == 0) return create_token(TOKEN_KEYWORD_AS, value, length, lexer->line, start_column);
        if (strncmp(value, "from", length) == 0) return create_token(TOKEN_KEYWORD_FROM, value, length, lexer->line, start_column);
        if (strncmp(value, "func", length) == 0) return create_token(TOKEN_KEYWORD_FUNC, value, length, lexer->line, start_column);
        if (strncmp(value, "var", length) == 0) return create_token(TOKEN_KEYWORD_VAR, value, length, lexer->line, start_column);
        return create_token(TOKEN_IDENTIFIER, value, length, lexer->line, start_column);
    }

    if (isdigit(current_char)) {
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
        if (peek(lexer) == 'a' && (peek_next(lexer) == ' ' || peek_next(lexer) == '\n' || peek_next(lexer) == '\0' || !isalnum(peek_next(lexer)))) {
            advance(lexer); // consume 'a'
            return create_token(TOKEN_ASCII_LITERAL, lexer->source + start_pos, lexer->position - start_pos, lexer->line, start_column);
        }
        return create_token(TOKEN_NUMBER, lexer->source + start_pos, lexer->position - start_pos, lexer->line, start_column);
    }

    if (current_char == '"') {
        advance(lexer); // consume '"'
        start_pos = lexer->position;
        while (peek(lexer) != '"' && peek(lexer) != '\0') {
            advance(lexer);
        }
        if (peek(lexer) == '\0') {
            fprintf(stderr, "Error: Unterminated string literal at line %d, column %d\n", lexer->line, start_column);
            exit(1);
        }
        Token token = create_token(TOKEN_STRING_LITERAL, lexer->source + start_pos, lexer->position - start_pos, lexer->line, start_column);
        advance(lexer); // consume '"'
        return token;
    }

    advance(lexer);
    switch (current_char) {
        case '=': 
            if (peek(lexer) == '=') {
                advance(lexer);
                return create_token(TOKEN_EQ, "==", 2, lexer->line, start_column);
            }
            return create_token(TOKEN_ASSIGN, "=", 1, lexer->line, start_column);
        case '!':
            if (peek(lexer) == '=') {
                advance(lexer);
                return create_token(TOKEN_NEQ, "!=", 2, lexer->line, start_column);
            }
            break; // Handle error or single '!' if needed
        case '<':
            if (peek(lexer) == '=') {
                advance(lexer);
                return create_token(TOKEN_LE, "<=", 2, lexer->line, start_column);
            }
            return create_token(TOKEN_LT, "<", 1, lexer->line, start_column);
        case '>':
            if (peek(lexer) == '=') {
                advance(lexer);
                return create_token(TOKEN_GE, ">=", 2, lexer->line, start_column);
            }
            return create_token(TOKEN_GT, ">", 1, lexer->line, start_column);
        case '(': return create_token(TOKEN_LPAREN, "(", 1, lexer->line, start_column);
        case ')': return create_token(TOKEN_RPAREN, ")", 1, lexer->line, start_column);
        case '{': return create_token(TOKEN_LBRACE, "{", 1, lexer->line, start_column);
        case '}': return create_token(TOKEN_RBRACE, "}", 1, lexer->line, start_column);
        case '[': return create_token(TOKEN_LBRACKET, "[", 1, lexer->line, start_column);
        case ']': return create_token(TOKEN_RBRACKET, "]", 1, lexer->line, start_column);
        case ',': return create_token(TOKEN_COMMA, ",", 1, lexer->line, start_column);
        case ';': return create_token(TOKEN_SEMICOLON, ";", 1, lexer->line, start_column);
        case '.': return create_token(TOKEN_DOT, ".", 1, lexer->line, start_column);
        case '+': return create_token(TOKEN_PLUS, "+", 1, lexer->line, start_column);
        case '-': return create_token(TOKEN_MINUS, "-", 1, lexer->line, start_column);
        case '*': return create_token(TOKEN_MULTIPLY, "*", 1, lexer->line, start_column);
        case '/': return create_token(TOKEN_DIVIDE, "/", 1, lexer->line, start_column);
        case '%': return create_token(TOKEN_MODULO, "%", 1, lexer->line, start_column);
    }

    fprintf(stderr, "Error: Unexpected character '%c' at line %d, column %d\n", current_char, lexer->line, start_column);
    exit(1);
}


