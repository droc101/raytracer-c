//
// Created by droc101 on 7/7/2024.
//

#ifndef GAME_COMMONASSETS_H
#define GAME_COMMONASSETS_H

#include "../defines.h"

#define WALL_TEXTURE_COUNT 5
#define ACTOR_TEXTURE_COUNT 12

extern const char *wallTextures[WALL_TEXTURE_COUNT];
extern const char *actorTextures[ACTOR_TEXTURE_COUNT];

extern Model *skyModel;

/**
 * Initialize common assets
 */
void InitCommonAssets();

/**
 * Find the index of a wall texture
 * @param texture The texture to find
 * @return The index of the texture
 */
int FindWallTextureIndex(const char *texture);

/**
 * Free any common assets that were allocated
 */
void DestroyCommonAssets();

#endif //GAME_COMMONASSETS_H
