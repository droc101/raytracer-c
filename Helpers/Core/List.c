//
// Created by droc101 on 4/21/2024.
//

#include "List.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../defines.h"
#include "Error.h"
#include "Logging.h"

void ListCreate(List *list, const size_t slots)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListCreate!");
	}

	list->allocatedSlots = slots;
	list->usedSlots = slots;
	if (slots == 0)
	{
		list->data = NULL;
		return;
	}
	list->data = calloc(slots, sizeof(void *));
	chk_malloc(list->data);
}

void ListAdd(List *list, void *data)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListAdd!");
	}

	if (list->usedSlots >= list->allocatedSlots || !list->data)
	{
		list->allocatedSlots = list->usedSlots + 1;
		list->data = GameReallocArray(list->data, list->allocatedSlots, sizeof(void *));
		chk_malloc(list->data);
	}
	list->data[list->usedSlots] = data;
	list->usedSlots++;
}

void ListAddBatched(List *list, const size_t count, ...)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListAddBatched!");
	}

	if (list->usedSlots + count > list->allocatedSlots || !list->data)
	{
		list->allocatedSlots = list->usedSlots + count;
		list->data = GameReallocArray(list->data, list->allocatedSlots, sizeof(void *));
		chk_malloc(list->data);
	}

	va_list args;
	va_start(args, count);
	for (size_t i = 0; i < count; i++)
	{
		list->data[list->usedSlots] = va_arg(args, void *);
		list->usedSlots++;
	}
	va_end(args);
}

void ListRemoveAt(List *list, const size_t index)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListRemoveAt!");
	}
	if (!list->usedSlots || !list->allocatedSlots || !list->data)
	{
		Error("Attempted to shrink empty list!");
	}
	if (index >= list->usedSlots)
	{
		Error("Attempted to remove an item past the end of a list!");
	}

	list->allocatedSlots--;
	if (list->allocatedSlots == 0)
	{
		list->usedSlots = 0;
		free(list->data);
		list->data = NULL;
		return;
	}

	list->usedSlots--;
	memmove(&list->data[index], &list->data[index + 1], sizeof(void *) * (list->usedSlots - index));
	list->data = GameReallocArray(list->data, list->allocatedSlots, sizeof(void *));
	chk_malloc(list->data);
}

void ListInsertAfter(List *list, size_t index, void *data)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListInsertAfter!");
	}
	if (index >= list->usedSlots)
	{
		Error("Attempted to insert an item past the end of a list!");
	}

	index++;
	list->allocatedSlots++;
	list->data = GameReallocArray(list->data, list->allocatedSlots, sizeof(void *));
	chk_malloc(list->data);

	memmove(&list->data[index + 1], &list->data[index], sizeof(void *) * (list->usedSlots - index));
	list->data[index] = data;
}

size_t ListFind(const List *list, const void *data)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListFind!");
	}
	if (!list->usedSlots || !list->allocatedSlots || !list->data)
	{
		Error("Attempted to find a value in an empty list!");
	}

	for (size_t i = 0; i < list->usedSlots; i++)
	{
		if (list->data[i] == data)
		{
			return i;
		}
	}
	return -1;
}

void ListFree(List *list, const bool freeListPointer)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListFree!");
	}

	free(list->data);
	if (freeListPointer)
	{
		free(list);
	}
}

void ListFreeOnlyContents(List *list)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListFreeOnlyContents!");
	}

	if (list->usedSlots && list->data)
	{
		for (size_t i = 0; i < list->usedSlots; i++)
		{
			free(list->data[i]);
		}
	}
}

void ListAndContentsFree(List *list, const bool freeListPointer)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListAndContentsFree!");
	}

	ListFreeOnlyContents(list);

	ListFree(list, freeListPointer);
}

void ListClear(List *list)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListClear!");
	}

	list->allocatedSlots = 0;
	list->usedSlots = 0;
	free(list->data);
	list->data = NULL;
}

void *GameReallocArray(void *ptr, const size_t arrayLength, const size_t elementSize)
{
	if (elementSize == 0)
	{
		LogWarning("GameReallocArray: elementSize is zero, returning NULL");
		return NULL;
	}
	if (arrayLength > SIZE_MAX / elementSize)
	{
		LogWarning("GameReallocArray: arrayLength * elementSize exceeds SIZE_MAX, returning NULL");
		errno = ENOMEM;
		return NULL;
	}
	return realloc(ptr, arrayLength * elementSize);
}
