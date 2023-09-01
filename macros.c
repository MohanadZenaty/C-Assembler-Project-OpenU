#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "macros.h"
#include "linear_allocator.h"
#include "reader.h"
#include "utils.h"
#include "table.h"

typedef struct Macro {
    String name;
    String source;
} Macro;

// appends string to allocator
void append_sub_string(LinearAllocator *allocator, String src, s32 start, s32 end) {
    s32 length = end - start;
    char *buffer = (char *) allocate(allocator, length);
    memcpy(buffer, src.data + start, length);
}

bool try_expend_macros(String src, String *new_source) {

    LinearAllocator expended_source_allocator = create_linear_allocator(32);

    Table macro_table = create_table(32, Macro);

    int idx = 0;
    int line = 1;

    // reads each line and trying to expend it / get macro definition
    while (end_of_file(src, idx) == false) {

        String macro_name;
        bool got_macro = try_read_macro_definition(src, &idx, &macro_name);

        // adding macro definition
        if (got_macro) {

            // check and save macro
            if (is_reserved_word(macro_name)) {
                printf("(macro name can't be operation or declaration name)");
                free_allocator(&expended_source_allocator);
                return false;
            }

            read_remove_till_end_of_line(src, &idx);
            line += 1;

            s32 macro_start_idx = idx;

            while (try_read_macro_end_definition(src, &idx) == false) {
                read_remove_till_end_of_line(src, &idx);
                line += 1;

                if (end_of_file(src, idx)) {
                    printf("invalid macro, 'endmcro' not defined for macro %s\n", macro_name.data);
                    free_allocator(&expended_source_allocator);
                    return false;
                }
            }

            String macro_source = sub_string(src, macro_start_idx, idx - macro_start_idx);

            read_remove_till_end_of_line(src, &idx);
            line += 1;

            Macro macro = (Macro) {macro_name, macro_source};

            bool added_macro = try_add_item(&macro_table, hash_string(macro.name), &macro);

            if (added_macro == false) {
                printf("[line %d] redeclaration of macro: %s\n", line, macro.name.data);
                free_allocator(&expended_source_allocator);
                return false;
            }

        } else {
            // trying to expend macro

            s32 start = idx;
            String macro_to_expend;
            bool got_label_name = try_read_label_name(src, &idx, &macro_to_expend);

            if (got_label_name) {
                Macro *macro;

                bool found_macro = try_get_item(&macro_table, hash_string(macro_to_expend), (void **) &macro);

                // if we found macro, we expending it and going to the next line
                if (found_macro) {
                    read_remove_till_end_of_line(src, &idx);
                    line += 1;

                    char *buffer = (char *) allocate(&expended_source_allocator, macro->source.length);
                    memcpy(buffer, macro->source.data, macro->source.length);
                    continue;
                }
            }

            // appending line when no macro name of macro definition found
            read_remove_till_end_of_line(src, &idx);
            line += 1;

            append_sub_string(&expended_source_allocator, src, start, idx);
        }
    }

    // free data

    TableIterator iterator = create_table_iterator(&macro_table);
    Macro *macro;

    while (iterate_table(&iterator, &macro_table, 0, &macro)) {
        free_string(macro->name);
        free_string(macro->source);
    }

    free_table(&macro_table);

    // set return values
    *new_source = (String) {(char *) expended_source_allocator.data, expended_source_allocator.allocated_bytes};
    return true;
}
