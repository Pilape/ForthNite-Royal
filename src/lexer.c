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
    /*struct {
        unsigned int line;
        char* start;
        char* current;
    } scanner;

    scanner.line = 1;
    scanner.start = program;
    scanner.current = program;*/

    TokenList tokens = TokenListCreate(16);

    size_t program_length = strlen(program);

    unsigned int line = 1;
    size_t start = 0;

    bool in_comment = false;
    bool has_errored = false;

    for (size_t current=0; current<program_length; current++) {
        switch (program[current]) {
            case '(':
                in_comment = true;
                continue;
            case ')':
                in_comment = false;
                start = current+1;
                continue;

            case '\n':
                line++;
            case ' ':
            case '\t':
                // Create token

                // Unless we're in a comment
                if (in_comment) {
                    start = current+1;
                    continue;
                }
                
                unsigned int token_length = current - start;

                if (token_length == 0) {
                    start = current+1;
                    continue;
                }

                char lexeme[token_length+1];
                for (int i=0; i<token_length; i++) {
                    lexeme[i] = program[start+i];
                }
                lexeme[token_length] = '\0';
                if (token_length > LEXEME_MAX_LENGTH+1) { 
                    printf("[ERROR]: Word: '%s' at line %d exceeds max word length of 32 characters\n", lexeme, line);
                    has_errored = true;
                    start = current+1;
                    continue;
                }

                printf("length: %d, line: %d, lexeme: %s\n", token_length, line, lexeme);
                start = current+1;
                break;
        }
    }

    if (has_errored) exit(-1);
    return tokens;
}
