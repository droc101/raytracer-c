//
// Created by droc101 on 4/21/2024.
//

#include <stdio.h>
#include "LevelLoader.h"
#include "Core/Error.h"
#include "../Structs/Actor.h"
#include "Core/DataReader.h"
#include "../Helpers/CommonAssets.h"
#include "Core/Logging.h"

Level *LoadLevel(byte *data)
{
    Level *l = CreateLevel();
    int i = 0;
    bool done = false;
    while (!done)
    {
        const byte opcode = data[i];
        i++;
        switch (opcode)
        {
            case LEVEL_CMD_WALL:
            {
                const double v1 = ReadDouble(data, &i);
                const double vt2 = ReadDouble(data, &i);
                const double v3 = ReadDouble(data, &i);
                const double v4 = ReadDouble(data, &i);
                const uint tid = ReadUint(data, &i);
                const float uvScale = ReadFloat(data, &i);
                const Vector2 va = v2(v1, vt2);
                const Vector2 vb = v2(v3, v4);
                Wall *w = CreateWall(va, vb, wallTextures[tid], uvScale, 0.0);
                ListAdd(l->walls, w);
                break;
            }
            case LEVEL_CMD_PLAYER:
            {
                const double x = ReadDouble(data, &i);
                const double y = ReadDouble(data, &i);
                const double r = ReadDouble(data, &i);
                l->position = v2(x, y);
                l->rotation = r;
                break;
            }
            case LEVEL_CMD_COLORS:
            {
                const uint sky = ReadUint(data, &i);
                i += sizeof(uint); // skip the second color
                l->SkyColor = sky;
                break;
            }
            case LEVEL_CMD_ACTOR:
            {
                const double x = ReadDouble(data, &i);
                const double y = ReadDouble(data, &i);
                const double r = ReadDouble(data, &i);
                const int type = ReadUint(data, &i);
                const byte paramA = ReadByte(data, &i);
                const byte paramB = ReadByte(data, &i);
                const byte paramC = ReadByte(data, &i);
                const byte paramD = ReadByte(data, &i);
                Actor *a = CreateActor(v2(x, y), r, type, paramA, paramB, paramC, paramD);
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
                const uint color = ReadUint(data, &i);
                const double start = ReadDouble(data, &i);
                const double end = ReadDouble(data, &i);
                l->FogColor = color;
                l->FogStart = start;
                l->FogEnd = end;
                break;
            }
            case LEVEL_CMD_FLOOR_CEIL:
            {
                const uint floor = ReadUint(data, &i);
                const uint ceil = ReadUint(data, &i);
                l->FloorTexture = floor;
                l->CeilingTexture = ceil;
                break;
            }
            case LEVEL_CMD_MUSIC:
            {
                const uint music = ReadUint(data, &i);
                l->MusicID = music;
                break;
            }
            default:
                LogError("Unknown level opcode %u at offset %u", opcode, i);
                Error("Unknown Level OpCode");
        }
    }
    return l;
}

LevelBytecode *GenerateBytecode(const Level *l)
{
    byte *data = malloc(1048576);
    int i = 0;
    for (int j = 0; j < l->walls->size; j++)
    {
        const Wall *w = ListGet(l->walls, j);
        data[i] = LEVEL_CMD_WALL;
        i++;

        const int wall_texID = FindWallTextureIndex(w->tex);

        WriteDouble(data, &i, w->a.x);
        WriteDouble(data, &i, w->a.y);
        WriteDouble(data, &i, w->b.x);
        WriteDouble(data, &i, w->b.y);
        WriteUint(data, &i, wall_texID);
        WriteFloat(data, &i, w->uvScale);
    }
    for (int j = 0; j < l->actors->size; j++)
    {
        const Actor *a = ListGet(l->actors, j);
        data[i] = LEVEL_CMD_ACTOR;
        i++;
        WriteDouble(data, &i, a->position.x);
        WriteDouble(data, &i, a->position.y);
        WriteDouble(data, &i, a->rotation);
        WriteUint(data, &i, a->actorType);
        WriteByte(data, &i, a->paramA);
        WriteByte(data, &i, a->paramB);
        WriteByte(data, &i, a->paramC);
        WriteByte(data, &i, a->paramD);
    }
    data[i] = LEVEL_CMD_PLAYER;
    i++;
    WriteDouble(data, &i, l->position.x);
    WriteDouble(data, &i, l->position.y);
    WriteDouble(data, &i, l->rotation);
    data[i] = LEVEL_CMD_COLORS;
    i++;
    WriteUint(data, &i, l->SkyColor);
    WriteUint(data, &i, 0);
    data[i] = LEVEL_CMD_FOG;
    i++;
    WriteUint(data, &i, l->FogColor);
    WriteDouble(data, &i, l->FogStart);
    WriteDouble(data, &i, l->FogEnd);
    data[i] = LEVEL_CMD_FLOOR_CEIL;
    i++;
    WriteUint(data, &i, l->FloorTexture);
    WriteUint(data, &i, l->CeilingTexture);
    data[i] = LEVEL_CMD_MUSIC;
    i++;
    WriteUint(data, &i, l->MusicID);
    data[i] = LEVEL_CMD_FINISH;
    i++;

    data = realloc(data, i);

    LevelBytecode *lb = malloc(sizeof(LevelBytecode));
    lb->data = data;
    lb->size = i;

    return lb;
}
