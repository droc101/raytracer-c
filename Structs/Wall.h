//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_WALL_H
#define GAME_WALL_H

#include "../defines.h"

/// How far out the hitbox of the wall extends from the actual wall (on both sides)
#define WALL_HITBOX_EXTENTS 0.2

/**
 * Create a wall
 * @param a Wall start point
 * @param b Wall end point
 * @param texture Wall texture name
 * @param uvScale Wall texture scale
 * @param uvOffset Wall texture offset
 * @return Wall pointer
 */
Wall *CreateWall(Vector2 a, Vector2 b, const char *texture, float uvScale, float uvOffset);

/**
 * Bake a wall's information
 * @param w Wall to bake
 */
void WallBake(Wall *w);

#endif //GAME_WALL_H
