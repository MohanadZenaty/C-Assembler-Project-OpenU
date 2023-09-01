#ifndef ASSEMBLER_TYPES_H
#define ASSEMBLER_TYPES_H

#include <stdbool.h>

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef unsigned long long u64;
typedef long long s64;

typedef struct String {
    char* data;
    s32 length;
} String;

#endif //ASSEMBLER_TYPES_H
