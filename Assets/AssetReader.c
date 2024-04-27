//
// Created by droc101 on 4/26/2024.
//

#include "AssetReader.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include "../Helpers/Error.h"
#include "../Helpers/DataReader.h"
#include "Assets.h"

uint AssetGetSize(const byte *asset) {
    int offset = 0;
    return ReadUint(asset, &offset);
}

byte *DecompressAsset(const byte *asset) {
    int offset = 0;
    // Read the first 4 bytes of the asset to get the size of the compressed data
    uint compressedSize = ReadUint(asset, &offset);
    uint decompressedSize = ReadUint(asset, &offset); // Read the decompressed size (4 bytes after the compressed size
    uint assetId = ReadUint(asset, &offset); // Read the asset ID (4 bytes after the decompressed size)
    uint type = ReadUint(asset, &offset); // Read the asset type (4 bytes after the asset ID)

    if (assetId >= ASSET_COUNT) {
        printf("Asset ID %d is out of range\n", assetId);
        Error("Asset ID out of range");
        return NULL;
    }
    asset += 16; // skip header

    // Allocate memory for the decompressed data
    byte *decompressedData = (byte *)malloc(decompressedSize);

    z_stream stream;
    memset(&stream, 0, sizeof(stream));

    // Initialize the zlib stream
    stream.next_in = (Bytef *) (asset);
    stream.avail_in = compressedSize;
    stream.next_out = (Bytef *) decompressedData;
    stream.avail_out = decompressedSize;

    // Initialize the zlib stream
    if (inflateInit2(&stream, MAX_WBITS | 16) != Z_OK) {
        free(decompressedData);
        printf("Failed to initialize zlib stream: %s\n", stream.msg);
        Error("Failed to initialize zlib stream");
        return NULL;
    }

    // Decompress the data
    int ret;
    do {
        ret = inflate(&stream, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            free(decompressedData);
            printf("Failed to decompress zlib stream: %s\n", stream.msg);
            Error("Failed to decompress zlib stream");
            return NULL;
        }
    } while (ret != Z_STREAM_END);

    // Clean up the zlib stream
    inflateEnd(&stream);

    return decompressedData;
}
