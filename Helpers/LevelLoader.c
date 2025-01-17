//
// Created by droc101 on 4/21/2024.
//

#include "LevelLoader.h"
#include "../Helpers/CommonAssets.h"
#include "../Structs/Actor.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"
#include "Core/DataReader.h"
#include "Core/Error.h"
#include "Core/Logging.h"

Level *LoadLevel(const byte *data)
{
	Level *l = CreateLevel();
	// TODO: new level bytecode
	return l;
}

LevelBytecode *GenerateBytecode(const Level *l)
{
	byte *dataBuffer = malloc(1048576);
	chk_malloc(dataBuffer);
	int dataBufferOffset = 0;
	for (int j = 0; j < l->walls->size; j++)
	{
		const Wall *w = ListGet(l->walls, j);
		dataBuffer[dataBufferOffset] = LEVEL_CMD_WALL;
		dataBufferOffset++;

		const int wall_texID = FindWallTextureIndex(w->tex);

		WriteDouble(dataBuffer, &dataBufferOffset, w->a.x);
		WriteDouble(dataBuffer, &dataBufferOffset, w->a.y);
		WriteDouble(dataBuffer, &dataBufferOffset, w->b.x);
		WriteDouble(dataBuffer, &dataBufferOffset, w->b.y);
		WriteUint(dataBuffer, &dataBufferOffset, wall_texID);
		WriteFloat(dataBuffer, &dataBufferOffset, w->uvScale);
	}
	for (int j = 0; j < l->actors->size; j++)
	{
		const Actor *a = ListGet(l->actors, j);
		dataBuffer[dataBufferOffset] = LEVEL_CMD_ACTOR;
		dataBufferOffset++;
		WriteDouble(dataBuffer, &dataBufferOffset, a->position.x);
		WriteDouble(dataBuffer, &dataBufferOffset, a->position.y);
		WriteDouble(dataBuffer, &dataBufferOffset, a->rotation);
		WriteUint(dataBuffer, &dataBufferOffset, a->actorType);
		WriteByte(dataBuffer, &dataBufferOffset, a->paramA);
		WriteByte(dataBuffer, &dataBufferOffset, a->paramB);
		WriteByte(dataBuffer, &dataBufferOffset, a->paramC);
		WriteByte(dataBuffer, &dataBufferOffset, a->paramD);
	}
	dataBuffer[dataBufferOffset] = LEVEL_CMD_PLAYER;
	dataBufferOffset++;
	WriteDouble(dataBuffer, &dataBufferOffset, l->player.pos.x);
	WriteDouble(dataBuffer, &dataBufferOffset, l->player.pos.y);
	WriteDouble(dataBuffer, &dataBufferOffset, l->player.angle);
	dataBuffer[dataBufferOffset] = LEVEL_CMD_COLORS;
	dataBufferOffset++;
	WriteUint(dataBuffer, &dataBufferOffset, l->skyColor);
	WriteUint(dataBuffer, &dataBufferOffset, 0);
	dataBuffer[dataBufferOffset] = LEVEL_CMD_FOG;
	dataBufferOffset++;
	WriteUint(dataBuffer, &dataBufferOffset, l->fogColor);
	WriteDouble(dataBuffer, &dataBufferOffset, l->fogStart);
	WriteDouble(dataBuffer, &dataBufferOffset, l->fogEnd);
	dataBuffer[dataBufferOffset] = LEVEL_CMD_FLOOR_CEIL;
	dataBufferOffset++;
	WriteUint(dataBuffer, &dataBufferOffset, l->floorTexture);
	WriteUint(dataBuffer, &dataBufferOffset, l->ceilingTexture);
	dataBuffer[dataBufferOffset] = LEVEL_CMD_MUSIC;
	dataBufferOffset++;
	WriteUint(dataBuffer, &dataBufferOffset, l->musicID);
	dataBuffer[dataBufferOffset] = LEVEL_CMD_METADATA;
	dataBufferOffset++;
	memcpy(dataBuffer + dataBufferOffset, l->name, 32);
	dataBufferOffset += 32;
	WriteInt(dataBuffer, &dataBufferOffset, l->courseNum);
	dataBuffer[dataBufferOffset] = LEVEL_CMD_FINISH;
	dataBufferOffset++;

	void *tmp = realloc(dataBuffer, dataBufferOffset);
	chk_malloc(tmp);
	dataBuffer = tmp;

	LevelBytecode *lb = malloc(sizeof(LevelBytecode));
	chk_malloc(lb);
	lb->data = dataBuffer;
	lb->size = dataBufferOffset;

	return lb;
}
