//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_ASSETREADER_H
#define GAME_ASSETREADER_H

#include "../defines.h"

/**
 * Get the DECOMPRESSED size of the asset
 * @param asset The compressed asset to get the size of
 * @return The decompressed size of the asset, header included
 * @note Subtract 16 bytes if you don't want the header
 */
uint AssetGetSize(const byte *asset);

/**
 * Get the type of the asset
 * @param asset The asset to check the type of
 * @return The asset's type
 * @see ASSET_TYPE_* defines
 */
uint AssetGetType(const byte *asset);

/**
 * Invalidate the asset cache
 * @warning If anything still has a pointer to an asset, it will become invalid!
 */
void InvalidateAssetCache();

/**
 * Decompress an asset and cache it
 * @param asset The asset to decompress
 * @return Decompressed asset, including header
 * @note The asset is cached
 */
byte *DecompressAsset(const byte *asset);

/**
 * Load a model from an asset
 * @param asset The asset to load the model from
 * @return The loaded model, or NULL if it failed
 */
Model *LoadModel(const byte *asset);

/**
 * Free a model
 * @param model The model to free
 */
void FreeModel(Model *model);

#define ASSET_TYPE_TEXTURE 0
#define ASSET_TYPE_MP3 1
#define ASSET_TYPE_WAV 2
#define ASSET_TYPE_LEVEL 3
#define ASSET_TYPE_GLSL 4
#define ASSET_TYPE_SPIRV_FRAG 5
#define ASSET_TYPE_SPIRV_VERT 6
#define ASSET_TYPE_MODEL 7

#endif //GAME_ASSETREADER_H
