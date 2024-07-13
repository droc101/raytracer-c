//
// Created by droc101 on 4/21/2024.
//

#ifndef GAME_WALL_H
#define GAME_WALL_H

#include "../Assets/Assets.h"

/**
 * Get the number of available wall textures
 * @return
 */
int GetTextureCount();

/**
 * Load a wall texture by index
 * @param index index of the texture
 * @return Wall texture as @c SDL_Texture
 */
SDL_Texture *LoadWallTexture(int index);

/**
 * Create a wall
 * @param a Wall start point
 * @param b Wall end point
 * @param tex Wall texture
 * @param uvScale Wall texture scale
 * @param uvOffset Wall texture offset
 * @return Wall pointer
 */
Wall *CreateWall(Vector2 a, Vector2 b, SDL_Texture *tex, float uvScale, float uvOffset);

/**
 * Free the memory used by a wall
 * @param w Wall to free
 * @warning This function also frees the texture used by the wall
 */
void FreeWall(Wall *w);

// How far out the hitbox of the wall extends from the actual wall (on both sides)
#define WALL_HITBOX_EXTENTS 0.4

/**
 * Bake a wall's information
 * @param w Wall to bake
 * @return Wall length...for some reason
 */
double WallBake(Wall *w);

#endif //GAME_WALL_H
