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
	int dataOffset = 0;
	bool done = false;
	while (!done)
	{
		const byte opcode = data[dataOffset];
		dataOffset++;
		switch (opcode)
		{
			case LEVEL_CMD_WALL:
			{
				const double firstVertX = ReadDouble(data, &dataOffset);
				const double firstVertY = ReadDouble(data, &dataOffset);
				const double secondVertX = ReadDouble(data, &dataOffset);
				const double secondVertY = ReadDouble(data, &dataOffset);
				const uint textureID = ReadUint(data, &dataOffset);
				const float uvScale = ReadFloat(data, &dataOffset);
				const Vector2 firstVertex = v2(firstVertX, firstVertY);
				const Vector2 secondVertex = v2(secondVertX, secondVertY);
				Wall *w = CreateWall(firstVertex, secondVertex, wallTextures[textureID], uvScale, 0.0);
				ListAdd(l->walls, w);
				break;
			}
			case LEVEL_CMD_PLAYER:
			{
				const double playerX = ReadDouble(data, &dataOffset);
				const double playerY = ReadDouble(data, &dataOffset);
				const double playerRotation = ReadDouble(data, &dataOffset);
				l->player.pos = v2(playerX, playerY);
				l->player.angle = playerRotation;
				break;
			}
			case LEVEL_CMD_COLORS:
			{
				const uint skyColor = ReadUint(data, &dataOffset);
				dataOffset += sizeof(uint); // skip the second color
				l->skyColor = skyColor;
				break;
			}
			case LEVEL_CMD_ACTOR:
			{
				const double actorX = ReadDouble(data, &dataOffset);
				const double actorY = ReadDouble(data, &dataOffset);
				const double actorRotation = ReadDouble(data, &dataOffset);
				const int actorType = ReadUint(data, &dataOffset);
				const byte paramA = ReadByte(data, &dataOffset);
				const byte paramB = ReadByte(data, &dataOffset);
				const byte paramC = ReadByte(data, &dataOffset);
				const byte paramD = ReadByte(data, &dataOffset);
				Actor *a = CreateActor(v2(actorX, actorY), actorRotation, actorType, paramA, paramB, paramC, paramD);
				ListAdd(l->actors, a);
				break;
			}
			case LEVEL_CMD_FINISH:
			{
				done = true;
				break;
			}
			case LEVEL_CMD_FOG:
			{
				const uint fogColor = ReadUint(data, &dataOffset);
				const double fogStartDistance = ReadDouble(data, &dataOffset);
				const double fogEndDistance = ReadDouble(data, &dataOffset);
				l->fogColor = fogColor;
				l->fogStart = fogStartDistance;
				l->fogEnd = fogEndDistance;
				break;
			}
			case LEVEL_CMD_FLOOR_CEIL:
			{
				const uint floorTextureIndex = ReadUint(data, &dataOffset);
				const uint ceilingTextureIndex = ReadUint(data, &dataOffset);
				l->floorTextureIndex = floorTextureIndex;
				l->ceilingTextureIndex = ceilingTextureIndex;
				break;
			}
			case LEVEL_CMD_MUSIC:
			{
				const uint musicIndex = ReadUint(data, &dataOffset);
				l->musicIndex = musicIndex;
				break;
			}
			case LEVEL_CMD_METADATA:
			{
				memcpy(l->name, data + dataOffset, sizeof(char) * 32);
				dataOffset += sizeof(char) * 32;
				int courseNumber = ReadInt(data, &dataOffset);
				l->courseNum = courseNumber;
				break;
			}
			default:
				LogError("Unknown level opcode %u at offset %u", opcode, dataOffset);
				Error("Unknown Level OpCode");
		}
	}
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
	WriteUint(dataBuffer, &dataBufferOffset, l->floorTextureIndex);
	WriteUint(dataBuffer, &dataBufferOffset, l->ceilingTextureIndex);
	dataBuffer[dataBufferOffset] = LEVEL_CMD_MUSIC;
	dataBufferOffset++;
	WriteUint(dataBuffer, &dataBufferOffset, l->musicIndex);
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
