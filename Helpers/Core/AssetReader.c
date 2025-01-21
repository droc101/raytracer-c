//
// Created by droc101 on 4/26/2024.
//

#include "AssetReader.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "../../Structs/GlobalState.h"
#include "DataReader.h"
#include "Error.h"
#include "Logging.h"

List assetCacheNames;
List assetCacheData;
TextureSizeTable *tsb;
uint textureId;
uint modelId;
Image *images[MAX_TEXTURES];
Model *models[MAX_MODELS];

FILE *OpenAssetFile(const char *relPath)
{
	const size_t maxPathLength = 300;
	char *path = calloc(maxPathLength, sizeof(char));
	chk_malloc(path);

	const size_t pathLen = strlen(GetState()->executableFolder) + strlen("assets/") + strlen(relPath) + 1;
	if (pathLen >= maxPathLength)
	{
		LogError("Path is too long: %s\n", relPath);
		free(path);
		return NULL;
	}
	if (snprintf(path, maxPathLength, "%sassets/%s", GetState()->executableFolder, relPath) > 300)
	{
		LogError("Asset path too long!\n");
		free(path);
		return NULL;
	}

	FILE *file = fopen(path, "rb");
	if (file == NULL)
	{
		LogError("Failed to open asset file: %s with errno %s\n", path, strerror(errno));
		free(path);
		return NULL;
	}

	free(path);

	return file;
}

void LoadTextureSizeTable()
{
	FILE *file = OpenAssetFile("tsizetable.gtsb");
	if (file == NULL)
	{
		Error("Failed to open texture size table file!");
	}
	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fileSize < sizeof(uint) * 2)
	{
		LogError("Failed to read texture size table, file was too small [a]! (%d bytes)", fileSize);
		Error("Failed to read texture size table, file was too small!");
	}

	tsb = malloc(sizeof(TextureSizeTable));
	chk_malloc(tsb);

	fread(&tsb->textureCount, sizeof(uint), 1, file);
	fread(&tsb->assetCount, sizeof(uint), 1, file);

	if (fileSize < (sizeof(uint) * 2) + (tsb->textureCount * (sizeof(char) * 32)))
	{
		LogError("Failed to read texture size table, file was too small [b]! (%d bytes)", fileSize);
		Error("Failed to read texture size table, file was too small!");
	}

	tsb->textureNames = calloc(tsb->textureCount, sizeof(char) * 32);
	chk_malloc(tsb->textureNames);

	fread(tsb->textureNames, sizeof(char) * 32, tsb->textureCount, file);

	fclose(file);
}

void FreeModel(Model *model)
{
	if (model == NULL)
	{
		return;
	}
	free(model->name);
	free(model->packedVertsUvsNormal);
	free(model->packedIndices);
	free(model);
	model = NULL;
}

const TextureSizeTable *GetTextureSizeTable()
{
	return tsb;
}

void AssetCacheInit()
{
	ListCreate(&assetCacheNames);
	ListCreate(&assetCacheData);
	LoadTextureSizeTable();
}

void DestroyAssetCache()
{
	for (int i = 0; i < assetCacheData.length; i++)
	{
		Asset *asset = ListGet(assetCacheData, i);
		free(asset->data);
		free(asset);
	}
	free(assetCacheData.data);

	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		if (images[i] != NULL)
		{
			free(images[i]->name);
			free(images[i]);
		}
	}

	for (int i = 0; i < MAX_MODELS; i++)
	{
		FreeModel(models[i]);
	}

	ListAndContentsFree(&assetCacheNames, false);
	free(tsb->textureNames);
	free(tsb);
}

