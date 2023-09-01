#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "assembler.h"
#include "reader.h"
#include "macros.h"
#include "utils.h"

const OpOperandType op_valid_source_operand[] = {
        IMMEDIATE | LABEL | REGISTER,
        IMMEDIATE | LABEL | REGISTER,
        IMMEDIATE | LABEL | REGISTER,
        IMMEDIATE | LABEL | REGISTER,
        0,
        0,
        LABEL,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
};

const OpOperandType op_valid_destination_operand[] = {
        LABEL | REGISTER,
        IMMEDIATE | LABEL | REGISTER,
        LABEL | REGISTER,
        LABEL | REGISTER,
        LABEL | REGISTER,
        LABEL | REGISTER,
        LABEL | REGISTER,
        LABEL | REGISTER,
        LABEL | REGISTER,
        LABEL | REGISTER,
        LABEL | REGISTER,
        LABEL | REGISTER,
        IMMEDIATE | LABEL | REGISTER,
        LABEL | REGISTER,
        0,
        0,
};

// prints current parsing line with number
void print_current_line(Assembler *assembler) {
    String line = get_line(assembler->source, assembler->line);

    printf("line:%d: %s", assembler->line, line.data);

    free_string(line);
}

// prints error with current line and sets had error flag
void handle_error(Assembler *assembler, const char *format, ...) {
    assembler->had_error = true;
    va_list argptr;
    va_start(argptr, format);
    print_current_line(assembler);
    vprintf(format, argptr);
    va_end(argptr);
}

// create new instance of assembler and allocates memory for it
Assembler create_assembler() {
    Assembler assembler;

    assembler.s_idx = 0;
    assembler.line = 1;

    assembler.code_word_count = 0;
    assembler.data_word_count = 0;

    assembler.code_buffer = create_linear_allocator(32);
    assembler.data_buffer = create_linear_allocator(32);
    assembler.extern_file_buffer = create_linear_allocator(32);
    assembler.entry_file_buffer = create_linear_allocator(32);

    assembler.label_table = create_table(32, Label);

    return assembler;
}

// frees all allocated memory
void free_assembler(Assembler* assembler) {

    free_string(assembler->source);

    free_allocator(&assembler->entry_file_buffer);
    free_allocator(&assembler->extern_file_buffer);
    free_allocator(&assembler->code_buffer);
    free_allocator(&assembler->data_buffer);

    TableIterator table_itr = create_table_iterator(&assembler->label_table);
    Label* label;

    while(iterate_table(&table_itr, &assembler->label_table, 0, &label))
    {
        LLNode* node = label->source_node;

        while(node) {
            LLNode* next = node->next;
            free(node->data);
            free_node(node);
            node = next;
        }

            free_string(label->name);
    }

    free_table(&assembler->label_table);
}

// create new label
Label create_label(String name) {
    Label label;

    label.name = name;
    label.defined = false;
    label.external = false;
    label.is_entry = false;
    label.data_label = false;
    label.buffer_word_idx = 0;
    label.source_node = 0;

    return label;
}

// get label by name, if it doesn't exist creates one
Label *get_label(Assembler *assembler, String name) {

    Label *label;
    u32 hash = hash_string(name);
    Table *table = &assembler->label_table;

    bool got_label = try_get_item(table, hash, (void **) &label);

    if (got_label == false) {
        Label label_to_add = create_label(name);
        try_add_item(table, hash, &label_to_add);
        try_get_item(table, hash, (void **) &label);
    }

    return label;
}

// appends code word idx to labels linked list of all instances where it used in code.
void label_append_source_node(Label *label, s32 word_idx) {

    s32 *data = calloc(1, sizeof(s32));
    *data = word_idx;

    LLNode *node = create_node();
    node->data = data;

    if (label->source_node == 0) {
        label->source_node = node;
    } else {
        append_end_node(label->source_node, node);
    }
}

// adds word to a buffer
void push_word(LinearAllocator *allocator, s32 *word_count, u16 data) {
    u16 *buffer = (u16 *) allocate(allocator, 2);
    *buffer = data;
    *word_count += 1;
}

// set operand type of source or destination
void set_op_operand(Op *op, bool is_source, u8 val) {
    if (is_source) {
        op->operand_src = val;
    } else {
        op->operand_des = val;
    }
}

