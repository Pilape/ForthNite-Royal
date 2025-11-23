#include "../include/codegen.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define ARR_LEN(arr) (sizeof(arr)/sizeof(arr[0]))

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

typedef struct {
    struct {
        char name[LEXEME_MAX_LENGTH+1];
        uint16_t address;
    } data[0xFFFF]; // I refuse to use the heap

    uint16_t size;
} WordList;

static inline void WordListInsert(char* name, uint16_t address, WordList* list) {
    list->data[list->size].address = address;
    strcpy(list->data[list->size].name, name);
    list->size++;
}

static inline bool NameInWordList(char* name, WordList* list) {
    for (int i=0; i<list->size; i++) {
        if (strcmp(name, list->data[i].name) == 0) return true;
    }
    return false;
}

// We assume that the name exists within the list because 'NameInWordList' is called first
static inline uint16_t NameIndex(char* name, WordList* list) {
    for (int i=0; i<list->size; i++) {
        if (strcmp(name, list->data[i].name) == 0) return i;
    }
    return 0;
}

static inline bool StringInArr(char* str, char* arr[], size_t arr_len) {
    for (int i=0; i<arr_len; i++) {
        if (strcmp(str, arr[i]) == 0) return true;
    }
    return false;
}

// We make the same assumptions as 'NameIndex'
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

struct LoopInfo {
    uint16_t address;
    uint16_t pending_exits[32];
    uint8_t pending_exit_ptr;
    unsigned int line;
};

typedef struct {
    struct LoopInfo data[0xFFFF]; // I love consuming unneccesary RAM
    uint16_t ptr;
} LoopStack;

static inline void PushLoopStack(uint16_t address, unsigned int line, LoopStack* stack) {
    stack->data[stack->ptr] = (struct LoopInfo){.address = address, .line = line};
    stack->ptr++;
}

static inline struct LoopInfo PopLoopStack(LoopStack* stack) {
    return stack->data[--stack->ptr];
}

static inline struct LoopInfo PeekLoopStack(LoopStack* stack) {
    return stack->data[stack->ptr-1];
}

static inline void RegisterLoopExit(uint16_t address, LoopStack* stack, bool* has_errored) {
    stack->data[stack->ptr-1].pending_exits[stack->data[stack->ptr-1].pending_exit_ptr++] = address;

    if (PeekLoopStack(stack).pending_exit_ptr > 32) {
        printf("[ERROR]: Too many 'leave' within loop at line %d. Max limit is 32\n", PeekLoopStack(stack).line);
        *has_errored = true;
    }
}

static inline void HandleLoopExits(LoopStack* stack, Rom* dest) {
    uint16_t address = dest->size;
    for (int i=0; i<PeekLoopStack(stack).pending_exit_ptr; i++) {
        dest->size = PeekLoopStack(stack).pending_exits[i];
        EmitWord(dest, address);  
    }
    dest->size = address;
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

    dest->size = 4; // Reserve space for calling 'main' + a HALT instruction
    dest->data[0] = CALL;
    dest->data[3] = HALT;

    WordList words = { 0 };
    LoopStack loops = { 0 };

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
                    // Creating new word
                    // Are we overwriting words?
                    token = src->data[++i];
                    if (token.type != WORD) {
                        printf("[ERROR]: Word '%s' at line %d has an invalid name. Word names cannot a number, ':', ';' or any control flow words\n", token.lexeme, token.line);
                        bool has_errored = true;
                    }

                    if (StringInArr(token.lexeme, instruction_primitives, ARR_LEN(instruction_primitives))) {
                        printf("[WARNING]: Word '%s' at line %d overwrites a primitive word\n", token.lexeme, token.line);
                    }
                    if (NameInWordList(token.lexeme, &words)) {
                        printf("[WARNING]: Word '%s' at line %d overwrites a previously defined word\n", token.lexeme, token.line);
                        words.data[NameIndex(token.lexeme, &words)].address = dest->size;
                    } else { 
                        WordListInsert(token.lexeme, dest->size, &words);
                    }
                    if (strcmp(token.lexeme, "main") == 0) {
                        uint16_t address = dest->size;
                        dest->size = 1;
                        EmitWord(dest, address);
                        dest->size = address; 
                    }

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

                    for (int i=loops.ptr; i>0; i--) {
                        printf("[ERROR]: Loop at line %d is unterminated\n", PopLoopStack(&loops).line);
                        has_errored = true;
                    }

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

            case LOOP_START:
                PushLoopStack(dest->size, token.line, &loops);
                break;

            case LOOP_AGAIN: {
                uint16_t distance_to_start = dest->size - PeekLoopStack(&loops).address;
                if (distance_to_start <= 127) {
                    EmitByte(dest, BRANCH);
                    EmitByte(dest, -distance_to_start);
                } else {
                    EmitByte(dest, JUMP);
                    EmitWord(dest, PeekLoopStack(&loops).address);
                }
                break;
            }

            case LOOP_WHILE: {
                if (loops.ptr == 0) {
                    printf("[ERROR]: 'while' found outside of loop at line %d\n", token.line);
                    has_errored = true;
                    break;
                }
                uint16_t start = PeekLoopStack(&loops).address;
                uint16_t distance_to_start = dest->size - start;
                if (distance_to_start <= 127) {
                    EmitByte(dest, BIFN0);
                    EmitByte(dest, -distance_to_start);
                } else {
                    // Skip jump if done
                    EmitByte(dest, BIF0);
                    EmitByte(dest, 3);
                    // Jump
                    EmitByte(dest, JUMP);
                    EmitWord(dest, start);
                }
                HandleLoopExits(&loops, dest);
                PopLoopStack(&loops);
                break;
            }

            case LOOP_UNTIL: {
                if (loops.ptr == 0) {
                    printf("[ERROR]: 'until' found outside of loop at line %d\n", token.line);
                    has_errored = true;
                    break;
                }
                uint16_t start = PeekLoopStack(&loops).address;
                uint16_t distance_to_start = dest->size - start;
                if (distance_to_start <= 127) {
                    EmitByte(dest, BIF0);
                    EmitByte(dest, -distance_to_start);
                } else {
                    // Skip jump if done
                    EmitByte(dest, BIFN0);
                    EmitByte(dest, 3);
                    // Jump
                    EmitByte(dest, JUMP);
                    EmitWord(dest, start);
                }
                HandleLoopExits(&loops, dest);
                PopLoopStack(&loops);
                break;
            }

            case LOOP_LEAVE: {
                // TODO: Add support for BRANCH as optimization
                EmitByte(dest, JUMP);
                RegisterLoopExit(dest->size, &loops, &has_errored);
                break;
            }

            case WORD:
                if (NameInWordList(token.lexeme, &words)) {
                    EmitByte(dest, CALL);
                    EmitWord(dest, words.data[NameIndex(token.lexeme, &words)].address);
                    break;
                }

                if (StringInArr(token.lexeme, instruction_primitives, ARR_LEN(instruction_primitives))) {
                    EmitByte(dest, StringIndex(token.lexeme, instruction_primitives, ARR_LEN(instruction_primitives)));
                    break;
                }
               break;
                
        
        }
    }
    if (has_errored) exit(-1);
}

