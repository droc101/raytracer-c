//
// Created by droc101 on 6/22/2024.
//

#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

#include "../defines.h"

/**
 * Check if a point is inside a cylinder
 * @param cylOrigin Center of the cylinder
 * @param cylRadius Radius of the cylinder
 * @param testPoint Point to test
 * @return Whether the point is inside the cylinder
 */
bool CollideCylinder(Vector2 cylOrigin, double cylRadius, Vector2 testPoint);

/**
 * Check if a point is inside an actor's cylinder
 * @param a Actor to check
 * @param testPoint Point to test
 * @return Whether the point is inside the actor's cylinder
 * @note The cylinder is defined by the actor's wall
 */
bool CollideActorCylinder(const Actor *a, Vector2 testPoint);

#endif //GAME_COLLISION_H
