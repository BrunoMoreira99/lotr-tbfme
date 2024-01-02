#include <stdlib.h>

#include "collections.h"

Stack Stack_init(void) {
    Stack new_stack;
    new_stack.length = 0;
    new_stack.top = NULL;
    return new_stack;
}

void Stack_push(Stack* stack, void* data) {
    if (stack == NULL) return;
    Node* new_node = malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = stack->top;
    stack->top = new_node;
    ++stack->length;
}

void* Stack_pop(Stack* stack) {
    if (stack == NULL || stack->top == NULL) return NULL;
    Node* node_to_remove = stack->top;
    stack->top = stack->top->next;
    void* removed_data = node_to_remove->data;
    free(node_to_remove);
    --stack->length;
    return removed_data;
}

void* Stack_peek(const Stack* stack) {
    if (stack == NULL || stack->top == NULL) return NULL;
    return stack->top->data;
}

void Stack_clear(Stack* stack) {
    if (stack == NULL) return;
    Node *current_node = stack->top;
    while (current_node != NULL) {
        Node *next_node = current_node->next;
        free(current_node->data); // Dangerous: This assumes the data is in the heap. It will crash if it is not.
        free(current_node);
        current_node = next_node;
    }
    stack->top = NULL;
    stack->length = 0;
}
