//
// Created by droc101 on 4/22/2024.
//

#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include "../defines.h"
#include "../Structs/Vector2.h"

Actor *CreateActor(Vector2 position, double rotation, int actorType);

void FreeActor(Actor *actor);

Wall GetTransformedWall(Actor *actor);

#endif //GAME_ACTOR_H
