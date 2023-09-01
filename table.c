#include <string.h>
#include "table.h"
#include "types.h"
#include "linear_allocator.h"

Table create_table_inner(s32 initial_size, s32 item_size) {
    Table table;

    table.keys_allocator = create_linear_allocator(initial_size * sizeof(u32));
    table.values_allocator = create_linear_allocator(initial_size * item_size);
    table.item_size = item_size;

    return table;
}

void free_table(Table *table) {
    free_allocator(&table->keys_allocator);
    free_allocator(&table->values_allocator);
}

bool try_add_item(Table *table, u32 key, void *item) {

    if (key_exists(table, key)) return false;

    u8 *buffer = allocate(&table->values_allocator, table->item_size);
    u32* key_to_add = (u32*)allocate(&table->keys_allocator, sizeof(u32));

    memcpy(buffer, item, table->item_size);
    *key_to_add = key;

    return true;
}

bool key_exists(Table *table, u32 key) {

    s32 item_count = table->keys_allocator.allocated_bytes / sizeof(u32);

    u32 *table_key = (u32 *) table->keys_allocator.data;

    for (int i = 0; i < item_count; ++i) {
        if (*table_key == key) return true;
        table_key += 1;
    }

    return false;
}

bool try_get_item_inner(Table *table, u32 key, void **item) {

    u32 itr_key;
    void* itr_item;

    TableIterator itr = create_table_iterator(table);

    while(iterate_table(&itr, table, &itr_key, &itr_item))
    {
        if(itr_key == key)  {
            *item = itr_item;
            return true;
        }
    }

    return false;
}

TableIterator create_table_iterator(Table *table) {
    return (TableIterator) {-1};
}

bool iterate_table_inner(TableIterator *iterator, Table *table, u32 *key, void **value) {

    iterator->idx += 1;
    s32 item_count = table->keys_allocator.allocated_bytes / sizeof(u32);

    if (iterator->idx >= item_count) return false;

    u32 *table_key = ((u32 *) table->keys_allocator.data) + iterator->idx;
    u8 *table_value = ((u8 *) table->values_allocator.data) + (table->item_size * iterator->idx);

    if (key != 0) {
        *key = *table_key;
    }

    if (value != 0) {
        *value = table_value;
    }

    return true;
}
