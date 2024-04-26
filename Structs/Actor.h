//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include "../defines.h"
#include "../Structs/Vector2.h"

// Create a new actor and return a pointer to it
Actor *CreateActor(Vector2 position, double rotation, int actorType);

// Free the actor from memory
void FreeActor(Actor *actor);

// Get the actor's wall transformed by the actor's position and rotation
Wall GetTransformedWall(Actor *actor);

#endif //GAME_ACTOR_H
