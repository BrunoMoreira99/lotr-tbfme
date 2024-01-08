#pragma once

typedef struct node_s {
    void* data;
    struct node_s* previous;
    struct node_s* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    size_t length;
    int (*compare)(const void* a, const void* b);
} List;

typedef struct {
    Node* top;
    size_t length;
} Stack;

/**
 * Initializes a new list.
 *
 * @param compareFunction A pointer to the comparison function.
 * @return The newly created list.
 */
List List_init(int (*compareFunction)(const void*, const void*));

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
 * Removes the first occurrence of the given data from the list.
 * 
 * @param list A pointer to the list from which to remove the data.
 * @param data_to_remove A pointer to the data to be removed.
 * @return A pointer to the data that was removed. The developer is responsible for freeing this memory.
 */
void* List_remove(List* list, void* data_to_remove);

/**
 * Clears the list, removing and freeing all data.
 * 
 * @param list A pointer to the list to be cleared.
 */
void List_clear(List* list);

/**
 * Initializes a new stack.
 *
 * @return The newly created stack.
 */
Stack Stack_init(void);

/**
 * Appends data to the end of the list.
 * 
 * @param stack A pointer to the stack.
 * @param data A pointer to the data to be appended.
 */
void Stack_push(Stack* stack, void* data);

/**
 * Removes the element at the top of the stack.
 * 
 * @param stack A pointer to the stack from which to remove the element.
 * @return A pointer to the data that was removed. The developer is responsible for freeing this memory.
 */
void* Stack_pop(Stack* stack);

/**
 * Returns the data at the top of the stack.
 * 
 * @param stack A pointer to the stack to peek at.
 * @return A pointer to the data at the top of the stack.
 */
void* Stack_peek(const Stack* stack);

/**
 * Clears the stack, removing and freeing all data.
 * 
 * @param stack A pointer to the stack to be cleared.
 */
void Stack_clear(Stack* stack);
