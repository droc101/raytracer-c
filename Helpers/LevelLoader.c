//
// Created by droc101 on 4/21/2024.
//

#include "LevelLoader.h"
#include <box2d/box2d.h>
#include <stdio.h>
#include "../Helpers/CommonAssets.h"
#include "../Structs/Actor.h"
#include "../Structs/Level.h"
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

	l->player.pos.x = ReadFloat(data, &offset);
	l->player.pos.y = ReadFloat(data, &offset);
	l->player.angle = ReadFloat(data, &offset);

	b2Body_SetTransform(l->player.bodyId, l->player.pos, b2MakeRot(l->player.angle));

	const uint actorCount = ReadUint(data, &offset);
	for (int i = 0; i < actorCount; i++)
	{
		const float actorX = ReadFloat(data, &offset);
		const float actorY = ReadFloat(data, &offset);
		const float actorRotation = ReadFloat(data, &offset);
		const int actorType = ReadInt(data, &offset);
		const byte actorParamA = ReadByte(data, &offset);
		const byte actorParamB = ReadByte(data, &offset);
		const byte actorParamC = ReadByte(data, &offset);
		const byte actorParamD = ReadByte(data, &offset);
		const char actorName[64];
		ReadString(data, &offset, (char *)&actorName, 64);
		const uint connectionCount = ReadUint(data, &offset);
		Actor *a = CreateActor(v2(actorX, actorY),
							   actorRotation,
							   actorType,
							   actorParamA,
							   actorParamB,
							   actorParamC,
							   actorParamD,
							   l->worldId);
		for (int j = 0; j < connectionCount; j++)
		{
			ActorConnection *ac = malloc(sizeof(ActorConnection));
			ac->myOutput = ReadByte(data, &offset);
			char outActorName[64];
			ReadString(data, &offset, (char *)&outActorName, 64);
			strcpy(ac->outActorName, outActorName);
			ac->targetInput = ReadByte(data, &offset);
			char outParamOverride[64];
			ReadString(data, &offset, (char *)&outParamOverride, 64);
			strcpy(ac->outParamOverride, outParamOverride);
			ListAdd(&a->ioConnections, ac);
		}
		ListAdd(&l->actors, a);
		if (actorName[0] != '\0')
		{
			NameActor(a, actorName, l);
		}
	}

	const uint wallCount = ReadUint(data, &offset);
	for (int i = 0; i < wallCount; i++)
	{
		const float wallAX = ReadFloat(data, &offset);
		const float wallAY = ReadFloat(data, &offset);
		const float wallBX = ReadFloat(data, &offset);
		const float wallBY = ReadFloat(data, &offset);
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

	return l;
}
