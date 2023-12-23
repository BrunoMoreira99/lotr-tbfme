#include <stdlib.h>

#include "list.h"

List* createList(void) {
    List *new_list = malloc(sizeof(List));
    new_list->size = 0;
    new_list->head = NULL;
    return new_list;
}

void List_push(List* list, void* data) {
    Node *new_node = malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = list->head;
    list->head = new_node;
    ++list->size;
}

void* List_pop(List* list, const size_t index) {
    if (list->size <= index) return NULL;
    Node *node_to_remove = list->head;
    void *removed_data;
    if (index == 0) {
        removed_data = node_to_remove->data;
        list->head = node_to_remove->next;
    } else {
        for (size_t i = 0; i < index - 1; ++i) {
            node_to_remove = node_to_remove->next;
        }
        Node *previous_node = node_to_remove;
        node_to_remove = node_to_remove->next;
        removed_data = node_to_remove->data;
        previous_node->next = node_to_remove->next;
    }
    free(node_to_remove);
    --list->size;
    return removed_data;
}

void freeList(List* list) {
    Node *current_node = list->head;
    while (current_node != NULL) {
        Node *next_node = current_node->next;
        free(current_node);
        current_node = next_node;
    }
    free(list);
}
