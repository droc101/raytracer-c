//
// Created by droc101 on 4/21/2024.
//

#include <stdio.h>
#include "LevelLoader.h"
#include "../defines.h"
#include "../Structs/level.h"
#include "../error.h"
#include "../Structs/wall.h"

double ReadDouble(byte *data, int *offset) {
    double d;
    memcpy(&d, data + *offset, sizeof(double));
    *offset += sizeof(double);

    // Reverse the byte order
    for (int i = 0; i < sizeof(double) / 2; i++) {
        byte temp = ((byte *) &d)[i];
        ((byte *) &d)[i] = ((byte *) &d)[sizeof(double) - i - 1];
        ((byte *) &d)[sizeof(double) - i - 1] = temp;
    }

    return d;
}

uint ReadUint(byte *data, int *offset) {
    uint i;
    memcpy(&i, data + *offset, sizeof(uint));
    *offset += sizeof(uint);

    // convert to little endian
    i = ((i >> 24) & 0xff) | // move byte 3 to byte 0
        ((i << 8) & 0xff0000) | // move byte 1 to byte 2
        ((i >> 8) & 0xff00) | // move byte 2 to byte 1
        ((i << 24) & 0xff000000); // byte 0 to byte 3

    return i;
}

Level LoadLevel(byte *data) {
    Level l = CreateLevel();
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
                printf("Wall: %f %f %f %f %u\n", v1, v2, v3, v4, tid);
                fflush(stdout);
                Vector2 va = vec2(v1, v2);
                Vector2 vb = vec2(v3, v4);
                Wall *w = CreateWall(va, vb, tid);
                ListAdd(l.walls, w);
                break;
            }
            case LEVEL_CMD_PLAYER: {
                double x = ReadDouble(data, &i);
                double y = ReadDouble(data, &i);
                double r = ReadDouble(data, &i);
                l.position = vec2(x, y);
                l.rotation = r;
                break;
            }
            case LEVEL_CMD_COLORS: {
                uint sky = ReadUint(data, &i);
                uint floor = ReadUint(data, &i);
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
