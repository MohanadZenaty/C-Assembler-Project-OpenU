#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "utils.h"
#include <stdio.h>

bool read_file(const char *file_path, String* str) {

    FILE *file = fopen(file_path, "r");

    if(file == 0) return false;

    fseek(file, 0, SEEK_END);
    s32 size = ftell(file);

    char *buffer = (char *) calloc(size, sizeof(char));

    fseek(file, 0, SEEK_SET);
    fread(buffer, sizeof(char), size, file);
    fclose(file);

    *str = (String) {buffer, size};

    return true;
}

void write_file(const char *file_path, u8 *data, s32 length) {
    assert(false);
}

String get_file_name_from_path(const char *file_path) {
    s32 length = strlen(file_path);

    s32 start = 0;
    s32 dot = -1;

    for (int i = length - 1; i >= 0; i -= 1) {
        if(file_path[i] == '\\' || file_path[i] == '/')
        {
            start = i + 1;
            break;
        }

        if(file_path[i] == '.' && dot == -1)
        {
            dot = i;
        }
    }

    if(start == length) return (String){0, 0};

    if(dot == -1) dot = length - 1;

    s32 file_name_length = dot - start;

    char* buffer = calloc(file_name_length + 1, sizeof(char));

    memcpy(buffer, file_path + start, file_name_length);

    return (String) {buffer, file_name_length};
}

String sub_string(String src, s32 start, s32 length) {

    if(src.length >= start + length){
        assert(src.length >= start + length);
    }

    char *text = (char *) calloc((length + 1), sizeof(char));

    memcpy(text, src.data + start, length);

    return (String) {text, length};
}

String concat_string(String str, const char* str2) {
    s32 len = strlen(str2);

    String new_str = (String) {(char*)calloc(str.length + len + 1, sizeof(char)), str.length + len};

    memcpy(new_str.data, str.data, str.length);
    memcpy(new_str.data + str.length, str2, len);

    new_str.data[new_str.length] = '\0';

    return new_str;
}

u32 hash_string(String str) {
    u32 hash = 0;

    for (s32 i = 0; i < str.length; ++i) {
        hash = ((hash << 5) + hash) + str.data[i]; /* hash * 33 + c */
    }

    return hash;
}

String get_line(String str, s32 line) {
    int idx_start = 0;
    int line_count = 1;

    while (line_count != line) {
        if (str.data[idx_start] == '\n') {
            line_count++;
        }

        idx_start += 1;
    }

    int idx_end = idx_start;

    while (str.length > idx_end && str.data[idx_end] != '\n') {
        idx_end += 1;
    }

    return sub_string(str, idx_start, idx_end - idx_start);
}

void free_string(String str) {
    free(str.data);
}

void print_bits(u16 *ptr, s32 length) {

    s32 bit_count = 0;

    for (int word_i = 0; word_i < length; word_i += 1) {

        u16 word = ptr[word_i];

        for (int bit_i = 11; bit_i >= 0; bit_i -= 1) {
            if (bit_count % 12 == 0) {
                printf("\n");
            }

            if (bit_count % 4 == 0) {
                printf(" ");
            }

            bool is_one = word & (1 << bit_i);

            printf(is_one ? "1" : "0");

            bit_count += 1;
        }
    }

}
