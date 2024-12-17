//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LIST_H
#define GAME_LIST_H

typedef struct Node Node;
typedef struct List List;
typedef struct SizedArray SizedArray;

// List item
struct Node
{
    void *data;
    Node *prev;
    Node *next;
};

// Doubly linked list
struct List
{
    Node *head;
    Node *tail;
    int size;
};

struct SizedArray
{
    void **elements;
    int size;
};

// Internal functions
Node *CreateListNode(void *data);

// Public functions

/**
 * Create a new list
 * @return brand-new list just for you
 */
List *CreateList();

/**
 * Append an item to the list
 * @param list List to append to
 * @param data Data to append
 */
void ListAdd(List *list, void *data);

/**
 * Remove a node from the list
 * @param list List to remove from
 * @param node List node (NOT DATA) to remove
 */
void ListRemove(List *list, Node *node);

/**
 * Remove an item from the list by index
 * @param list List to remove from
 * @param index Index to remove
 */
void ListRemoveAt(List *list, int index);

/**
 * Insert an item after a node
 * @param list List to insert into
 * @param prevNode Node to insert after
 * @param data Data to insert
 */
void ListInsertAfter(List *list, Node *prevNode, void *data);

/**
 * Get an item from the list by index
 * @param list List to get from
 * @param index Index to get
 * @return Data at index (not node)
 */
void *ListGet(const List *list, int index);

/**
 * Free the list structure
 * @param list List to free
 * @warning This does not free the data in the list
 */
void ListFree(List *list);

/**
 * Free the list structure and the data in the list
 * @param list List to free
 * @warning If the data is a struct, any pointers in the struct will not be freed, just the struct itself
 */
void ListFreeWithData(List *list);

/**
 * Get the size of the list
 * @param list List to get size of
 * @return Number of items in the list
 */
int ListGetSize(const List *list);

/**
 * Find an item in the list
 * @param list List to search
 * @param data Data to search for
 * @return Index of the item in the list, -1 if not found
 */
int ListFind(const List *list, const void *data);

/**
 * Clear all items from the list
 * @param list List to clear
 * @warning This does not free the data in the list
 */
void ListClear(List *list);

/**
 * Create a sized array from a list
 * @param list List to convert
 * @return Sized array
 */
SizedArray *ToSizedArray(const List *list);

/**
 * Get an element from a sized array
 * @param array The @c SizedArray to get the element from
 * @param index The index of the element to get
 * @warning This does no safety checks, make sure the index is valid
 */
#define SizedArrayGet(array, index) ((array)->elements[(index)])

/**
 * Free a sized array
 * @param array Array to free
 */
void DestroySizedArray(SizedArray *array);

#endif //GAME_LIST_H
