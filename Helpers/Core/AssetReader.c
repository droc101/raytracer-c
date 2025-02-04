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
uint textureId;
uint modelId;
Image *images[MAX_TEXTURES];
Model *models[MAX_MODELS];

FILE *OpenAssetFile(const char *relPath)
{
	const size_t maxPathLength = 300;
	char *path = calloc(maxPathLength, sizeof(char));
	CheckAlloc(path);

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

void FreeModel(Model *model)
{
	if (model == NULL)
	{
		return;
	}
	free(model->name);
	free(model->vertexData);
	free(model->indexData);
	free(model);
	model = NULL;
}

void AssetCacheInit()
{
	ListCreate(&assetCacheNames);
	ListCreate(&assetCacheData);
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
	CheckAlloc(asset);
	fseek(file, 0, SEEK_SET);
	fread(asset, 1, fileSize, file);

	fclose(file);

	Asset *assetStruct = malloc(sizeof(Asset));
	CheckAlloc(assetStruct);

	size_t offset = 0;
	// Read the first 4 bytes of the asset to get the size of the compressed data
	const uint compressedSize = ReadUint(asset, &offset);
	const uint decompressedSize = ReadUint(asset, &offset);
	ReadUint(asset, &offset); // Asset ID was deprecated, this value is no longer used
	const uint assetType = ReadUint(asset, &offset);

	assetStruct->compressedSize = compressedSize;
	assetStruct->size = decompressedSize;
	assetStruct->type = assetType;

	asset += offset; // skip header

	// Allocate memory for the decompressed data
	byte *decompressedData = malloc(decompressedSize);
	CheckAlloc(decompressedData);

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
	const size_t pathLength = strlen(relPath) + 1;
	char *data = malloc(pathLength);
	CheckAlloc(data);
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
	CheckAlloc(src->pixelData);

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
	CheckAlloc(img);

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
	CheckAlloc(img->name);
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
	CheckAlloc(model);
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
	CheckAlloc(model->name);
	strncpy(model->name, asset, nameLength);

	const size_t vertsSizeBytes = model->header.vertexCount * (sizeof(float) * 8);
	const size_t indexSizeBytes = model->header.indexCount * sizeof(uint);

	model->vertexCount = model->header.vertexCount;
	model->indexCount = model->header.indexCount;

	model->vertexData = malloc(vertsSizeBytes);
	CheckAlloc(model->vertexData);
	model->indexData = malloc(indexSizeBytes);
	CheckAlloc(model->indexData);

	// Copy the index data, then the vertex data
	memcpy(model->indexData, assetData->data + sizeof(ModelHeader), indexSizeBytes);
	memcpy(model->vertexData, assetData->data + sizeof(ModelHeader) + indexSizeBytes, vertsSizeBytes);

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

Font *LoadFont(const char *asset)
{
	const Asset *assetData = DecompressAsset(asset);
	if (assetData == NULL)
	{
		LogError("Failed to load font from asset, asset was NULL!");
		Error("Failed to load model!");
	}
	if (assetData->size < sizeof(Font) - sizeof(Image*))
	{
		LogError("Failed to load font from asset, size was too small!");
		return NULL;
	}
	Font *font = malloc(sizeof(Font));
	CheckAlloc(font);
	memcpy(font, assetData->data, sizeof(Font));
	char temp[32];
	strncpy(temp, font->texture, 32);
	sprintf(font->texture, "texture/%s.gtex", temp);
	font->image = LoadImage(font->texture);

	return font;
}
