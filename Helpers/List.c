//
// Created by droc101 on 4/21/2024.
//

#include <stdlib.h>
#include <stdio.h>
#include "List.h"

Node* createNode(void *data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("List: malloc fail\n");
        exit(1);
    }
    newNode->data = data;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

List* CreateList() {
    List* newList = (List*)malloc(sizeof(List));
    if (newList == NULL) {
        printf("List: malloc fail\n");
        exit(1);
    }
    newList->head = NULL;
    newList->tail = NULL;
    newList->size = 0;
    return newList;
}

void ListAdd(List* list, void *data) {
    Node* newNode = createNode(data);
    if (list->head == NULL) {
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
    if (node == NULL)
        return;

    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        list->head = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    list->size--;
    free(node);
}

void ListInsertAfter(List* list, Node* prevNode, void *data) {
    if (prevNode == NULL) {
        printf("List: Previous node cannot be NULL\n");
        return;
    }

    Node* newNode = createNode(data);
    newNode->next = prevNode->next;
    newNode->prev = prevNode;
    if (prevNode->next != NULL)
        prevNode->next->prev = newNode;
    else
        list->tail = newNode;
    prevNode->next = newNode;
    list->size++;
}

void* ListGet(List* list, int index) {
    Node* current = list->head;
    int i = 0;
    while (current != NULL && i < index) {
        current = current->next;
        i++;
    }
    if (current == NULL) {
        printf("List: Index out of bounds\n");
        exit(1);
    }
    return current->data;
}

void ListFree(List* list) {
    Node* current = list->head;
    while (current != NULL) {
        Node* next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

void ListFreeWithData(List* list) {
    Node* current = list->head;
    while (current != NULL) {
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
