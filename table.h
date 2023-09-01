#ifndef ASSEMBLER_TABLE_H
#define ASSEMBLER_TABLE_H

#include "types.h"
#include "linear_allocator.h"

typedef struct Table {
    LinearAllocator keys_allocator;
    LinearAllocator values_allocator;
    s32 item_size;
} Table;

typedef struct TableIterator {
    s32 idx;
} TableIterator;

// create new table, allocated all the memory for it
Table create_table_inner(s32 initial_size, s32 item_size);

// free all the table data
void free_table(Table* table);

// tries to add new item, if key taken return false else adds the item
bool try_add_item(Table *table, u32 key, void *item);

// tries to retrieve the item from table if item not found returns false else return true and puts the item into the pointer
bool try_get_item_inner(Table *table, u32 key, void **item);

// check if item with key exists in the table
bool key_exists(Table* table, u32 key);

// create iterator for the table that can be uses to get all key and values from the table
TableIterator create_table_iterator(Table *table);

// iterates over the table returns true if got next value else return false when no items left to iterate over,
// key or value can be null / 0 if not needed
bool iterate_table_inner(TableIterator *iterator, Table *table, u32 *key, void **value);

#define create_table(initial_size, type) (create_table_inner(initial_size, sizeof(type)))

#define try_get_item(table, key, item) (try_get_item_inner(table, key, item))
#define iterate_table(iterator, table, key, item) (iterate_table_inner(iterator, table, key, (void*)item))

#endif //ASSEMBLER_TABLE_H
