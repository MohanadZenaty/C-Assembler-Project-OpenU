#ifndef ASSEMBLER_UTILS_H
#define ASSEMBLER_UTILS_H

#include "types.h"

// reads text file into a string
bool read_file(const char *file_path, String* str);

// write data to file, override the old data
void write_file(const char* file_path, u8* data, s32 length);

// get file name with from path without extension
String get_file_name_from_path(const char* file_path);

// cuts and allocates new string using malloc
String sub_string(String src, s32 start, s32 length);

// concats two strings and returns new one
String concat_string(String str, const char* str2);

// cuts line from string and allocates new string using malloc
String get_line (String str, s32 line);

void free_string(String str);

// hashes string into u32
u32 hash_string(String str);

// prints buffer in binary, 12 bits each word
void print_bits(u16* ptr, s32 length);

#define offset_void_ptr(ptr, count) ((void*)(((u64)ptr) + count * sizeof(void*)))

#endif //ASSEMBLER_UTILS_H
