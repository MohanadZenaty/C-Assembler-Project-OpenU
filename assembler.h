#ifndef ASSEMBLER_ASSEBLER_H
#define ASSEMBLER_ASSEBLER_H

#include <stdio.h>
#include "types.h"
#include "linear_allocator.h"
#include "table.h"
#include "linked_list.h"

typedef enum OpCode {
    OP_MOV = 0,
    OP_CMP = 1,
    OP_ADD = 2,
    OP_SUB = 3,
    OP_NOT = 4,
    OP_CLR = 5,
    OP_LEA = 6,
    OP_INC = 7,
    OP_DEC = 8,
    OP_JMP = 9,
    OP_BNE = 10,
    OP_RED = 11,
    OP_PRN = 12,
    OP_JSR = 13,
    OP_RTS = 14,
    OP_STOP = 15,
    OP_COUNT = 16
} OpCode;

typedef enum Declaration {
    DECL_DATA = 0,
    DECL_EXTERN = 1,
    DECL_ENTRY = 2,
    DECL_STRING = 3,
    DECL_COUNT = 4
} Declaration;

// we remove the check for this variable because the warning is wrong.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static char *op_strings[OP_COUNT] = {
        "mov",
        "cmp",
        "add",
        "sub",
        "not",
        "clr",
        "lea",
        "inc",
        "dec",
        "jmp",
        "bne",
        "red",
        "prn",
        "jsr",
        "rts",
        "stop"
};

static char *declaration_strings[DECL_COUNT] = {
        ".data",
        ".extern",
        ".entry",
        ".string"
};

#pragma GCC diagnostic pop

typedef enum Register {
    R0 = 0,
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
    PSW = 8,
} Register;

typedef enum ARE {
    ABSOLUTE = 0,
    EXTERNAL = 1,
    RELOCATABLE = 2
} ARE;

typedef enum OpOperandType {
    IMMEDIATE = 1,
    LABEL = 2,
    REGISTER = 4,
} OpOperandType;

#define WORD_SIZE_IN_BITS 12
#define RAM_SIZE_INT_WORDS 1024
#define RAM_SIZE (WORD_SIZE_IN_BITS * RAM_SIZE_INT_WORDS)

typedef union Op {
    struct {
        u16 are: 2; // absolute / relocatable / external
        u16 operand_des: 3;
        u16 op_code: 4;
        u16 operand_src: 3;
    };

    u16 data;
} Op;

typedef union OpOperand {
    struct {
        u16 are: 2;
        u16 value: 10;
    };

    struct {
        u16 : 2;
        u16 register_destination: 5;
        u16 register_source: 5;
    };

    u16 data;
} OpOperand;

typedef struct Label {
    String name;
    bool external;
    bool is_entry;
    bool defined;
    bool data_label;
    s32 buffer_word_idx;
    LLNode *source_node; // linked list of LabelSource
} Label;

typedef struct LabelSource {
    s32 word_idx;
} LabelSource;

typedef struct Word {
    u16 first : 6;
    u16 second : 6;
    u16 : 4; // padding
} Word;

typedef struct Assembler {
    String source; // source file
    s32 s_idx; // offset in to source where the next character to read
    s32 line;

    bool had_error;

    s32 code_word_count;
    s32 data_word_count;

    LinearAllocator code_buffer;
    LinearAllocator data_buffer;
    LinearAllocator extern_file_buffer;
    LinearAllocator entry_file_buffer;

    Table label_table;
} Assembler;

// compiles single file
void compile(const char *file_path);

#endif //ASSEMBLER_ASSEBLER_H
