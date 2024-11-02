//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_RAY_H
#define GAME_RAY_H

#include "../defines.h"
#include <math.h>
#include "Vector2.h"

/**
 * Perform a raycast against a wall
 * @param wall Wall to check
 * @param from Ray origin
 * @param direction Ray direction
 * @return @c RayCastResult with hit information
 */
RayCastResult Intersect(Wall wall, Vector2 from, double direction);

#endif //GAME_RAY_H
