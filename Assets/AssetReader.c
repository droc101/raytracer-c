//
// Created by droc101 on 4/26/2024.
//

#include "AssetReader.h"
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "../Helpers/Core/DataReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"
// #include "Assets.h"
#include <errno.h>


#include "../Structs/GlobalState.h"

FILE *openAssetFile(const char *relPath)
{
	char path[260] = {0};

	const size_t pathLen = strlen(GetState()->executableFolder) + strlen(relPath) + strlen("assets/") + 1;
	if (pathLen >= 260)
	{
		LogError("Path is too long: %s\n", relPath);
		Error("Game path too long. Please rethink your file structure.");
	}
	sprintf(path, "%sassets/%s", GetState()->executableFolder, relPath);

	FILE *file = fopen(path, "rb");
	if (file == NULL)
	{
		LogError("Failed to open asset file: %s with errno %s\n", path, strerror(errno));
		Error("Failed to open asset file. The game installation may be damaged.");
	}

	return file;
}

uint AssetGetSize(const char *relPath)
{
	FILE *file = openAssetFile(relPath);

	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);
	if (fileSize < 16)
	{
		LogError("Failed to get asset size, file was too small!");
		fclose(file);
		Error("Failed to read asset file. The game installation may be damaged.");
	}
	fseek(file, 0, SEEK_SET);

	byte header[16];
	fread(header, 1, 16, file);
	fclose(file);
	uint size = ReadUintA(header, 4);
	return size;
}

uint AssetGetType(const char *relPath)
{
	FILE *file = openAssetFile(relPath);

	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);
	if (fileSize < 16)
	{
		LogError("Failed to get asset type, file was too small!");
		fclose(file);
		Error("Failed to read asset file. The game installation may be damaged.");
	}
	fseek(file, 0, SEEK_SET);

	byte header[16];
	fread(header, 1, 16, file);
	fclose(file);
	uint size = ReadUintA(header, 12);
	return size;
}

List *AssetCacheNames;
List *AssetCacheData;

void AssetCacheInit()
{
	AssetCacheNames = CreateList();
	AssetCacheData = CreateList();
}

void InvalidateAssetCache()
{
	ListFreeWithData(AssetCacheData);
	ListFree(AssetCacheNames);

	AssetCacheInit();
}

byte *DecompressAsset(const char *relPath)
{
	// see if relPath is already in the cache
	for (int i = 0; i < AssetCacheNames->size; i++)
	{
		if (strcmp(ListGet(AssetCacheNames, i), relPath) == 0)
		{
			return ListGet(AssetCacheData, i);
		}
	}

	FILE *file = openAssetFile(relPath);

	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);

	const byte *asset = malloc(fileSize);
	chk_malloc(asset);
	fseek(file, 0, SEEK_SET);
	fread((void *)asset, 1, fileSize, file);

	fclose(file);

	int offset = 0;
	// Read the first 4 bytes of the asset to get the size of the compressed data
	const uint compressedSize = ReadUint(asset, &offset);
	const uint decompressedSize = ReadUint(asset, &offset);

	asset += 16; // skip header

	// Allocate memory for the decompressed data
	byte *decompressedData = malloc(decompressedSize);
	chk_malloc(decompressedData);

	z_stream stream = {0};

	// Initialize the zlib stream
	stream.next_in = (Bytef *)asset;
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

	// Add the asset to the cache
	ListAdd(AssetCacheNames, strdup(relPath));
	ListAdd(AssetCacheData, decompressedData);

	return decompressedData;
}

Model *LoadModel(const char *asset)
{
	const size_t size = AssetGetSize(asset);
	if (size < sizeof(ModelHeader))
	{
		LogError("Failed to load model from asset, size was too small!");
		return NULL;
	}
	Model *model = malloc(sizeof(Model));
	chk_malloc(model);
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
	chk_malloc(model->packedVertsUvs);
	model->packedIndices = malloc(indexSizeBytes);
	chk_malloc(model->packedIndices);

	memcpy(model->packedVertsUvs, (byte *)assetData + sizeof(ModelHeader), vertsSizeBytes);

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
