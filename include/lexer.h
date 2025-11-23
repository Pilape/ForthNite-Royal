#ifndef LEXER_HEADER
#define LEXER_HEADER

#include <stdlib.h>

#define LEXEME_MAX_LENGTH 32

typedef enum {
    FUNC_START,
    FUNC_END,

    NUM_DEC,
    NUM_HEX,
    NUM_OCT,
    NUM_BIN,

    LOOP_START,
    LOOP_UNTIL,
    LOOP_WHILE,
    LOOP_AGAIN,
    LOOP_LEAVE,

    IF_START,
    IF_THEN,
    IF_ELSE,

    WORD,
} TokenType;

typedef struct {
    unsigned int line;
    TokenType type;
    char lexeme[LEXEME_MAX_LENGTH+1];
} Token;

typedef struct {
    size_t length;
    size_t capacity;
    Token* data;
} TokenList;

TokenList Scan(char* program);

#endif 
