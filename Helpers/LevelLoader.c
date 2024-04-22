//
// Created by droc101 on 4/21/2024.
//

#include <stdio.h>
#include "LevelLoader.h"
#include "../defines.h"
#include "../Structs/level.h"
#include "../error.h"
#include "../Structs/wall.h"

Level LoadLevel(byte *data) {
    Level l = CreateLevel();
    int i = 0;
    bool done = false;
    while (!done) {
        byte opcode = data[i];
        i++;
        switch (opcode) {
            case LEVEL_CMD_WALL: {
                double v1 = (double) data[i]; i += sizeof(double);
                double v2 = (double) data[i]; i += sizeof(double);
                double v3 = (double) data[i]; i += sizeof(double);
                double v4 = (double) data[i]; i += sizeof(double);
                uint tid = (uint)data[i]; i += sizeof(uint);
                Vector2 va = vec2(v1, v2);
                Vector2 vb = vec2(v3, v4);
                Wall *w = CreateWall(va, vb, tid);
                ListAdd(l.walls, w);
                break;
            }
            case LEVEL_CMD_PLAYER: {
                double x = (double) data[i]; i += sizeof(double);
                double y = (double) data[i]; i += sizeof(double);
                double r = (double) data[i]; i += sizeof(double);
                l.position = vec2(x, y);
                l.rotation = r;
                break;
            }
            case LEVEL_CMD_COLORS: {
                uint sky = (uint)data[i]; i += sizeof(uint);
                uint floor = (uint)data[i]; i += sizeof(uint);
                l.SkyColor = sky;
                l.FloorColor = floor;
                break;
            }
            case LEVEL_CMD_ACTOR: {
                break; // TODO
            }
            case LEVEL_CMD_FINISH: {
                done = true;
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
