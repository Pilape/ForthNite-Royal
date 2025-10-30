#ifndef COMPILER_HEADER
#define COMPILER_HEADER

#include <stdio.h>
#include <stdlib.h>

#define ASSERT(condition, msg)                    \
    if (!(condition)) {                                \
        fprintf(stderr, "[ERROR]: " msg "\n"); \
        exit(-1);                                      \
    }                                                  \

#define ASSERT_FORMAT(condition, msg, ...)                    \
    if (!(condition)) {                                \
        fprintf(stderr, "[ERROR]: " msg "\n", __VA_ARGS__); \
        exit(-1);                                      \
    }                                                  \

#endif
