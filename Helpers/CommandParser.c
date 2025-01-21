//
// Created by droc101 on 1/18/25.
//

#include "CommandParser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../GameStates/GLoadingState.h"
#include "../GameStates/GMainState.h"
#include "../Structs/GlobalState.h"
#include "Core/List.h"


void ExecuteCommand(const char *command)
{
	// need to strdup because strtok_r modifies the string (removing results in sigsegv)
	// no, passing it as non-const does not work
	char *rwCommand = strdup(command);
	List *commandList = CreateList();
	char *token = strtok(rwCommand, " ");
	while (token != NULL)
	{
		ListAdd(commandList, strdup(token));
		token = strtok(NULL, " ");
	}
	free(rwCommand);

	char *commandName = ListGet(commandList, 0);
	if (strcmp(commandName, "level") == 0)
	{
		if (commandList->size < 2)
		{
			printf("level command requires a level name\n");
		} else
		{
			GLoadingSelectStateSet((char*)ListGet(commandList, 1));
		}
	}
	// TODO: signal system and related commands
	else
	{
		printf("Unknown command: %s\n", commandName);
	}

	ListFreeWithData(commandList);
}