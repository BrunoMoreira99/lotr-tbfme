#pragma once

typedef struct Node {
    void* data;
    struct Node* next;
} Node;

typedef struct {
    size_t size;
    Node* head;
} List;

List* createList(void);
void List_push(List* list, void* data);
void* List_pop(List* list, const size_t index);
void freeList(List* list);