// @params
// assembler - in
// code - in
// op - in out
// operand - out
// is_register - out
// parses next operand, setting op operand fields, setting operand data and setting is_register
bool
try_parse_operand(Assembler *assembler, OpCode code, bool is_source, Op *op, OpOperand *operand, bool *is_register,
                  s32 word_idx_for_label) {

    // getting possible types of operands for op
    OpOperandType possible_types = is_source ? op_valid_source_operand[code] : op_valid_destination_operand[code];

    operand->value = 0;

    if (is_register != 0) {
        *is_register = false;
    }

    // try to parse label operand
    String label_name;
    bool got_label = try_read_label_name(assembler->source, &assembler->s_idx, &label_name);

    if (got_label) {

        if ((possible_types & LABEL) == 0) {
            handle_error(assembler, " | labels are not supported as %s in %s op\n",
                         is_source ? "source" : "destination",
                         op_strings[code]);
            return false;
        }

        set_op_operand(op, is_source, 3);

        Label *label = get_label(assembler, label_name);

        label_append_source_node(label, word_idx_for_label);

        return true;
    }

    // try to parse register operand

    Register reg;
    bool got_register = try_read_register(assembler->source, &assembler->s_idx, &reg);

    if (got_register) {

        if ((possible_types & REGISTER) == 0) {
            handle_error(assembler, " | registers are not supported as %s in %s op\n",
                         is_source ? "source" : "destination",
                         op_strings[code]);
            return false;
        }

        set_op_operand(op, is_source, 5);
        operand->are = ABSOLUTE;

        if (is_source) {
            operand->register_source = reg;
        } else {
            operand->register_destination = reg;
        }

        if (is_register != 0) {
            *is_register = true;
        }

        return true;
    }

    // try to parse number operand

    s32 number;
    bool got_number = try_read_number(assembler->source, &assembler->s_idx, &number);

    if (got_number) {

        if ((possible_types & IMMEDIATE) == 0) {
            handle_error(assembler, " | immediate values are not supported as %s in %s op\n",
                         is_source ? "source" : "destination",
                         op_strings[code]);
            return false;
        }

        set_op_operand(op, is_source, 1);
        operand->are = ABSOLUTE;
        operand->value = number;

        return true;
    }

    return false;
}


// trying to parse op and adding to label instance of use if op valid
bool try_parse_op(Assembler *assembler, Label *label) {
    OpCode op_code;
    bool got_op = try_read_op(assembler->source, &assembler->s_idx, &op_code);

    if (got_op) {

        if (label) {
            label->buffer_word_idx = assembler->code_word_count;
        }

        // encode op
        Op op;
        op.data = 0;
        op.op_code = op_code;

        // two operand operations
        if (op_code == OP_MOV || op_code == OP_CMP || op_code == OP_ADD || op_code == OP_SUB || op_code == OP_LEA) {

            OpOperand source_operand;
            bool source_is_register;
            bool got_source_operand = try_parse_operand(assembler, op_code, true, &op, &source_operand,
                                                        &source_is_register, assembler->code_word_count + 1);
            if (got_source_operand == false) return false;

            bool got_delimiter = try_read_delimiter(assembler->source, &assembler->s_idx);
            if (got_delimiter == false) return false;

            OpOperand destination_operand;
            bool destination_is_register;
            bool got_destination_operand = try_parse_operand(assembler, op_code, false, &op, &destination_operand,
                                                             &destination_is_register, assembler->code_word_count + 2);
            if (got_destination_operand == false) return false;

            push_word(&assembler->code_buffer, &assembler->code_word_count, op.data);

            // checking both operands are registers if true encoding as one word else as two
            if (source_is_register && destination_is_register) {
                source_operand.register_destination = destination_operand.register_destination;
                push_word(&assembler->code_buffer, &assembler->code_word_count, source_operand.data);
            } else {
                push_word(&assembler->code_buffer, &assembler->code_word_count, source_operand.data);
                push_word(&assembler->code_buffer, &assembler->code_word_count, destination_operand.data);
            }

        } else if (op_code == OP_NOT || op_code == OP_CLR || op_code == OP_INC || op_code == OP_DEC ||
                   op_code == OP_JMP || op_code == OP_BNE || op_code == OP_RED || op_code == OP_PRN ||
                   op_code == OP_JSR) { // one operand operations

            OpOperand source_operand;
            bool got_source_operand = try_parse_operand(assembler, op_code, false, &op, &source_operand, 0,
                                                        assembler->code_word_count + 1);
            if (got_source_operand == false) return false;

            push_word(&assembler->code_buffer, &assembler->code_word_count, op.data);
            push_word(&assembler->code_buffer, &assembler->code_word_count, source_operand.data);

        } else { // no operand operations, OP_RTS || OP_STOP
            push_word(&assembler->code_buffer, &assembler->code_word_count, op.data);
        }

    }

    return got_op;
}

