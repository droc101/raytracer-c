//
// Created by droc101 on 4/26/2024.
//

#include "AssetReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "Assets.h"
#include "../Helpers/Core/DataReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"

uint AssetGetSize(const byte *asset)
{
    return ReadUintA(asset, 4);
}

uint AssetGetType(const byte *asset)
{
    return ReadUintA(asset, 12);
}

byte *AssetCache[ASSET_COUNT];

void InvalidateAssetCache()
{
    for (int i = 0; i < ASSET_COUNT; i++)
    {
        if (AssetCache[i] != NULL)
        {
            free(AssetCache[i]);
            AssetCache[i] = NULL;
        }
    }
}

byte *DecompressAsset(const byte *asset)
{
    int offset = 0;
    // Read the first 4 bytes of the asset to get the size of the compressed data
    const uint compressedSize = ReadUint(asset, &offset);
    const uint decompressedSize = ReadUint(asset, &offset);
    // Read the decompressed size (4 bytes after the compressed size
    const uint assetId = ReadUint(asset, &offset); // Read the asset ID (4 bytes after the decompressed size)
    //uint type = ReadUint(asset, &offset); // Read the asset type (4 bytes after the asset ID)

    if (assetId >= ASSET_COUNT)
    {
        LogError("Asset ID %d is out of range\n", assetId);
        Error("Asset ID out of range");
    }

    if (AssetCache[assetId] != NULL)
    {
        return AssetCache[assetId];
    }

    asset += 16; // skip header

    // Allocate memory for the decompressed data
    byte *decompressedData = malloc(decompressedSize);

    z_stream stream = {0};

    // Initialize the zlib stream
    stream.next_in = (Bytef *) asset;
    stream.avail_in = compressedSize;
    stream.next_out = decompressedData;
    stream.avail_out = decompressedSize;

    // Initialize the zlib stream
    if (inflateInit2(&stream, MAX_WBITS | 16) != Z_OK)
    {
        free(decompressedData);
        decompressedData = NULL;
        LogError("Failed to initialize zlib stream: %s\n", stream.msg);
        Error("Failed to initialize zlib stream");
    }

    // Decompress the data
    int ret;
    do
    {
        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END)
        {
            free(decompressedData);
            decompressedData = NULL;
            LogError("Failed to decompress zlib stream: %s\n", stream.msg);
            Error("Failed to decompress zlib stream");
        }
    } while (ret != Z_STREAM_END);

    // Clean up the zlib stream
    inflateEnd(&stream);

    AssetCache[assetId] = decompressedData;

    return decompressedData;
}

Model *LoadModel(const byte *asset)
{
    const size_t size = AssetGetSize(asset);
    if (size < sizeof(ModelHeader))
    {
        LogError("Failed to load model from asset, size was too small!");
        return NULL;
    }
    Model *model = malloc(sizeof(Model));
    const byte *assetData = DecompressAsset(asset);
    memcpy(&model->header, assetData, sizeof(ModelHeader));

    if (strcmp(model->header.sig, "MSH") != 0)
    {
        LogError("Tried to load a model, but its first magic was incorrect (got %s)!", model->header.sig);
        free(model);
        return NULL;
    }

    if (strcmp(model->header.dataSig, "DAT") != 0)
    {
        LogError("Tried to load a model, but its second magic was incorrect (got %s)!", model->header.dataSig);
        free(model);
        return NULL;
    }

    const size_t vertsSizeBytes = model->header.indexCount * (sizeof(float) * 8);
    const size_t indexSizeBytes = model->header.indexCount * sizeof(uint);

    model->packedVertsUvsCount = model->header.indexCount;
    model->packedIndicesCount = model->header.indexCount;

    model->packedVertsUvs = malloc(vertsSizeBytes);
    model->packedIndices = malloc(indexSizeBytes);

    memcpy(model->packedVertsUvs, (byte *) assetData + sizeof(ModelHeader), vertsSizeBytes);

    for (int i = 0; i < model->header.indexCount; i++)
    {
        model->packedIndices[i] = i;
    }

    return model;
}

void FreeModel(Model *model)
{
    if (model == NULL)
    {
        LogWarning("Tried to free NULL model!");
        return;
    }
    free(model->packedVertsUvs);
    free(model->packedIndices);
    free(model);
}
