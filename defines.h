//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include "Helpers/List.h"
#include <SDL.h>
#include <SDL_mixer.h>

// "boolean"
#define bool unsigned char // unsigned 8-bit integer (nonzero is true)
#define true 1
#define false 0

#define byte unsigned char // unsigned 8-bit integer
#define ushort unsigned short // unsigned 16-bit integer
#define uint unsigned int // unsigned 32-bit integer
#define ulong unsigned long // unsigned 64-bit integer

#define NULLPTR NULL

#define SFX_CHANNEL_COUNT 16

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
    uint FloorLowerColor;
    uint MusicID;
} Level;

// Utility functions are in Structs/ray.h
typedef struct {
    Vector2 CollisionPoint;
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
} GlobalState;

// Actor (interactable/moving wall) struct
typedef struct {
    Vector2 position;
    double rotation;
    Wall *actorWall;
    bool solid;
    void *extra_data;
    void (*Init)(void *self);
    void (*Update)(void *self);
    void (*Destroy)(void *self);
} Actor;

// pi
#define PI 3.14159265358979323846

// Target frame rate (the game is tied to this :D)
#define TARGET_FPS 60
#define TARGET_MS (1000 / TARGET_FPS)
#define TARGET_NS (1000000000 / TARGET_FPS) // nanoseconds because precision

// Default window size
#define DEF_WIDTH 1280
#define DEF_HEIGHT 720

// Player movement speed
#define MOVE_SPEED 0.125
#define ROT_SPEED 0.04

#endif //GAME_DEFINES_H
