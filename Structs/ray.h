//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_RAY_H
#define GAME_RAY_H

#include "../defines.h"
#include <math.h>
#include "Vector2.h"

// Perform a raycast against a wall
RayCastResult Intersect(Wall wall, Vector2 from, double direction);

// Perform a raycast against a level (returning the closest hit)
RayCastResult HitscanLevel(Level l, Vector2 pos, double angle, bool scanWalls, bool scanActors, bool alwaysCollideActors);

#endif //GAME_RAY_H
