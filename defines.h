//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include "Helpers/List.h"

// "boolean"
#define bool unsigned char
#define true 1
#define false 0

#define byte unsigned char
#define ushort unsigned short
#define uint unsigned int
#define ulong unsigned long

typedef struct {
    double x;
    double y;
} Vector2;

typedef struct {
    Vector2 a;
    Vector2 b;
    uint *tex;
} Wall;

typedef struct {
    List *actors;
    List *walls;
    Vector2 position;
    double rotation;
    uint SkyColor;
    uint FloorColor;
} Level;

typedef struct {
    Vector2 CollisonPoint;
    bool Collided;
    Wall CollisionWall;
} RayCastResult;

#define PI 3.14159265358979323846

#define TARGET_FPS 60
#define TARGET_MS (1000 / TARGET_FPS)

#define WIDTH 1066
#define HEIGHT 600

#endif //GAME_DEFINES_H
