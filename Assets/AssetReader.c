//
// Created by droc101 on 4/26/2024.
//

#include "AssetReader.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include "../Helpers/Error.h"
#include "../Helpers/LevelLoader.h" // for ReadUInt

byte *DecompressAsset(byte *asset) {
    int offset = 0;
    // Read the first 4 bytes of the asset to get the size of the compressed data
    uint compressedSize = ReadUint(asset, &offset);
    uint decompressedSize = ReadUint(asset, &offset); // Read the decompressed size (4 bytes after the compressed size

    //printf("Compressed size: %d\n", compressedSize);
    //printf("Decompressed size: %d\n", decompressedSize);

    // Skip the first 8 bytes (4 for compressed size, 4 for decompressed size)
    asset += 8;

    //printf("First few bytes of compressed data: %x %x %x %x\n", asset[0], asset[1], asset[2], asset[3]);

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

    //printf("First few bytes of decompressed data: %x %x %x %x\n", decompressedData[0], decompressedData[1], decompressedData[2], decompressedData[3]);

    return decompressedData;
}
