#include <stdlib.h>

#include "collections.h"

List List_init(int (*compareFunction)(const void*, const void*)) {
    List new_list;
    new_list.length = 0;
    new_list.head = NULL;
    new_list.tail = NULL;
    new_list.compare = compareFunction;
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
    if (index >= list->length) return NULL;
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
    void *removed_data = node_to_remove->data;
    free(node_to_remove);
    --list->length;
    return removed_data;
}

void* List_remove(List* list, const void* data_to_remove) {
    if (list == NULL || data_to_remove == NULL) return NULL; 
    Node* current_node = list->head;
    while (current_node != NULL) {
        if (list->compare(current_node->data, data_to_remove) == 0) {
            if (current_node == list->head) {
                list->head = list->head->next;
            }
            if (current_node->previous != NULL) {
                current_node->previous->next = current_node->next;
            }
            if (current_node->next != NULL) {
                current_node->next->previous = current_node->previous;
            }
            if (current_node == list->tail) {
                list->tail = list->tail->previous;
            }
            void* removed_data = current_node->data;
            free(current_node);
            --list->length;
            return removed_data;
        }
        current_node = current_node->next;
    }
    return NULL;
}

void List_clear(List* list) {
    if (list == NULL) return;
    Node *current_node = list->head;
    while (current_node != NULL) {
        Node *next_node = current_node->next;
        free(current_node->data); // Dangerous: This assumes the data is in the heap. It will crash if it is not.
        free(current_node);
        current_node = next_node;
    }
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}
