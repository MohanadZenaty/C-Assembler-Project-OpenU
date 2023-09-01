#include <stdio.h>
#include <string.h>
#include "reader.h"
#include "types.h"
#include "assembler.h"
#include "utils.h"

bool is_white_space(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_number(char c) {
    return c >= '0' && c <= '9';
}

char get_char_at(String src, s32 idx, s32 offset) {
    return src.data[idx + offset];
}

bool has_chars_left(String src, s32 idx, s32 number_of_chars) {
    return src.length > idx + number_of_chars;
}

bool end_of_file(String src, s32 idx) {
    return src.length <= idx || src.data[idx] == 0;
}

bool is_reserved_word(String str) {
    for (int i = 0; i < OP_COUNT; ++i) {
        if (strcmp(str.data, op_strings[i]) == 0) return true;
    }

    for (int i = 0; i < DECL_COUNT; ++i) {
        if (strcmp(str.data, declaration_strings[i]) == 0) return true;
    }

    if (strcmp("mcro", str.data) == 0) return false;
    if (strcmp("endmcro", str.data) == 0) return false;

    return false;
}

bool read_remove_comments(String src, s32 *idx) {
    read_remove_spaces(src, idx);

    if (end_of_file(src, *idx) == false && get_char_at(src, *idx, 0) == ';') {
        while (end_of_file(src, *idx) == false) {
            *idx += 1;

            if(get_char_at(src, *idx, -1) == '\n')
            {
                break;
            }
        }

        return true;
    }

    return false;
}

bool read_remove_whitespace_line(String src, s32 *idx) {

    while (end_of_file(src, *idx) == false) {
        char c = get_char_at(src, *idx, 0);

        if (c == '\n') {
            *idx += 1;
            return true;
        }

        if (c != ' ' && c != '\t' && c != '\r') return false;

        *idx += 1;
    }

    return false;
}

// read removes while space of tab
void read_remove_spaces(String src, s32 *idx) {
    while (end_of_file(src, *idx) == false) {
        char c = get_char_at(src, *idx, 0);
        if (c != ' ' && c != '\t' && c != '\r') return;
        *idx += 1;
    }
}

void read_remove_till_end_of_line(String src, s32 *idx) {
    while (end_of_file(src, *idx) == false) {

        if (get_char_at(src, *idx, 0) == '\n') {
            *idx += 1;
            break;
        }

        *idx += 1;
    }
}

bool read_remove_whitespace_till_end_of_line(String src, s32 *idx) {
    while (end_of_file(src, *idx) == false) {

        char c = get_char_at(src, *idx, 0);

        if (c == '\n') {
            *idx += 1;
            break;
        }

        else if(c != ' ' && c != '\t' && c != '\r') {
            return false;
        }

        *idx += 1;
    }

    return true;
}

bool strcmp_string_word(const char *str, String string, s32 start) {
    // comparing characters
    int i = 0;

    while (str[i] != '\0' && string.length > start + i) {
        if (str[i] != string.data[start + i]) return false;

        i += 1;
    }

    // if the word is not ended properly, return false
    bool eof = end_of_file(string, start + i);
    if (eof == false && is_white_space(string.data[start + i]) == false) return false;

    return true;
}

bool try_read_label(String src, s32 *idx, String *label) {
    read_remove_spaces(src, idx);

    s32 start = *idx;
    bool got_label = try_read_label_name(src, idx, label);

    if (got_label == false || get_char_at(src, *idx, 0) != ':') {
        *idx = start;
        return false;
    }

    *idx += 1;

    return true;
}

bool try_read_op(String src, s32 *idx, OpCode *op_code) {
    read_remove_spaces(src, idx);

    if (end_of_file(src, *idx)) return false;

    for (int i = 0; i < OP_COUNT; i += 1) {
        if (strcmp_string_word(op_strings[i], src, *idx)) {
            *op_code = i;
            *idx += (s32) strlen(op_strings[i]);
            return true;
        }
    }

    return false;
}

bool try_read_declaration(String src, s32 *idx, Declaration *declaration) {
    read_remove_spaces(src, idx);

    if (end_of_file(src, *idx)) return false;

    for (int i = 0; i < DECL_COUNT; i += 1) {
        if (strcmp_string_word(declaration_strings[i], src, *idx)) {
            *declaration = i;
            *idx += (s32) strlen(declaration_strings[i]);
            return true;
        }
    }

    return false;
}

bool try_read_register(String src, s32 *idx, Register *reg) {
    read_remove_spaces(src, idx);

    if (has_chars_left(src, *idx, 3) == false) return false;

    if (get_char_at(src, *idx, 0) != '@') return false;
    if (get_char_at(src, *idx, 1) != 'r') return false;

    char c = get_char_at(src, *idx, 2);
    if (c < '0' || c > '7') return false;

    *idx += 3;
    *reg = c - '0';

    return true;
}

bool try_read_label_name(String src, s32 *idx, String *label) {
    read_remove_spaces(src, idx);

    if (end_of_file(src, *idx)) return false;

    if (is_letter(get_char_at(src, *idx, 0)) == false) return false;

    int i = 1;

    while (has_chars_left(src, *idx, i)) {
        char c = get_char_at(src, *idx, i);

        if (!is_letter(c) && !is_number(c)) {
            break;
        }

        i += 1;
    }

    // creating label
    s32 length = i;
    *label = sub_string(src, *idx, length);
    *idx += length;

    if(length > 31) {
        printf("Warning: the label %s is to long it will be clamped to 31 characters\n", label->data);
        free_string(*label);
        *label = sub_string(src, *idx, 31);
    }

    return true;
}

bool try_read_number(String src, s32 *idx, s32 *number) {
    read_remove_spaces(src, idx);

    if (end_of_file(src, *idx)) return false;

    char c = get_char_at(src, *idx, 0);

    bool number_is_negative = false;
    s32 num = 0;
    int i = 0;

    if (c == '-') {
        number_is_negative = true;
        i = 1;
    } else if (c == '+') {
        i = 1;
    } else if (!is_number(c)) {
        return false;
    }

    while (has_chars_left(src, *idx, i)) {
        char c = get_char_at(src, *idx, i);

        if(is_number(c) == false) break;

        num *= 10;
        num += get_char_at(src, *idx, i) - '0';
        i += 1;
    }

    if (number_is_negative) {
        num *= -1;
    }

    *number = num;

    *idx += i;

    if(num > 511 || num < -512) {
        printf("Warning: %d can't fit in 10 bits of data, the number will be clamped (max: 511, min:-512)\n", num);
    }

    return true;
}

bool try_read_string(String src, s32 *idx, String *string) {
    read_remove_spaces(src, idx);

    if (end_of_file(src, *idx)) return false;

    if (get_char_at(src, *idx, 0) != '"') return false;

    int i = 1;
    bool string_terminated_successfully = false;

    while (has_chars_left(src, *idx, i + 1)) {
        if (get_char_at(src, *idx, i) == '"') {
            string_terminated_successfully = true;
            i += 1;
            break;
        }

        i += 1;
    }

    if (string_terminated_successfully == false) return false;

    int length = i - 2;
    *string = sub_string(src, *idx + 1, length);
    *idx += i;

    return true;
}

bool try_read_delimiter(String src, s32 *idx) {
    read_remove_spaces(src, idx);

    if (end_of_file(src, *idx)) return false;

    if (get_char_at(src, *idx, 0) == ',') {
        *idx += 1;
        return true;
    }

    return false;
}

bool try_read_macro_definition(String src, s32 *idx, String *macro_label_name) {
    read_remove_spaces(src, idx);

    if (end_of_file(src, *idx)) return false;

    if (has_chars_left(src, *idx, 4) == false) return false;

    if (strcmp_string_word("mcro", src, *idx) == false) return false;

    s32 start = *idx + 4;

    bool got_label_name = try_read_label_name(src, &start, macro_label_name);

    if (got_label_name == false) return false;

    *idx = start;

    return true;
}

bool try_read_macro_end_definition(String src, s32 *idx) {
    read_remove_spaces(src, idx);

    if (end_of_file(src, *idx)) return false;

    if (has_chars_left(src, *idx, sizeof("endmcro")) == false) return false;

    if (strcmp_string_word("endmcro", src, *idx) == false) return false;

    return true;
}