// trying to parse declaration, setting labels flags and adding sources if parsed properly
bool try_parse_declaration(Assembler *assembler, Label *label) {

    Declaration declaration;
    bool got_declaration = try_read_declaration(assembler->source, &assembler->s_idx, &declaration);

    if (got_declaration) {

        if (label) {
            label->data_label = true;
        }

        // string declaration
        if (declaration == DECL_STRING) {
            String str;
            bool got_string = try_read_string(assembler->source, &assembler->s_idx, &str);

            if (got_string == false) {
                handle_error(assembler, " | Error expected string\n");
            } else {

                s32 word_start = assembler->data_word_count;

                for (int i = 0; i < str.length + 1; ++i) {
                    push_word(&assembler->data_buffer, &assembler->data_word_count, str.data[i]);
                }

                if (label) {
                    label->data_label = true;
                    label->buffer_word_idx = word_start;
                }
            }

            free_string(str);
        } else if (declaration == DECL_DATA) { // data declaration

            bool need_to_parse_number = true;
            s32 word_start = assembler->data_word_count;

            while (need_to_parse_number) {

                need_to_parse_number = false;

                s32 num;
                bool got_number = try_read_number(assembler->source, &assembler->s_idx, &num);

                if (got_number) {
                    push_word(&assembler->data_buffer, &assembler->data_word_count, num);
                } else {
                    handle_error(assembler, " | Error expected integer / number\n");
                }

                bool got_delimiter = try_read_delimiter(assembler->source, &assembler->s_idx);

                if (got_delimiter) {
                    need_to_parse_number = true;
                }
            }

            if (label) {
                label->data_label = true;
                label->buffer_word_idx = word_start;
            }

        } else if (declaration == DECL_ENTRY) { // entry declaration

            if(label) {
                print_current_line(assembler);
                printf(" | Warning using label on entry statement has no meaning\n");
            }

            String label_name;
            bool got_label_name = try_read_label_name(assembler->source, &assembler->s_idx, &label_name);

            if (got_label_name == false) {
                handle_error(assembler, " | Error expected label name\n");
            } else {

                Label *decl_label = get_label(assembler, label_name);

                if (decl_label->is_entry) {
                    handle_error(assembler, " | Warning redeclaration of entry label %s\n", decl_label->name.data);
                }

                decl_label->is_entry = true;
            }
        } else if (declaration == DECL_EXTERN) { // extern declaration

            if(label) {
                print_current_line(assembler);
                printf(" | Warning using label on extern statement has no meaning\n");
            }

            String label_name;
            bool got_label_name = try_read_label_name(assembler->source, &assembler->s_idx, &label_name);

            if (got_label_name == false) {
                handle_error(assembler, " | Error expected label name\n");
            } else {
                Label *decl_label = get_label(assembler, label_name);

                if (decl_label->external) {
                    handle_error(assembler, " | Warning redeclaration of extern label %s\n", decl_label->name.data);
                }

                decl_label->external = true;
                decl_label->defined = true;
            }
        }
    }

    return got_declaration;
}

