//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_RAY_H
#define GAME_RAY_H

#include "../defines.h"
#include <math.h>
#include "Vector2.h"

RayCastResult Intersect(Wall wall, Vector2 from, double direction);
RayCastResult HitscanLevel(Level l, Vector2 pos, double angle);

#endif //GAME_RAY_H
