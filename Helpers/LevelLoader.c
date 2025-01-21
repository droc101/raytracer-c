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
#include "../Structs/Trigger.h"

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
	l->player.pos.x = ReadDouble(data, &offset);
	l->player.pos.y = ReadDouble(data, &offset);
	l->player.angle = ReadDouble(data, &offset);

	const uint actorCount = ReadUint(data, &offset);
	for (int i = 0; i < actorCount; i++)
	{
		const double actorX = ReadDouble(data, &offset);
		const double actorY = ReadDouble(data, &offset);
		const double actorRot = ReadDouble(data, &offset);
		const int actorType = ReadInt(data, &offset);
		const byte actorParamA = ReadByte(data, &offset);
		const byte actorParamB = ReadByte(data, &offset);
		const byte actorParamC = ReadByte(data, &offset);
		const byte actorParamD = ReadByte(data, &offset);
		Actor *a = CreateActor(v2(actorX, actorY),
							   actorRot,
							   actorType,
							   actorParamA,
							   actorParamB,
							   actorParamC,
							   actorParamD);
		ListAdd(l->actors, a);
	}

	const uint wallCount = ReadUint(data, &offset);
	for (int i = 0; i < wallCount; i++)
	{
		const double wallAX = ReadDouble(data, &offset);
		const double wallAY = ReadDouble(data, &offset);
		const double wallBX = ReadDouble(data, &offset);
		const double wallBY = ReadDouble(data, &offset);
		char lDataWallTex[32];
		ReadString(data, &offset, (char *)&lDataWallTex, 32);
		const char wallTex[48];
		snprintf(wallTex, 48, "texture/%s.gtex", lDataWallTex);
		const float wallUVScale = ReadFloat(data, &offset);
		const float wallUVOffset = ReadFloat(data, &offset);
		Wall *w = CreateWall(v2(wallAX, wallAY), v2(wallBX, wallBY), wallTex, wallUVScale, wallUVOffset);
		ListAdd(l->walls, w);
	}

	const uint triggerCount = ReadUint(data, &offset);
	for (int i = 0; i < triggerCount; i++)
	{
		const double trigX = ReadDouble(data, &offset);
		const double trigY = ReadDouble(data, &offset);
		const double trigRot = ReadDouble(data, &offset);
		const double trigExtX = ReadDouble(data, &offset);
		const double trigExtY = ReadDouble(data, &offset);
		char trigCommand[64];
		ReadString(data, &offset, (char *)&trigCommand, 64);
		Trigger *t = CreateTrigger(v2(trigX, trigY), v2(trigExtX, trigExtY), trigRot, trigCommand);
		ListAdd(l->triggers, t);
	}

	return l;
}
