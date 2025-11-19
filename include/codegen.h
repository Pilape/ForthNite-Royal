#ifndef CODEGEN_HEADER
#define CODEGEN_HEADER

#include <stdint.h>
#include "lexer.h"

typedef struct {
    uint8_t data[0x10000/2]; // 32 KiB of ROM
    uint16_t size; 
} Rom;

void GenerateCode(const TokenList* src, Rom* dest);

#endif 
