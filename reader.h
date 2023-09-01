#ifndef ASSEMBLER_READER_H
#define ASSEMBLER_READER_H

#include "assembler.h"

// checks if end of file
bool end_of_file(String src, s32 idx);

// checks if str is reserved word
bool is_reserved_word(String str);

// reads line if it is a comment return true if comment else false
bool read_remove_comments(String src, s32* idx);

// reads line if it is an empty line return true if empty else false
bool read_remove_whitespace_line(String src, s32*);

// reads till end of a line
void read_remove_till_end_of_line(String src, s32 *idx);

// read till end of a line and checks if any non whitespace character exist, if exist return false else true
bool read_remove_whitespace_till_end_of_line(String src, s32 *idx);

// read tables and spaces
void read_remove_spaces(String src, s32 *idx);

// tries to read label if succeed return true and fills the label pointer
bool try_read_label(String src, s32 *idx, String *label);

// tries to read op if succeed return true and fills the op_code pointer
bool try_read_op(String src, s32 *idx, OpCode *op_code);

// tries to read declaration if succeed return true and fills the declaration pointer
bool try_read_declaration(String src, s32 *idx, Declaration *declaration);

// tries to read register if succeed return true and fills the reg pointer
bool try_read_register(String src, s32 *idx, Register *reg);

// tries to read label_name if succeed return true and fills the label pointer
bool try_read_label_name(String src, s32 *idx, String *label);

// tries to read number if succeed return true and fills the number pointer
bool try_read_number(String src, s32 *idx, s32 *number);

// tries to read string if succeed return true and fills the string pointer
bool try_read_string(String src, s32 *idx, String* string);

// tries to read delimiter if succeed return true
bool try_read_delimiter(String src, s32 *idx);

// tries to read macro definition if succeed return true and fills macro_label_name pointer
bool try_read_macro_definition(String src, s32 *idx, String* macro_label_name);

// tries to read macro end definition if succeed return true
bool try_read_macro_end_definition(String src, s32 *idx);


#endif //ASSEMBLER_READER_H
