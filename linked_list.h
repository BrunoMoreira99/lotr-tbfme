#pragma once

typedef struct listNode {
    void* data;
    struct listNode* previous;
    struct listNode* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    size_t length;
} List;

/**
 * Initializes a new list.
 * 
 * @return The newly created list.
 */
List List_init(void);

/**
 * Appends data to the end of the list.
 * 
 * @param list A pointer to the list.
 * @param data A pointer to the data to be appended.
 */
void List_append(List* list, void* data);

/**
 * Removes an element from the list at the specified index.
 * 
 * @param list A pointer to the list from which to remove the element.
 * @param index The index of the element to be removed.
 * @return A pointer to the data that was removed. The developer is responsible for freeing this memory.
 */
void* List_pop(List* list, size_t index);

/**
 * Clears the list, removing and freeing all data.
 * 
 * @param list A pointer to the list to be cleared.
 */
void List_clear(List* list);
