//
// Created by droc101 on 4/20/2024.
//

#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "Helpers/Core/List.h"

#define byte uint8_t // unsigned 8-bit integer
#define ushort uint16_t // unsigned 16-bit integer
#define uint uint32_t // unsigned 32-bit integer
#define ulong uint64_t // unsigned 64-bit integer

#define STR(x) #x
#define TO_STR(x) STR(x)

#define STR(x) #x
#define TO_STR(x) STR(x)

// Utility functions are in Structs/Vector2.h
typedef struct Vector2
{
    double x;
    double y;
} Vector2;

typedef struct Camera
{
    float x;
    float z;
    float y;

    float pitch;
    float yaw;
    float roll;

    float fov;
} Camera;

// Utility functions are in Structs/wall.h
typedef struct Wall
{
    Vector2 a; // The first point of the wall
    Vector2 b; // The second point of the wall
    const byte *tex; // The raw asset data for the texture
    int texId; // The texture ID
    double Length; // The length of the wall (Call WallBake to update)
    double Angle; // The angle of the wall (Call WallBake to update)
    double dx;
    double dy;
    float uvScale; // The X scale of the texture
    float uvOffset; // The X offset of the texture
    float height; // height of the wall for rendering. Does not affect collision
} Wall;

// Utility functions are in Structs/level.h
typedef struct Level
{
    List *actors; // The list of actors in the level. You must bake this into staticActors before it is used.
    List *walls; // The list of walls in the level. You must bake this into staticWalls before it is used.
    Vector2 position; // The player's position
    double rotation; // The player's rotation
    uint SkyColor; // The color of the sky
    uint FloorTexture; // The texture index of the floor
    uint CeilingTexture; // The texture index + 1 of the ceiling. 0 is no ceiling
    uint MusicID; // The music ID
    uint FogColor; // The color of the fog
    double FogStart; // The start of the fog
    double FogEnd; // The end of the fog
    SizedArray *staticWalls; // The static array of walls in the level
    SizedArray *staticActors; // The static array of actors in the level
} Level;

// Utility functions are in Structs/ray.h
typedef struct RayCastResult
{
    Vector2 CollisionPoint; // The point of collision
    double Distance; // The distance to the collision
    bool Collided; // Whether the ray collided with anything
    Wall CollisionWall; // The wall that was collided with
} RayCastResult;

typedef struct TextBox
{
    char *text; // The text to display
    int rows; // The number of rows to display
    int cols; // The number of columns to display
    int x; // The x position of the text box
    int y; // The y position of the text box

    int h_align; // The horizontal alignment of the text box
    int v_align; // The vertical alignment of the text box

    int theme; // The theme of the text box

    void (*Close)(void *textBox); // The function to call when the text box is closed
} TextBox;

typedef enum
{
    EDITOR_STATE,
    LEVEL_SELECT_STATE,
    LOGO_SPLASH_STATE,
    MAIN_STATE,
    MENU_STATE,
    PAUSE_STATE,
    OPTIONS_STATE
} CurrentState;

typedef enum Renderer
{
    RENDERER_OPENGL,
    RENDERER_VULKAN,
    RENDERER_MAX
} Renderer;

typedef struct Options
{
    ushort checksum; // Checksum of the options struct (helps prevent corruption)
    Renderer renderer; // The renderer to use
    double musicVolume; // The volume of the music
    double sfxVolume; // The volume of the sound effects
    double masterVolume; // The master volume
    double mouseSpeed; // The look speed (it affects controller speed too)
    bool fullscreen; // Whether the game is fullscreen
    bool vsync; // Whether vsync is enabled
    bool controllerMode; // Whether the game is in controller mode
} __attribute__((packed)) Options; // This is packed because it is saved to disk

// Global state of the game
typedef struct GlobalState
{
    Level *level; // Current level
    void (*UpdateGame)(struct GlobalState *State); // State update function
    void (*RenderGame)(void *State); // State render function
    SDL_TimerID FixedFramerateUpdate; // Timer for fixed framerate update
    CurrentState currentState; // The current state of the game
    int hp; // Player health
    int maxHp; // Player max health
    int ammo; // Player ammo
    int maxAmmo; // Player max ammo
    int coins; // The number of coins the player has
    int blueCoins; // The number of blue coins the player has
    ulong physicsFrame; // The number of physics frames that have passed since the last game state change
    bool requestExit; // Request to exit the game
    Mix_Music *music; // background music
    Mix_Chunk *channels[SFX_CHANNEL_COUNT]; // sound effects
    double CameraY; // The Y position of the camera
    int levelID; // The current level ID

    bool textBoxActive; // Whether the text box is active
    TextBox textBox; // The text box
    int textBoxPage; // The current page of the text box

    Camera *cam; // The camera

    Options options; // Game options

    char executablePath[261]; // The path to the executable

    double uiScale; // The scale of the UI.
} GlobalState;

// Actor (interactable/moving wall) struct
typedef struct Actor
{
    Vector2 position; // The position of the actor
    double rotation; // The rotation of the actor
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
#define PIf 3.14159265358979323846f

#define PHYSICS_TARGET_MS (1000 / PHYSICS_TARGET_TPS)
#define PHYSICS_TARGET_NS (1000000000 / PHYSICS_TARGET_TPS) // nanoseconds because precision

#endif //GAME_DEFINES_H
