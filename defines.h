//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include "Helpers/List.h"
#include "SDL.h"

// "boolean"
#define bool unsigned char // unsigned 8-bit integer (nonzero is true)
#define true 1
#define false 0

#define byte unsigned char // unsigned 8-bit integer
#define ushort unsigned short // unsigned 16-bit integer
#define uint unsigned int // unsigned 32-bit integer
#define ulong unsigned long // unsigned 64-bit integer

// Utility functions are in Structs/Vector2.h
typedef struct {
    double x;
    double y;
} Vector2;

// Utility functions are in Structs/wall.h
typedef struct {
    Vector2 a;
    Vector2 b;
    SDL_Texture *tex;
} Wall;

// Utility functions are in Structs/level.h
typedef struct {
    List *actors;
    List *walls;
    Vector2 position;
    double rotation;
    uint SkyColor;
    uint FloorColor;
} Level;

// Utility functions are in Structs/ray.h
typedef struct {
    Vector2 CollisonPoint;
    bool Collided;
    Wall CollisionWall;
} RayCastResult;

typedef struct {
    Level *level;
    void (*UpdateGame)();
    void (*RenderGame)();
    int hp;
    int maxHp;
    int ammo;
    int maxAmmo;
    ulong frame;
} GlobalState;

#define PI 3.14159265358979323846

#define TARGET_FPS 120
#define TARGET_MS (1000 / TARGET_FPS)

#define WIDTH 1280
#define HEIGHT 720

#define MOVE_SPEED 0.15
#define ROT_SPEED 0.02

#endif //GAME_DEFINES_H
