#ifndef LEXER_HEADER
#define LEXER_HEADER

#include <stdlib.h>

#define LEXEME_MAX_LENGTH 32

typedef enum {


} TokenType;

typedef struct {
    unsigned int line;
    TokenType type;
    char lexeme[LEXEME_MAX_LENGTH-1];
} Token;

typedef struct {
    size_t length;
    size_t capacity;
    Token* data;
} TokenList;

#endif 
