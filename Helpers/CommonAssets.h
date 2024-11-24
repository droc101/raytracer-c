//
// Created by droc101 on 7/7/2024.
//

#ifndef GAME_COMMONASSETS_H
#define GAME_COMMONASSETS_H

#include <SDL.h>
#include "../defines.h"

#define WALL_TEXTURE_COUNT 5
#define ACTOR_TEXTURE_COUNT 12

extern const byte *wallTextures[WALL_TEXTURE_COUNT];
extern const byte *actorTextures[ACTOR_TEXTURE_COUNT];

extern Model *skyModel;

/**
 * Initialize common assets
 */
void InitCommonAssets();

/**
 * Find the index of a wall texture
 * @param tex The texture to find
 * @return The index of the texture
 */
int FindWallTextureIndex(const byte *tex);

/**
 * Free any common assets that were allocated
 */
void DestroyCommonAssets();

#endif //GAME_COMMONASSETS_H
