//
// Created by droc101 on 4/21/2024.
//

#include "List.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../defines.h"
#include "Error.h"

Node *CreateListNode(void *data)
{
	Node *newNode = malloc(sizeof(Node));
	chk_malloc(newNode);
	newNode->data = data;
	newNode->prev = NULL;
	newNode->next = NULL;
	return newNode;
}

List *CreateList()
{
	List *newList = malloc(sizeof(List));
	chk_malloc(newList);
	newList->head = NULL;
	newList->tail = NULL;
	newList->size = 0;
	return newList;
}

void ListAdd(List *list, void *data)
{
	Node *newNode = CreateListNode(data);
	if (list->head == NULL)
	{
		list->head = newNode;
		list->tail = newNode;
	} else
	{
		list->tail->next = newNode;
		newNode->prev = list->tail;
		list->tail = newNode;
	}
	list->size++;
}

void ListRemove(List *list, Node *node)
{
	if (node == NULL)
	{
		return;
	}

	if (node->prev != NULL)
	{
		node->prev->next = node->next;
	} else
	{
		list->head = node->next;
	}

	if (node->next != NULL)
	{
		node->next->prev = node->prev;
	} else
	{
		list->tail = node->prev;
	}
	list->size--;
	free(node);
	node = NULL;
}

void ListRemoveAt(List *list, const int index)
{
	Node *current = list->head;
	int i = 0;
	while (current != NULL && i < index)
	{
		current = current->next;
		i++;
	}
	if (current == NULL)
	{
		Error("List: Index out of bounds");
	}
	ListRemove(list, current);
}

void ListInsertAfter(List *list, Node *prevNode, void *data)
{
	if (prevNode == NULL)
	{
		Error("List: Previous node is NULL");
	}

	Node *newNode = CreateListNode(data);
	newNode->next = prevNode->next;
	newNode->prev = prevNode;
	if (prevNode->next != NULL)
	{
		prevNode->next->prev = newNode;
	} else
	{
		list->tail = newNode;
	}
	prevNode->next = newNode;
	list->size++;
}

void *ListGet(const List *list, const int index)
{
	const Node *current = list->head;
	int i = 0;
	while (current != NULL && i < index)
	{
		current = current->next;
		i++;
	}
	if (current == NULL)
	{
		Error("List: Index out of bounds");
	}
	return current->data;
}

void ListFree(List *list)
{
	Node *current = list->head;
	while (current != NULL)
	{
		Node *next = current->next;
		free(current);
		current = next;
	}
	free(list);
	list = NULL;
}

void ListFreeWithData(List *list)
{
	Node *current = list->head;
	while (current != NULL)
	{
		Node *next = current->next;
		free(current->data); // free the node's data too
		free(current);
		current = next;
	}
	free(list);
	list = NULL;
}

inline int ListGetSize(const List *list)
{
	return list->size;
}

int ListFind(const List *list, const void *data)
{
	const Node *current = list->head;
	int i = 0;
	while (current != NULL)
	{
		if (current->data == data)
		{
			return i;
		}
		current = current->next;
		i++;
	}
	return -1;
}

SizedArray *ToSizedArray(const List *list)
{
	SizedArray *array = malloc(sizeof(SizedArray));
	chk_malloc(array);
	array->size = list->size;
	array->elements = malloc(list->size * sizeof(void *));
	chk_malloc(array->elements);

	for (int i = 0; i < list->size; i++)
	{
		array->elements[i] = ListGet(list, i);
	}

	return array;
}

void DestroySizedArray(SizedArray *array)
{
	free(array->elements);
	free(array);
	array = NULL;
}

void ListClear(List *list)
{
	Node *current = list->head;
	while (current != NULL)
	{
		Node *next = current->next;
		free(current);
		current = next;
	}
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}
