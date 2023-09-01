#ifndef ASSEMBLER_LINKED_LIST_H
#define ASSEMBLER_LINKED_LIST_H

#include "types.h"

// linked list node
typedef struct LLNode {
    void *data;
    void *next;
} LLNode;

// allocated new node
LLNode* create_node(void);

// free the node
void free_node(LLNode* node);

// appends node to the end of the linked list
void append_end_node(LLNode* head, LLNode *node);

// iterates over linked list return false when the iteration is over
bool iterate_linked_list(LLNode** node, void** next_item);

#endif //ASSEMBLER_LINKED_LIST_H
