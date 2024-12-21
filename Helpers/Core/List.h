//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_LIST_H
#define GAME_LIST_H

typedef struct List List;

struct List
{
	int size;
	void **data;
};

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
 * Remove an item from the list by index
 * @param list List to remove from
 * @param index Index to remove
 */
void ListRemoveAt(List *list, int index);

/**
 * Insert an item after a node
 * @param list List to insert into
 * @param index Index to insert after
 * @param data Data to insert
 */
void ListInsertAfter(List *list, int index, void *data);

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
 * Get an item from the list by index
 * @param list The list to get from
 * @param index The index to get
 */
#define ListGet(list, index) (list)->data[(index)]

#endif //GAME_LIST_H