const char base64_table[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

// writes program to files
// if files exist overrides the data of the file
void write_program_to_file(String file_name, Assembler *assembler) {

    // ob file
    {
        String ob_file_name = concat_string(file_name, ".ob");

        FILE *file = fopen(ob_file_name.data, "w");

        fprintf(file, "%d %d\n", assembler->code_word_count, assembler->data_word_count);

        bool first_line = true;
        //int word_idx = 101;

        for (int i = 0; i < assembler->code_word_count; ++i) {
            Word *word = ((Word *) assembler->code_buffer.data) + i;

            if(first_line == false)
            {
                fputc('\n', file);
            }

            first_line = false;

            fprintf(file,"%c%c", base64_table[word->second], base64_table[word->first]);

            //word_idx += 1;
        }

        for (int i = 0; i < assembler->data_word_count; ++i) {
            Word *word = ((Word *) assembler->data_buffer.data) + i;

            if(first_line == false)
            {
                fputc('\n', file);
            }
            first_line = false;

            fprintf(file,"%c%c", base64_table[word->second], base64_table[word->first]);

            //word_idx += 1;
        }

        fclose(file);

        free_string(ob_file_name);
    }

    // ext file
    if(assembler->extern_file_buffer.allocated_bytes > 0) {

        String ext_file_name = concat_string(file_name, ".ext");

        FILE *file = fopen(ext_file_name.data, "w");

        fprintf(file, (char *) assembler->extern_file_buffer.data);

        fclose(file);

        free_string(ext_file_name);
    }

    // ent file
    if(assembler->entry_file_buffer.allocated_bytes > 0){
        String ent_file_name = concat_string(file_name, ".ent");

        FILE *file = fopen(ent_file_name.data, "w");

        fprintf(file, (char *) assembler->entry_file_buffer.data);

        fclose(file);

        free_string(ent_file_name);
    }

}

// we call this procedure after read the source file and filling the code and data buffers.
// iterating over all labels, fills fields of label offsets in code and setting ARE.
// if the label is extern of entry writing declarations to entry or extern buffers.
void resolve_labels(Assembler *assembler) {
    TableIterator table_itr = create_table_iterator(&assembler->label_table);
    Label *label;

    while (iterate_table(&table_itr, &assembler->label_table, 0, &label)) {
        if (label->defined) {

            LLNode *start = label->source_node;
            s32 *word_idx;

            if (label->external) {
                while (iterate_linked_list(&start, (void **) &word_idx)) {
                    OpOperand *operand = (OpOperand *) assembler->code_buffer.data;
                    operand += *word_idx;
                    operand->are = EXTERNAL;

                    // write to extern
                    char char_buffer[128];

                    s32 offset = 100 + *word_idx;
                    sprintf(char_buffer, "%d\t%s\n", offset, label->name.data);

                    s32 len = strlen(char_buffer);

                    char *buffer = (char *) allocate(&assembler->extern_file_buffer, len);
                    memcpy(buffer, char_buffer, len);

                }
            } else {

                s32 offset = label->buffer_word_idx + 100;

                if (label->data_label) {
                    offset += assembler->code_word_count;
                }

                while (iterate_linked_list(&start, (void **) &word_idx)) {
                    OpOperand *operand = (OpOperand *) assembler->code_buffer.data;
                    operand += *word_idx;
                    operand->value = (u16) offset;
                    operand->are = RELOCATABLE;
                }
            }

            if (label->is_entry) {
                // write in to entry file
                char char_buffer[128];

                s32 offset = label->buffer_word_idx + 100;

                if (label->data_label) {
                    offset += assembler->code_word_count;
                }

                sprintf(char_buffer, "%d\t%s\n", offset, label->name.data);

                s32 len = strlen(char_buffer);

                char *buffer = (char *) allocate(&assembler->entry_file_buffer, len);
                memcpy(buffer, char_buffer, len);
            }

        } else {
            handle_error(assembler, " | Error label %s not defined but used", label->name.data);
        }
    }
}

void compile(const char *file_path) {

    // reading source file
    String file_name = get_file_name_from_path(file_path);

    String source;

    bool got_file = read_file(file_path, &source);

    if(got_file == false)
    {
        printf("Error could not open file %s\n", file_path);
        return;
    }

    // expending macros
    Assembler assembler = create_assembler();
    bool expended_macros = try_expend_macros(source, &assembler.source);
    free_string(source);


    if (expended_macros == false) {
        handle_error(&assembler, "Error could not expend macros in file %s\n", file_path);
        free_assembler(&assembler);
        return;
    }

    // reading each line and handling it
    while (end_of_file(assembler.source, assembler.s_idx) == false) {

        // removing whites spaces and comments
        while (read_remove_comments(assembler.source, &assembler.s_idx) ||
               read_remove_whitespace_line(assembler.source, &assembler.s_idx)) {
            assembler.line += 1;
        }

        // trying to read label
        String label_string;
        bool got_label = try_read_label(assembler.source, &assembler.s_idx, &label_string);

        Label *label = 0;

        if (got_label) {

            if (is_reserved_word(label_string)) {
                handle_error(&assembler, " | Error reserved name used as label\n");
            } else {
                label = get_label(&assembler, label_string);
                label->defined = true;
            }
        }

        // trying to parse op or declaration
        bool parsed = try_parse_op(&assembler, label);

        if (parsed == false) {
            parsed = try_parse_declaration(&assembler, label);
        }

        if(parsed == false) {
            handle_error(&assembler," | Error invalid line, unknown source code\n");
            read_remove_till_end_of_line(assembler.source, &assembler.s_idx);
        } else {
            // preparing to next line
            bool got_proper_end_of_line = read_remove_whitespace_till_end_of_line(assembler.source, &assembler.s_idx);

            if(got_proper_end_of_line == false)
            {
                handle_error(&assembler," | Error unexpected source code at the end of a line\n");
                read_remove_till_end_of_line(assembler.source, &assembler.s_idx);
            }
        }

        assembler.line += 1;
    }

    // doing all the label logic
    resolve_labels(&assembler);

    // writing file if had no error while parsing the source code
    if (assembler.had_error == false) {
        write_program_to_file(file_name, &assembler);
    }

    free_assembler(&assembler);
}
