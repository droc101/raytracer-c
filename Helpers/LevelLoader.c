//
// Created by droc101 on 4/21/2024.
//

#include "LevelLoader.h"
#include <box2d/box2d.h>
#include <stdio.h>
#include "../Helpers/CommonAssets.h"
#include "../Structs/Actor.h"
#include "../Structs/Level.h"
#include "../Structs/Trigger.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"
#include "Core/DataReader.h"

Level *LoadLevel(const byte *data)
{
	Level *l = CreateLevel();
	size_t offset = 0;

	ReadString(data, &offset, l->name, 32);
	l->courseNum = ReadShort(data, &offset);
	l->hasCeiling = ReadByte(data, &offset);

	char lDataCeilOrSkyTex[32];
	char lDataFloorTex[32];

	ReadString(data, &offset, lDataCeilOrSkyTex, 32);
	ReadString(data, &offset, lDataFloorTex, 32);


	snprintf(l->ceilOrSkyTex, 48, "texture/%s.gtex", lDataCeilOrSkyTex);
	snprintf(l->floorTex, 48, "texture/%s.gtex", lDataFloorTex);

	ReadString(data, &offset, l->music, 32);

	l->fogColor = ReadUint(data, &offset);
	l->fogStart = ReadDouble(data, &offset);
	l->fogEnd = ReadDouble(data, &offset);

	l->player.pos.x = (float)ReadDouble(data, &offset);
	l->player.pos.y = (float)ReadDouble(data, &offset);
	l->player.angle = (float)ReadDouble(data, &offset);

	b2Body_SetTransform(l->player.bodyId, l->player.pos, b2MakeRot(l->player.angle));

	const uint actorCount = ReadUint(data, &offset);
	for (int i = 0; i < actorCount; i++)
	{
		const float actorX = (float)ReadDouble(data, &offset);
		const float actorY = (float)ReadDouble(data, &offset);
		const float actorRotation = (float)ReadDouble(data, &offset);
		const int actorType = ReadInt(data, &offset);
		const byte actorParamA = ReadByte(data, &offset);
		const byte actorParamB = ReadByte(data, &offset);
		const byte actorParamC = ReadByte(data, &offset);
		const byte actorParamD = ReadByte(data, &offset);
		Actor *a = CreateActor(v2(actorX, actorY),
							   actorRotation,
							   actorType,
							   actorParamA,
							   actorParamB,
							   actorParamC,
							   actorParamD,
							   l->worldId);
		ListAdd(&l->actors, a);
	}

	const uint wallCount = ReadUint(data, &offset);
	for (int i = 0; i < wallCount; i++)
	{
		const float wallAX = (float)ReadDouble(data, &offset);
		const float wallAY = (float)ReadDouble(data, &offset);
		const float wallBX = (float)ReadDouble(data, &offset);
		const float wallBY = (float)ReadDouble(data, &offset);
		char lDataWallTex[32];
		ReadString(data, &offset, (char *)&lDataWallTex, 32);
		const char wallTex[48];
		snprintf(wallTex, 48, "texture/%s.gtex", lDataWallTex);
		const float wallUVScale = ReadFloat(data, &offset);
		const float wallUVOffset = ReadFloat(data, &offset);
		Wall *w = CreateWall(v2(wallAX, wallAY), v2(wallBX, wallBY), wallTex, wallUVScale, wallUVOffset);
		WallBake(w);
		CreateWallCollider(w, l->worldId);
		ListAdd(&l->walls, w);
	}

	const uint triggerCount = ReadUint(data, &offset);
	for (int i = 0; i < triggerCount; i++)
	{
		const float trigX = (float)ReadDouble(data, &offset);
		const float trigY = (float)ReadDouble(data, &offset);
		const float trigRot = (float)ReadDouble(data, &offset);
		const float trigExtX = (float)ReadDouble(data, &offset);
		const float trigExtY = (float)ReadDouble(data, &offset);
		char trigCommand[64];
		ReadString(data, &offset, (char *)&trigCommand, 64);
		const uint flags = ReadUint(data, &offset);
		Trigger *t = CreateTrigger(v2(trigX, trigY), v2(trigExtX, trigExtY), trigRot, trigCommand, flags);
		ListAdd(&l->triggers, t);
	}

	return l;
}
