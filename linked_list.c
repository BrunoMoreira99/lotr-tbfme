#include <stdlib.h>

#include "linked_list.h"

List List_init(void) {
    List new_list;
    new_list.length = 0;
    new_list.head = NULL;
    new_list.tail = NULL;
    return new_list;
}

void List_append(List* list, void* data) {
    if (list == NULL) return;
    Node *new_node = malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = NULL;
    new_node->previous = list->tail;
    if (list->tail != NULL) {
        list->tail->next = new_node;
    }
    list->tail = new_node;
    if (list->head == NULL) {
        list->head = new_node;
    }
    ++list->length;
}

void* List_pop(List* list, const size_t index) {
    if (list == NULL) return list;
    if (index >= list->length) {
        return NULL;
    }
    Node* node_to_remove;
    if (index < list->length / 2) {
        node_to_remove = list->head;
        for (size_t i = 0; i < index; ++i) {
            node_to_remove = node_to_remove->next;
        }
    } else {
        node_to_remove = list->tail;
        for (size_t i = list->length - 1; i > index; --i) {
            node_to_remove = node_to_remove->previous;
        }
    }
    if (node_to_remove->previous != NULL) {
        node_to_remove->previous->next = node_to_remove->next;
    }
    if (node_to_remove->next != NULL) {
        node_to_remove->next->previous = node_to_remove->previous;
    }
    if (list->head == node_to_remove) {
        list->head = node_to_remove->next;
    }
    if (list->tail == node_to_remove) {
        list->tail = node_to_remove->previous;
    }
    void *remove_data = node_to_remove->data;
    free(node_to_remove);
    --list->length;
    return remove_data;
}

void List_clear(List* list) {
    if (list == NULL) return;
    Node *current_node = list->head;
    while (current_node != NULL) {
        Node *next_node = current_node->next;
        free(current_node->data); // This will crash if the data is not in the heap.
        free(current_node);
        current_node = next_node;
    }
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}
