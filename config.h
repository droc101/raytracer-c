//
// Created by droc101 on 6/25/2024.
//

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#pragma region Window Settings

// Default window size
#define DEF_WIDTH 1280
#define DEF_HEIGHT 720

// Minimum and maximum window size
#define MIN_WIDTH 800
#define MIN_HEIGHT 600

// Maximum window size
#define MAX_WIDTH 9999
#define MAX_HEIGHT 9999

#pragma endregion

#pragma region Movement

#define MOVE_SPEED 0.1 // Player movement speed
#define SLOW_MOVE_SPEED 0.01 // Player movement speed when shift is held
#define MOUSE_SENSITIVITY 200 // higher is less sensitive

//#define KEYBOARD_ROTATION // Uncomment to enable keyboard rotation and disable mouse rotation
#define ROT_SPEED 0.04 // Keyboard rotation speed

#pragma endregion

#pragma region Debug

// See /Debug/FrameGrapher.h for frame grapher settings

// Press F6 while in game to open the level editor
#define ENABLE_LEVEL_EDITOR

// Show debug print statements in the top left corner
#define ENABLE_DEBUG_PRINT

// Shows a level select after main menu and pause exit course.
#define USE_LEVEL_SELECT

// Show error trace in release builds (function, source, and line #)
#define ERROR_TRACE_IN_RELEASE

#pragma endregion

// Program information
#define VERSION "0.0.1"
#define COPYRIGHT "2024 Droc101 Development"
#define GAME_TITLE "Game." // Used for window title

#define APPDATA_ORG_NAME "Droc101 Development"
#define APPDATA_APP_NAME "Game"

// The level ID to load when pause exiting
#define PAUSE_EXIT_LEVEL 2

// The level to start on when the game loads
#define STARTING_LEVEL 2

// Number of sound effect channels
// (sound effects that can play at the same time)
#define SFX_CHANNEL_COUNT 16

// Target frame rate (the game is tied to this, so be careful! :D)
#define TARGET_FPS 60

#define FOV 90 // Field of view
#define NEAR_Z 0.01
#define FAR_Z 1000

#define MSAA_SAMPLES 4 // usually 2, 4, or 8

#endif //GAME_CONFIG_H
