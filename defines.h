//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include "Helpers/Core/List.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

#define byte uint8_t // unsigned 8-bit integer
#define ushort uint16_t // unsigned 16-bit integer
#define uint uint32_t // unsigned 32-bit integer
#define ulong uint64_t // unsigned 64-bit integer

#define NULLPTR NULL

// Utility functions are in Structs/Vector2.h
typedef struct Vector2 {
    double x;
    double y;
} Vector2;

typedef struct Camera {
    float x;
    float z;
    float y;

    float pitch;
    float yaw;
    float roll;

    float fov;
} Camera;

// Utility functions are in Structs/wall.h
typedef struct Wall {
    Vector2 a;
    Vector2 b;
    const byte *tex;
    int texId;
    double Length;
    double Angle;
    double dx;
    double dy;
    float uvScale;
    float uvOffset;
    float height; // height of the wall for rendering. Does not affect collision
} Wall;

// Utility functions are in Structs/level.h
typedef struct Level {
    List *actors;
    List *walls;
    Vector2 position;
    double rotation;
    uint SkyColor;
    uint FloorTexture;
    uint CeilingTexture; // 0 for none
    uint MusicID;
    uint FogColor;
    double FogStart;
    double FogEnd;
    SizedArray *staticWalls;
    SizedArray *staticActors;
} Level;

// Utility functions are in Structs/ray.h
typedef struct RayCastResult {
    Vector2 CollisionPoint;
    double Distance;
    bool Collided;
    Wall CollisionWall;
} RayCastResult;

typedef struct TextBox {
    char *text;
    int rows;
    int cols;
    int x;
    int y;

    int h_align;
    int v_align;

    int theme;

    void (*Close)(void *tbox);
} TextBox;

typedef enum {
    EDITOR_STATE,
    LEVEL_SELECT_STATE,
    LOGO_SPLASH_STATE,
    MAIN_STATE,
    MENU_STATE,
    PAUSE_STATE,
    OPTIONS_STATE
} CurrentState;

typedef enum Renderer {
    RENDERER_OPENGL,
    RENDERER_VULKAN
} Renderer;

typedef struct Options {
    Renderer renderer;
    double musicVolume;
    double sfxVolume;
    double masterVolume;
    double uiScale;
    bool fullscreen;
    bool vsync;
} __attribute__((packed)) Options; // This is packed because it is saved to disk

// Global state of the game
typedef struct GlobalState {
    Level *level; // Current level
    void (*UpdateGame)(struct GlobalState* State); // State update function
    void (*RenderGame)(void* State); // State render function
    SDL_TimerID FixedFramerateUpdate;
    CurrentState currentState;
    int hp; // Player health
    int maxHp; // Player max health
    int ammo; // Player ammo
    int maxAmmo; // Player max ammo
    int coins;
    int blueCoins;
    ulong physicsFrame;
    bool requestExit;
    Mix_Music *music; // background music
    Mix_Chunk *channels[SFX_CHANNEL_COUNT]; // sound effects
    double CameraY;
    int levelID;

    bool textBoxActive;
    TextBox textBox;
    int textBoxPage;

    Camera *cam;

    Options options;

    char executablePath[261];
} GlobalState;

// Actor (interactable/moving wall) struct
typedef struct Actor {
    Vector2 position;
    double rotation;
    Wall *actorWall; // (0,0) in this wall is the actor's position (also transformed by rotation)
    bool solid; // can the player walk through this actor?
    int health; // health. may be unused for some actors
    void *extra_data; // extra data for the actor
    void (*Init)(void *self); // call once to set up the actor
    void (*Update)(void *self); // call every physicsFrame to update the actor
    void (*Destroy)(void *self); // call once to clean up the actor
    int actorType; // type of actor. do not change this after creation.
    byte paramA; // extra parameters for the actor. saved in level data, so can be used during Init
    byte paramB;
    byte paramC;
    byte paramD;
    float yPosition; // y position for rendering. Does not affect collision
    bool showShadow; // should the actor cast a shadow?
    float shadowSize; // size of the shadow
} Actor;

// pi 🥧
#define PI 3.14159265358979323846

#define PHYSICS_TARGET_MS (1000 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_NS (1000000000 / PHYSICS_TARGET_TPS) // nanoseconds because precision

#endif //GAME_DEFINES_H
