#ifndef CODEGEN_HEADER
#define CODEGEN_HEADER

#include <stdint.h>
#include "lexer.h"

#define ROM_SIZE_MAX (0x10000/2)

typedef struct {
    uint8_t data[ROM_SIZE_MAX]; // 32 KiB of ROM
    uint16_t size; 
} Rom;

void GenerateCode(const TokenList* src, Rom* dest);

#endif 