Asset *DecompressAsset(const char *relPath)
{
	// see if relPath is already in the cache
	for (int i = 0; i < assetCacheNames.length; i++)
	{
		if (strncmp(ListGet(assetCacheNames, i), relPath, 48) == 0)
		{
			return ListGet(assetCacheData, i);
		}
	}

	FILE *file = OpenAssetFile(relPath);
	if (file == NULL)
	{
		LogError("Failed to open asset file: %s\n", relPath);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	const size_t fileSize = ftell(file);

	byte *asset = malloc(fileSize);
	chk_malloc(asset);
	fseek(file, 0, SEEK_SET);
	fread(asset, 1, fileSize, file);

	fclose(file);

	Asset *assetStruct = malloc(sizeof(Asset));
	chk_malloc(assetStruct);

	size_t offset = 0;
	// Read the first 4 bytes of the asset to get the size of the compressed data
	const uint compressedSize = ReadUint(asset, &offset);
	const uint decompressedSize = ReadUint(asset, &offset);
	const uint assetId = ReadUint(asset, &offset);
	const uint assetType = ReadUint(asset, &offset);

	assetStruct->compressedSize = compressedSize;
	assetStruct->size = decompressedSize;
	assetStruct->assetId = assetId;
	assetStruct->type = assetType;

	asset += offset; // skip header

	// Allocate memory for the decompressed data
	byte *decompressedData = malloc(decompressedSize);
	chk_malloc(decompressedData);

	z_stream stream = {0};

	// Initialize the zlib stream
	stream.next_in = asset;
	stream.avail_in = compressedSize;
	stream.next_out = decompressedData;
	stream.avail_out = decompressedSize;

	// Initialize the zlib stream
	if (inflateInit2(&stream, MAX_WBITS | 16) != Z_OK)
	{
		free(decompressedData);
		free(asset - 16);
		free(assetStruct);
		LogError("Failed to initialize zlib stream: %s\n", stream.msg);
		return NULL;
	}

	// Decompress the data
	int inflateReturnValue = inflate(&stream, Z_NO_FLUSH);
	while (inflateReturnValue != Z_STREAM_END)
	{
		if (inflateReturnValue != Z_OK)
		{
			free(decompressedData);
			free(asset - 16);
			free(assetStruct);
			LogError("Failed to decompress zlib stream: %s\n", stream.msg);
			return NULL;
		}
		inflateReturnValue = inflate(&stream, Z_NO_FLUSH);
	}

	// Clean up the zlib stream
	if (inflateEnd(&stream) != Z_OK)
	{
		free(decompressedData);
		free(asset - 16);
		free(assetStruct);
		LogError("Failed to end zlib stream: %s\n", stream.msg);
		return NULL;
	}

	assetStruct->data = decompressedData;

	// Add the asset to the cache
	const size_t pathLength = strlen(relPath);
	char *data = malloc(pathLength + 1);
	chk_malloc(data);
	strncpy(data, relPath, pathLength);
	ListAdd(&assetCacheNames, data);
	ListAdd(&assetCacheData, assetStruct);

	return assetStruct;
}

void GenFallbackImage(Image *src)
{
	src->width = 64;
	src->height = 64;
	src->pixelDataSize = 64 * 64 * 4;
	src->pixelData = malloc(src->pixelDataSize);
	chk_malloc(src->pixelData);

	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 64; y++)
		{
			if ((x < 32) ^ (y < 32))
			{
				src->pixelData[(x + y * 64) * 4] = 0;
				src->pixelData[(x + y * 64) * 4 + 1] = 0;
				src->pixelData[(x + y * 64) * 4 + 2] = 0;
				src->pixelData[(x + y * 64) * 4 + 3] = 255;
			} else
			{
				src->pixelData[(x + y * 64) * 4] = 255;
				src->pixelData[(x + y * 64) * 4 + 1] = 0;
				src->pixelData[(x + y * 64) * 4 + 2] = 255;
				src->pixelData[(x + y * 64) * 4 + 3] = 255;
			}
		}
	}
}

Image *LoadImage(const char *asset)
{
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		Image *img = images[i];
		if (img == NULL)
		{
			break;
		}
		if (strncmp(asset, img->name, 48) == 0)
		{
			return img;
		}
	}

	Image *img = malloc(sizeof(Image));
	chk_malloc(img);

	const Asset *textureAsset = DecompressAsset(asset);
	if (textureAsset == NULL || textureAsset->type != ASSET_TYPE_TEXTURE)
	{
		GenFallbackImage(img);
	} else
	{
		img->pixelDataSize = ReadUintA(textureAsset->data, IMAGE_SIZE_OFFSET);
		img->width = ReadUintA(textureAsset->data, IMAGE_WIDTH_OFFSET);
		img->height = ReadUintA(textureAsset->data, IMAGE_HEIGHT_OFFSET);
		img->pixelData = textureAsset->data + sizeof(uint) * 4;
	}

	img->id = textureId;

	const size_t nameLength = strlen(asset) + 1;
	img->name = malloc(nameLength);
	strncpy(img->name, asset, nameLength);

	images[textureId] = img;

	textureId++;

	return img;
}

Model *LoadModel(const char *asset)
{
	for (int i = 0; i < MAX_MODELS; i++)
	{
		Model *model = models[i];
		if (model == NULL)
		{
			break;
		}
		if (strcmp(asset, model->name) == 0)
		{
			return model;
		}
	}

	const Asset *assetData = DecompressAsset(asset);
	if (assetData == NULL)
	{
		LogError("Failed to load model from asset, asset was NULL!");
		Error("Failed to load model!");
	}
	if (assetData->size < sizeof(ModelHeader))
	{
		LogError("Failed to load model from asset, size was too small!");
		return NULL;
	}
	Model *model = malloc(sizeof(Model));
	chk_malloc(model);
	memcpy(&model->header, assetData->data, sizeof(ModelHeader));

	if (strncmp(model->header.sig, "MSH", 3) != 0)
	{
		LogError("Tried to load a model, but its first magic was incorrect (got %s)!", model->header.sig);
		free(model);
		return NULL;
	}

	if (strncmp(model->header.dataSig, "DAT", 3) != 0)
	{
		LogError("Tried to load a model, but its second magic was incorrect (got %s)!", model->header.dataSig);
		free(model);
		return NULL;
	}

	model->id = modelId;
	models[modelId] = model;
	modelId++;

	const size_t nameLength = strlen(asset) + 1;
	model->name = malloc(nameLength);
	strncpy(model->name, asset, nameLength);

	const size_t vertsSizeBytes = model->header.indexCount * (sizeof(float) * 8);
	const size_t indexSizeBytes = model->header.indexCount * sizeof(uint);

	model->packedVertsUvsNormalCount = model->header.indexCount;
	model->packedIndicesCount = model->header.indexCount;

	model->packedVertsUvsNormal = malloc(vertsSizeBytes);
	chk_malloc(model->packedVertsUvsNormal);
	model->packedIndices = malloc(indexSizeBytes);
	chk_malloc(model->packedIndices);

	memcpy(model->packedVertsUvsNormal, (byte *)assetData->data + sizeof(ModelHeader), vertsSizeBytes);

	for (int i = 0; i < model->header.indexCount; i++)
	{
		model->packedIndices[i] = i;
	}

	return model;
}

Model *GetModelFromId(const uint id)
{
	if (id >= modelId)
	{
		Error("Invalid model ID!");
	}

	return models[id];
}
