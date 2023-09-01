#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "linear_allocator.h"

LinearAllocator create_linear_allocator(s32 initial_size) {
    LinearAllocator allocator;

    allocator.data = (u8 *) calloc(1, initial_size);
    allocator.allocated_bytes = 0;
    allocator.total_size = initial_size;

    return allocator;
}

u8 *allocate(LinearAllocator *allocator, s32 size) {

    while (allocator->total_size < allocator->allocated_bytes + size) {
        allocator->data = (u8 *) realloc(allocator->data, allocator->total_size * 2);
        assert(allocator->data);
        memset(allocator->data + allocator->total_size, 0, allocator->total_size);
        allocator->total_size = allocator->total_size * 2;
    }

    u8 *ptr = allocator->data + allocator->allocated_bytes;

    allocator->allocated_bytes += size;

    return ptr;
}

void reset_allocator(LinearAllocator *allocator) {
    allocator->allocated_bytes = 0;
}

u8 *get_current_allocator_position(LinearAllocator *allocator) {
    return allocator->data + allocator->allocated_bytes;
}

void free_allocator(LinearAllocator *allocator) {
    free(allocator->data);
}
