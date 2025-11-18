#include "../include/lexer.h"
#include "../include/compiler.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static TokenList TokenListCreate(unsigned int start_capacity) {
    TokenList new_list = {
        .capacity = start_capacity,
        .length = 0,
    };

    new_list.data = malloc(start_capacity*sizeof(Token));
    ASSERT(new_list.data != NULL, "Failed to allocate memory for token list");

    return new_list;
}

static void AddToken(char lexeme[LEXEME_MAX_LENGTH+1], TokenType type, unsigned int line, TokenList* list) {
    if (list->length >= list->capacity) {
        list->capacity *= 2;
        list->data = realloc(list->data, list->capacity*sizeof(Token));
        ASSERT(list->data != NULL, "Failed to reallocate memory for token list");
    }

    Token new_token;
    new_token.type = type;
    new_token.line = line;
    strcpy(new_token.lexeme, lexeme);
    list->data[list->length] = new_token;

    list->length++;
}

static void FetchLexemes(char* program, TokenList* tokens) { 
    size_t program_length = strlen(program);

    unsigned int line = 1;
    unsigned int next_line = 1; // Because defer doesn't exist in C
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
                next_line++;
            case ' ':
            case '\t':
                // Create token

                // Unless we're in a comment
                if (in_comment) {
                    start = current+1;
                    line = next_line;
                    continue;
                }
                
                unsigned int token_length = current - start;

                if (token_length == 0) {
                    start = current+1;
                    line = next_line;
                    continue;
                }

                char lexeme[token_length+1];
                for (int i=0; i<token_length; i++) {
                    lexeme[i] = tolower(program[start+i]);
                }
                lexeme[token_length] = '\0';
                if (token_length > LEXEME_MAX_LENGTH+1) { 
                    printf("[ERROR]: Word: '%s' at line %d exceeds max word length of 32 characters\n", lexeme, line);
                    has_errored = true;
                    start = current+1;
                    line = next_line;
                    continue;
                }

                AddToken(lexeme, -1, line, tokens); // We don't know the type yet
                
                start = current+1;
                line = next_line;
                break;
        }
    }

    if (has_errored) exit(-1); 
}

static bool StrIsDecimal(char* str) {
    size_t str_length = strlen(str);
    for (int i=0; i<str_length; i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

static bool StrIsHex(char* str) {
    size_t str_length = strlen(str);
    for (int i=2; /* skip '0x' prefix */ i<str_length; i++) {
        if (!isdigit(str[i])) {
            if (str[i] < 'a' || str[i] > 'f') return false;
        }
    }
    return true;
}

static bool StrIsBin(char* str) {
    size_t str_length = strlen(str);
    for (int i=2; /* skip '0x' prefix */ i<str_length; i++) {
        if (str[i] != '0' && str[i] != '1') return false;
    }
    return true;
}

static bool StrIsOct(char* str) {
    size_t str_length = strlen(str);
    for (int i=2; /* skip '0x' prefix */ i<str_length; i++) {
        if (str[i] < '0' || str[i] > '7') return false;
    }
    return true;
}

static void AssignTypes(TokenList* tokens) {
    bool has_errored = false;

    for (int i=0; i<tokens->length; i++) {
        Token* token = &tokens->data[i];

        if (strcmp(token->lexeme, ":") == 0) {
            token->type = FUNC_START;
            continue;
        }

        if (strcmp(token->lexeme, ";") == 0) {
            token->type = FUNC_END;
            continue;
        }

        // Is it a base-10 number??
        if (StrIsDecimal(token->lexeme)) {
            token->type = NUM_DEC;
            continue;
        }

        // Other number formats (hex, binary, octal)
        if (token->lexeme[0] == '0') {
            switch (token->lexeme[1]) {
                // hex
                case 'x': 
                    if (StrIsHex(token->lexeme)) {
                        token->type = NUM_HEX;
                    } else {
                        printf("[ERROR]: '%s' at line %d is not a valid hexadecimal number\n", token->lexeme, token->line);
                        has_errored = true;
                    }
                    continue;

                // binary
                case 'b': 
                    if (StrIsBin(token->lexeme)) {
                        token->type = NUM_BIN;
                    } else {
                        printf("[ERROR]: '%s' at line %d is not a valid binary number\n", token->lexeme, token->line);
                        has_errored = true;
                    }
                    continue;

                // octal
                case 'o': 
                    if (StrIsOct(token->lexeme)) {
                        token->type = NUM_OCT;
                    } else {
                        printf("[ERROR]: '%s' at line %d is not a valid octal number\n", token->lexeme, token->line);
                        has_errored = true;
                    }
                    continue;
            
            }

        }
        // Default
        token->type = WORD;
    }
    if (has_errored) exit(-1);
}

TokenList Scan(char* program) {
    TokenList tokens = TokenListCreate(16);
    FetchLexemes(program, &tokens);
    AssignTypes(&tokens);

    for (int i=0; i<tokens.length; i++) {
        printf("| Line: %2d | Lexeme: %31s | Type: %d |\n", tokens.data[i].line, tokens.data[i].lexeme, tokens.data[i].type);
    }
    return tokens;
}
