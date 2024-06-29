//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include "../defines.h"
#include "../Structs/Vector2.h"

int GetActorTypeCount();

// Create a new Actor and return a pointer to it
Actor *CreateActor(Vector2 position, double rotation, int actorType, byte paramA, byte paramB, byte paramC, byte paramD);

// Free the Actor from memory
void FreeActor(Actor *actor);

// Get the Actor's wall transformed by the Actor's position and rotation
Wall GetTransformedWall(Actor *actor);

#endif //GAME_ACTOR_H
