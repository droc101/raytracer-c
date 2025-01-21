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

void ListCreate(List *list)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListCreate!");
	}

	list->length = 0;
	list->data = NULL;
}

void ListAdd(List *list, void *data)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListAdd!");
	}

	list->data = GameReallocArray(list->data, list->length + 1, sizeof(void *));
	chk_malloc(list->data);
	list->data[list->length] = data;
	list->length++;
}

void ListAddBatched(List *list, const size_t count, ...)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListAddBatched!");
	}

	list->data = GameReallocArray(list->data, list->length + count, sizeof(void *));
	chk_malloc(list->data);

	va_list args;
	va_start(args, count);
	for (size_t i = 0; i < count; i++)
	{
		list->data[list->length] = va_arg(args, void *);
		list->length++;
	}
	va_end(args);
}

void ListRemoveAt(List *list, const size_t index)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListRemoveAt!");
	}
	if (!list->length || !list->data)
	{
		Error("Attempted to shrink empty list!");
	}
	if (index >= list->length)
	{
		Error("Attempted to remove an item past the end of a list!");
	}

	list->length--;
	if (list->length == 0)
	{
		free(list->data);
		list->data = NULL;
		return;
	}
	memmove(&list->data[index], &list->data[index + 1], sizeof(void *) * (list->length - index));
	list->data = GameReallocArray(list->data, list->length, sizeof(void *));
	chk_malloc(list->data);
}

void ListInsertAfter(List *list, size_t index, void *data)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListInsertAfter!");
	}
	if (index >= list->length)
	{
		Error("Attempted to insert an item past the end of a list!");
	}

	index++;
	list->length++;
	list->data = GameReallocArray(list->data, list->length, sizeof(void *));
	chk_malloc(list->data);

	memmove(&list->data[index + 1], &list->data[index], sizeof(void *) * (list->length - index - 1));
	list->data[index] = data;
}

size_t ListFind(const List list, const void *data)
{
	if (!list.length || !list.data)
	{
		return -1;
	}

	for (size_t i = 0; i < list.length; i++)
	{
		if (list.data[i] == data)
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

void ListFreeOnlyContents(const List list)
{
	if (list.length && list.data)
	{
		for (size_t i = 0; i < list.length; i++)
		{
			free(list.data[i]);
		}
	}
}

void ListAndContentsFree(List *list, const bool freeListPointer)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListAndContentsFree!");
	}

	ListFreeOnlyContents(*list);

	ListFree(list, freeListPointer);
}

void ListClear(List *list)
{
	if (!list)
	{
		Error("A NULL list must not be passed to ListClear!");
	}

	list->length = 0;
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
