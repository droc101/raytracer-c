//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include "Helpers/List.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include "config.h"

// "boolean"
#define bool unsigned char // unsigned 8-bit integer (nonzero is true)
#define true 1
#define false 0

#define byte unsigned char // unsigned 8-bit integer
#define ushort unsigned short // unsigned 16-bit integer
#define uint unsigned int // unsigned 32-bit integer
#define ulong unsigned long // unsigned 64-bit integer

#define NULLPTR NULL

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
    int texId;
    double Length;
    double Angle;
    double dx;
    double dy;
    float uvScale;
} Wall;

// Utility functions are in Structs/level.h
typedef struct {
    List *actors;
    List *walls;
    Vector2 position;
    double rotation;
    uint SkyColor;
    uint FloorColor;
    uint FloorLowerColor;
    uint MusicID;
    uint FogColor;
    double FogStart;
    double FogEnd;
    SizedArray *staticWalls;
    SizedArray *staticActors;
} Level;

// Utility functions are in Structs/ray.h
typedef struct {
    Vector2 CollisionPoint;
    double Distance;
    bool Collided;
    Wall CollisionWall;
} RayCastResult;

// Global state of the game
typedef struct {
    Level *level;
    void (*UpdateGame)();
    void (*RenderGame)();
    int hp;
    int maxHp;
    int ammo;
    int maxAmmo;
    ulong frame;
    bool requestExit;
    Mix_Music *music; // background music
    Mix_Chunk *channels[SFX_CHANNEL_COUNT]; // sound effects
    double FakeHeight; // fake camera height for rendering
    int levelID;
} GlobalState;

// Actor (interactable/moving wall) struct
typedef struct {
    Vector2 position;
    double rotation;
    Wall *actorWall;
    bool solid;
    int health;
    void *extra_data;
    void (*Init)(void *self);
    void (*Update)(void *self);
    void (*Destroy)(void *self);
    int actorType;
    byte paramA;
    byte paramB;
    byte paramC;
    byte paramD;
} Actor;

// pi 🥧
#define PI 3.14159265358979323846

#define TARGET_MS (1000 / TARGET_FPS)
#define TARGET_NS (1000000000 / TARGET_FPS) // nanoseconds because precision

#endif //GAME_DEFINES_H
