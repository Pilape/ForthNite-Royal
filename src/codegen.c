#include "../include/codegen.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define ARR_LEN(arr) (sizeof(arr)/sizeof(arr[0]))

typedef enum {
    IF_STATEMENT, LOOP,
} ControlFlowType;

typedef struct {
    ControlFlowType type;
    uint16_t address;
} ControlFlowLabel;

typedef struct {
    ControlFlowLabel data[0xFFFF];
    int ptr;
} ControlFlowStack;

static inline void ControlFlowStackPush(ControlFlowStack* stack, ControlFlowType type, uint16_t address) {
    stack->data[stack->ptr] = (ControlFlowLabel){.type = type, .address = address};
    stack->ptr++;
}

static inline ControlFlowLabel ControlFlowStackPop(ControlFlowStack* stack) {
    stack->ptr--;
    return stack->data[stack->ptr];
}

static inline ControlFlowLabel ControlFlowStackPeek(ControlFlowStack* stack) {
    return stack->data[stack->ptr-1];
}

typedef enum {
    NOP,    HALT,
    PUSH,   DUP,
    OVER,   POP,
    NIP,    SWAP,
    ROT,    LOAD,
    STORE,  LOADb,
    STOREb, ADD,
    SUB,    ADDc,
    SUBc,   SHL,
    SHR,    bNAND,
    NAND,   EQUAL,
    MORE,   LESS,
    JUMP,   BRANCH,
    BIF0,   BIFN0,
    CALL,   RET
} Instruction;

static inline void EmitByte(Rom* dest, uint8_t byte) {
    if (dest->size >= ROM_SIZE_MAX) {
        printf("[ERROR]: Program size exceeds rom size limit\n");
    }
    dest->data[dest->size++] = byte;
}

static inline void EmitWord(Rom* dest, uint16_t word) {
    if (dest->size+1 >= ROM_SIZE_MAX) {
        printf("[ERROR]: Program size exceeds rom size limit\n");
    }
    dest->data[dest->size++] = (word & 0xFF00) >> 8;
    dest->data[dest->size++] = word & 0xFF;
}

static inline bool StringInArr(char* str, char* arr[], size_t arr_len) {
    for (int i=0; i<arr_len; i++) {
        if (strcmp(str, arr[i]) == 0) return true;
    }
    return false;
}

static inline uint8_t StringIndex(char* str, char* arr[], size_t arr_len) {
    for (int i=0; i<arr_len; i++) {
        if (strcmp(str, arr[i]) == 0) return i;
    }
    return 0;
}

// We assume the string is a valid octal number since the compiler checks it in the scanning stage
static inline uint32_t StringToOct(char* str) {
    uint32_t octal_number = 0;

    int str_length = strlen(str);
    int j = 0;
    for (int i=str_length-1; i>=2;/* Ignore prefix */ i--) {
        int digit = str[i] - 48;
        octal_number += digit * pow(8, j);
        j++;
    }
    return octal_number;
}

static inline void AssertIntLimit(uint32_t num, Token token, bool* has_errored) {
    if (num > 0xFFFF) {
        printf("[ERROR]: The number: '%s' at line %d is too large (exceeds 16-bit int limit)\n", token.lexeme, token.line);
        *has_errored = true;
    }
}

static inline void EmitPushNum(Rom* dest, Token token, char* format, bool* has_errored) {
    EmitByte(dest, PUSH);

    uint32_t number = 0;
    sscanf(token.lexeme, format, &number);
    AssertIntLimit(number, token, has_errored);

    EmitWord(dest, (uint16_t)number);
}

void GenerateCode(const TokenList* src, Rom* dest) {
    // Primitives are words that are defined in the language itself
    // They can be overwritten by code, however it will cause a warning. TODO: Add a flag to hide warnings
    char* instruction_primitives[] = {
        "nop",    "halt",
        "push",   "dup",
        "over",   "pop",
        "nip",    "swap",
        "rot",    "load",
        "store",  "loadb",
        "storeb", "+",
        "-",      "addc",
        "subc",   "shl",
        "shr",    "bnand",
        "nand",   "=",
        ">",      "<",
    };

    char* control_flow_primitives[] = {
        "if", "else", "then",
        "do", "until", "while", 
        "again", "leave",
    };

    bool has_errored = false;
    bool in_word_definition = false;
    for (int i=0; i<src->length; i++) {
        Token token = src->data[i];

        switch (token.type) {
            case FUNC_START:
                if (in_word_definition) {
                    printf("[ERROR]: ':' found inside of word definition at line %d\n", token.line);
                    has_errored = true;
                } else {
                    in_word_definition = true;
                }
                break;

            case FUNC_END:
                if (!in_word_definition) {
                    printf("[ERROR]: ';' found outside of word definition at line %d\n", token.line);
                    has_errored = true;
                } else {
                    EmitByte(dest, RET);
                    in_word_definition = false;
                }
                break;

            case NUM_BIN:
                // Address sanitizer gives a warning, but it works fine so just ignore it
                EmitPushNum(dest, token, "%b", &has_errored);
                break;

            case NUM_DEC:
                EmitPushNum(dest, token, "%d", &has_errored);
                break;

            case NUM_OCT: {
                EmitByte(dest, PUSH);
                uint32_t number = StringToOct(token.lexeme);
                AssertIntLimit(number, token, &has_errored);
                EmitWord(dest, (uint16_t)number);
                break;
            }

            case NUM_HEX: 
                EmitPushNum(dest, token, "%x", &has_errored);
                break;

            case WORD:
                if (StringInArr(token.lexeme, instruction_primitives, ARR_LEN(instruction_primitives))) {
                    EmitByte(dest, StringIndex(token.lexeme, instruction_primitives, ARR_LEN(instruction_primitives)));
                    break;
                }

                if (StringInArr(token.lexeme, control_flow_primitives, ARR_LEN(control_flow_primitives))) {
                    printf("control flow: %s\n", token.lexeme);
                    break;
                }
               break;
                
        
        }
    }
}

