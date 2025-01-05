//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_ASSETREADER_H
#define GAME_ASSETREADER_H

#include "../defines.h"

/**
 * Initialize the asset cache
 */
void AssetCacheInit();

/**
 * Invalidate the asset cache
 * @warning If anything still has a pointer to an asset, it will become invalid!
 */
void InvalidateAssetCache();

/**
 * Decompress an asset and cache it
 * @param relPath The asset to decompress
 * @return Decompressed asset, including header
 * @note The asset is cached
 */
Asset *DecompressAsset(const char *relPath);

/**
 * Load a model from an asset
 * @param asset The asset to load the model from
 * @return The loaded model, or NULL if it failed
 */
Model *LoadModel(const char *asset);

/**
 * Free a model
 * @param model The model to free
 */
void FreeModel(Model *model);

#define TEXTURE(x) ("texture/" x ".gtex")
#define MUSIC(x) ("audio/" x ".gmus")
#define SOUND(x) ("audio/" x ".gsnd")
#define LEVEL(x) ("level/" x ".gmap")
#define OGL_SHADER(x) ("glshader/" x ".gshd")
#define VK_FRAG(x) ("vkshader/" x ".gfrg")
#define VK_VERT(x) ("vkshader/" x ".gvert")
#define MODEL(x) ("model/" x ".gmdl")

#define ASSET_TYPE_TEXTURE 0
#define ASSET_TYPE_MP3 1
#define ASSET_TYPE_WAV 2
#define ASSET_TYPE_LEVEL 3
#define ASSET_TYPE_GLSL 4
// ... vulkan branch stuff 5 - 6
#define ASSET_TYPE_MODEL 7

#endif //GAME_ASSETREADER_H
