//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include "Helpers/List.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdint.h>
#include "config.h"

// "boolean"
#define bool unsigned char // unsigned 8-bit integer (nonzero is true)
#define true 1
#define false 0

#define byte uint8_t // unsigned 8-bit integer
#define ushort uint16_t // unsigned 16-bit integer
#define uint uint32_t // unsigned 32-bit integer
#define ulong uint64_t // unsigned 64-bit integer

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
    Level *level; // Current level
    void (*UpdateGame)(); // State update function
    void (*RenderGame)(); // State render function
    int hp; // Player health
    int maxHp; // Player max health
    int ammo; // Player ammo
    int maxAmmo; // Player max ammo
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
    Wall *actorWall; // (0,0) in this wall is the actor's position (also transformed by rotation)
    bool solid; // can the player walk through this actor?
    int health; // health. may be unused for some actors
    void *extra_data; // extra data for the actor
    void (*Init)(void *self); // call once to set up the actor
    void (*Update)(void *self); // call every frame to update the actor
    void (*Destroy)(void *self); // call once to clean up the actor
    int actorType; // type of actor. do not change this after creation.
    byte paramA; // extra parameters for the actor. saved in level data, so can be used during Init
    byte paramB;
    byte paramC;
    byte paramD;
} Actor;

// pi 🥧
#define PI 3.14159265358979323846

#define TARGET_MS (1000 / TARGET_FPS)
#define TARGET_NS (1000000000 / TARGET_FPS) // nanoseconds because precision

#endif //GAME_DEFINES_H
