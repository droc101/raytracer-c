//
// Created by droc101 on 4/21/2024.
//

#include <stdlib.h>
#include <stdio.h>
#include "List.h"
#include "../../defines.h"
#include "Error.h"

Node* createNode(void *data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULLPTR) {
        Error("Node: malloc fail");
    }
    newNode->data = data;
    newNode->prev = NULLPTR;
    newNode->next = NULLPTR;
    return newNode;
}

List* CreateList() {
    List* newList = (List*)malloc(sizeof(List));
    if (newList == NULLPTR) {
        Error("List: malloc fail");
    }
    newList->head = NULLPTR;
    newList->tail = NULLPTR;
    newList->size = 0;
    return newList;
}

void ListAdd(List* list, void *data) {
    Node* newNode = createNode(data);
    if (list->head == NULLPTR) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        newNode->prev = list->tail;
        list->tail = newNode;
    }
    list->size++;
}

void ListRemove(List* list, Node* node) {
    if (node == NULLPTR)
        return;

    if (node->prev != NULLPTR)
        node->prev->next = node->next;
    else
        list->head = node->next;

    if (node->next != NULLPTR)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    list->size--;
    free(node);
}

void ListRemoveAt(List *list, int index) {
    Node* current = list->head;
    int i = 0;
    while (current != NULLPTR && i < index) {
        current = current->next;
        i++;
    }
    if (current == NULLPTR) {
        Error("List: Index out of bounds");
    }
    ListRemove(list, current);
}

void ListInsertAfter(List* list, Node* prevNode, void *data) {
    if (prevNode == NULLPTR) {
        Error("List: Previous node is NULL");
    }

    Node* newNode = createNode(data);
    newNode->next = prevNode->next;
    newNode->prev = prevNode;
    if (prevNode->next != NULLPTR)
        prevNode->next->prev = newNode;
    else
        list->tail = newNode;
    prevNode->next = newNode;
    list->size++;
}

void* ListGet(List* list, int index) {
    Node* current = list->head;
    int i = 0;
    while (current != NULLPTR && i < index) {
        current = current->next;
        i++;
    }
    if (current == NULLPTR) {
        Error("List: Index out of bounds");
    }
    return current->data;
}

void ListFree(List* list) {
    Node* current = list->head;
    while (current != NULLPTR) {
        Node* next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

void ListFreeWithData(List* list) {
    Node* current = list->head;
    while (current != NULLPTR) {
        Node* next = current->next;
        free(current->data); // free the node's data too
        free(current);
        current = next;
    }
    free(list);
}

int ListGetSize(List* list) {
    return list->size;
}

int ListFind(List *list, void *data) {
    Node* current = list->head;
    int i = 0;
    while (current != NULLPTR) {
        if (current->data == data) {
            return i;
        }
        current = current->next;
        i++;
    }
    return -1;
}

SizedArray *ToSizedArray(List *list) {
    SizedArray *array = malloc(sizeof(SizedArray));
    array->size = list->size;
    array->elements = malloc(list->size * sizeof(void*));

    for (int i = 0; i < list->size; i++) {
        array->elements[i] = ListGet(list, i);
    }

    return array;
}

void DestroySizedArray(SizedArray *array) {
    free(array->elements);
    free(array);
}

void ListClear(List *list) {
    Node* current = list->head;
    while (current != NULLPTR) {
        Node* next = current->next;
        free(current);
        current = next;
    }
    list->head = NULLPTR;
    list->tail = NULLPTR;
    list->size = 0;
}

