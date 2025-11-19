#include "../include/codegen.h"

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
        "begin", "until", "while, repeat",
        "again", "leave",
    };
}

