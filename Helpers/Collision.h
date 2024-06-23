//
// Created by droc101 on 6/22/2024.
//

#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include "../defines.h"

// Move around in the world, accounting for solid walls
Vector2 Move(Vector2 position, Vector2 moveVec, void *ignore);

// Check if a point is inside a cylinder
bool CollideCylinder(Vector2 cylOrigin, double cylRadius, Vector2 testPoint);

// Check if a point is inside an actor's cylinder
bool CollideActorCylinder(Actor *a, Vector2 testPoint);

#endif //GAME_COLLISION_H
