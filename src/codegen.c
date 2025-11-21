#include "../include/codegen.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

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

void GenerateCode(const TokenList* src, Rom* dest) {

    enum {
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
                }
                in_word_definition = true;
                break;

            case FUNC_END:
                if (!in_word_definition) {
                    printf("[ERROR]: ';' found outside of word definition at line %d\n", token.line);
                    has_errored = true;
                }
                in_word_definition = false;
                break;

            case NUM_BIN:
                break;
            case NUM_DEC:
                break;
            case NUM_OCT:
                break;
            case NUM_HEX:
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

