//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LIST_H
#define GAME_LIST_H

// List item
typedef struct Node {
    void *data;
    struct Node *prev;
    struct Node *next;
} Node;

// Doubly linked list
typedef struct {
    Node *head;
    Node *tail;
    int size;
} List;

typedef struct {
    void **elements;
    int size;
} SizedArray;

// Internal functions
Node* createNode(void *data);

// Public functions

// Create a new list
List* CreateList();

// Add an item to the list
void ListAdd(List* list, void *data);

// Remove an item from the list
void ListRemove(List* list, Node* node);

void ListRemoveAt(List *list, int index);

// Insert an item after a given node
void ListInsertAfter(List* list, Node* prevNode, void *data);

// Get an item from the list
void *ListGet(List* list, int index);

// Free the list structure, but not the data
void ListFree(List* list);

// Free the list structure and the data
void ListFreeWithData(List* list);

// Get the size of the list (you could also use list->size, that would be faster)
int ListGetSize(List* list);

SizedArray* ToSizedArray(List *list);

#define SizedArrayGet(array, index) ((array)->elements[(index)])

void DestroySizedArray(SizedArray *array);

#endif //GAME_LIST_H
