#include <stdlib.h>
#include "linked_list.h"

LLNode* create_node(void) {
    return calloc(1, sizeof(LLNode));
}

void free_node(LLNode* node) {
    free(node);
}

void append_end_node(LLNode* head, LLNode *node) {

    LLNode* link = head;

    while(link->next != 0) {
        link = (LLNode*) link->next;
    }

    link->next = node;
}

bool iterate_linked_list(LLNode** node, void** next_item) {
    if(*node == 0)
    {
        return false;
    }

    *next_item = (*node)->data;
    *node = (*node)->next;

    return true;
}
