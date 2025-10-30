#include "../include/lexer.h"
#include "../include/compiler.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static TokenList TokenListCreate(unsigned int start_capacity) {
    TokenList new_list = {
        .capacity = start_capacity,
        .length = 0,
    };

    new_list.data = malloc(start_capacity*sizeof(Token));
    ASSERT(malloc != NULL, "Failed to allocate memory for token list");

    return new_list;
}

static void AddToken(char lexeme[LEXEME_MAX_LENGTH], TokenType type, TokenList* list) {
    if (list->length >= list->capacity) {
        list->capacity *= 2;
        ASSERT(realloc(list->data, list->capacity), "Failed to reallocate memory for token list");
    }

    Token new_token;
    new_token.type = type;
    strcpy(new_token.lexeme, lexeme);
    list->data[list->length] = new_token;

    list->length++;
}

TokenList Scan(char* program) {
    TokenList tokens = TokenListCreate(16);
    bool has_errored = true;

    struct {
        unsigned int line;
        char* start;
        char* current;
    } scanner;

    scanner.line = 1;
    scanner.start = program;
    scanner.current = program;

    return tokens;
}
