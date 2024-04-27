//
// Created by droc101 on 4/26/2024.
//

#ifndef GAME_ASSETREADER_H
#define GAME_ASSETREADER_H

#include "../defines.h"

// Get the **DECOMPRESSED** size of the asset
uint AssetGetSize(const byte *asset);

uint AssetGetType(const byte *asset);

// Dangerous! If anything is still using the asset, it will almost certainly crash!
void InvalidateAssetCache();

byte *DecompressAsset(const byte *asset);

#define ASSET_TYPE_TEXTURE 0
#define ASSET_TYPE_MP3 1
#define ASSET_TYPE_WAV 2

#endif //GAME_ASSETREADER_H
