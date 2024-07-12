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

/**
 * Perform a raycast against a level
 * @param l Level to check
 * @param pos Ray origin
 * @param angle Ray angle
 * @param scanWalls Whether to scan walls
 * @param scanActors Whether to scan actors
 * @param alwaysCollideActors Whether to always collide actors
 * @return @c RayCastResult with hit information
 */
RayCastResult HitscanLevel(Level l, Vector2 pos, double angle, bool scanWalls, bool scanActors, bool alwaysCollideActors);

#endif //GAME_RAY_H
