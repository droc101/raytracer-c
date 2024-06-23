//
// Created by droc101 on 4/21/2024.
//

#include <stdio.h>
#include "LevelLoader.h"
#include "Error.h"
#include "../Structs/Actor.h"
#include "DataReader.h"

Level *LoadLevel(byte *data) {
    Level *l = CreateLevel();
    int i = 0;
    bool done = false;
    while (!done) {
        byte opcode = data[i];
        i++;
        switch (opcode) {
            case LEVEL_CMD_WALL: {
                double v1 = ReadDouble(data, &i);
                double v2 = ReadDouble(data, &i);
                double v3 = ReadDouble(data, &i);
                double v4 = ReadDouble(data, &i);
                uint tid = ReadUint(data, &i);
                Vector2 va = vec2(v1, v2);
                Vector2 vb = vec2(v3, v4);
                Wall *w = CreateWall(va, vb, tid);
                ListAdd(l->walls, w);
                break;
            }
            case LEVEL_CMD_PLAYER: {
                double x = ReadDouble(data, &i);
                double y = ReadDouble(data, &i);
                double r = ReadDouble(data, &i);
                l->position = vec2(x, y);
                l->rotation = r;
                break;
            }
            case LEVEL_CMD_COLORS: {
                uint sky = ReadUint(data, &i);
                uint floor = ReadUint(data, &i);
                l->SkyColor = sky;
                l->FloorColor = floor;
                break;
            }
            case LEVEL_CMD_ACTOR: {
                double x = ReadDouble(data, &i);
                double y = ReadDouble(data, &i);
                double r = ReadDouble(data, &i);
                int type = ReadUint(data, &i);
                Actor *a = CreateActor(vec2(x, y), r, type);
                ListAdd(l->actors, a);
                break;
            }
            case LEVEL_CMD_FINISH: {
                done = true;
                break;
            }
            case LEVEL_CMD_FOG: {
                uint color = ReadUint(data, &i);
                double start = ReadDouble(data, &i);
                double end = ReadDouble(data, &i);
                l->FogColor = color;
                l->FogStart = start;
                l->FogEnd = end;
                break;
            }
            default:
                printf("Unknown level opcode %u at offset %u", opcode, i);
                fflush(stdout);
                Error("Unknown Level OpCode");
        }
    }
    return l;
}
