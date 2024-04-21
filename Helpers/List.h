//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LIST_H
#define GAME_LIST_H

typedef struct Node {
    void *data;
    struct Node *prev;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int size;
} List;

Node* createNode(void *data);
List* CreateList();
void ListAdd(List* list, void *data);
void ListRemove(List* list, Node* node);
void ListInsertAfter(List* list, Node* prevNode, void *data);
void *ListGet(List* list, int index);

// !!!WARNING!!! DOES NOT FREE DATA ITEMS
void ListFree(List* list);

// This DOES free the data.
void ListFreeWithData(List* list);

int ListGetSize(List* list);

#endif //GAME_LIST_H
