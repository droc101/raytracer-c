//
// Created by droc101 on 1/24/25.
//
// This file is a library for the level editor to get information about actors.
//

#include "../defines.h"

char *ActorNames[] = {
	"NullActor",
	"TestActor",
	"Coin",
	"Goal",
	"Door",
};

// Array of actor parameter names
// Each actor type has 4 parameters
char ActorParamNames[][4][16] = {
	{"N/A", "N/A", "N/A", "N/A"},
	{"N/A", "N/A", "N/A", "N/A"},
	{"Anim Frame", "Blue Coin?", "N/A", "N/A"},
	{"N/A", "N/A", "N/A", "N/A"},
	{"N/A", "N/A", "N/A", "N/A"},
};

/**
 * Get the name of an actor type
 * @param actor The actor type
 * @return The name of the actor type, or "Invalid Actor!" if the actor type is invalid
 */
EXPORT_SYM char *GetActorName(const int actor)
{
	const int actorNameCount = sizeof(ActorNames) / sizeof(char *);
	if (actor > actorNameCount - 1)
	{
		return "Invalid Actor!";
	}
	return ActorNames[actor];
}

/**
 * Get the name of an actor's parameter
 * @param actor The actor type
 * @param param The parameter index (0-3)
 * @return The name of the parameter, or "Invalid Actor!" if the actor type is invalid
 */
EXPORT_SYM char *GetActorParamName(const int actor, const byte param)
{
	const int actorNameCount = sizeof(ActorNames) / sizeof(char *);
	if (actor > actorNameCount - 1)
	{
		return "Invalid Actor!";
	}
	return ActorParamNames[actor][param];
}

/**
 * Get the number of Actor types
 * @return Actor type count
 */
EXPORT_SYM int GetActorTypeCount()
{
	return sizeof(ActorNames) / sizeof(char*);
}
