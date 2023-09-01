#ifndef ASSEMBLER_LINEAR_ALLOCATOR_H
#define ASSEMBLER_LINEAR_ALLOCATOR_H

#include "types.h"

// growable array data structure
typedef struct LinearAllocator {
    u8* data; // pointer to the start of the buffer
    s32 allocated_bytes;
    s32 total_size; // size in bytes
} LinearAllocator;

// create new linear allocator
LinearAllocator create_linear_allocator(s32 initial_size);

// allocates memory using linear allocator
u8* allocate(LinearAllocator* allocator, s32 size);

// free all linear allocator memory
void reset_allocator(LinearAllocator* allocator);

// gets current 'length' of the allocator / end point
u8* get_current_allocator_position(LinearAllocator* allocator);

// frees underlying allocator arena
void free_allocator(LinearAllocator* allocator);

// defines for ease of use
#define allocate_single(allocator, type) (type*)(allocate(allocator, sizeof(type)))
#define allocate_multiple(allocator, type, count) (type*)(allocate(allocator, sizeof(type) * count))

#endif //ASSEMBLER_LINEAR_ALLOCATOR_H
