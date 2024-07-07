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

#define MOVE_SPEED 0.125 // Player movement speed
#define MOUSE_SENSITIVITY 100 // higher is less sensitive

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
#define STR(x) #x
#define TO_STR(x) STR(x)
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 1
#define VERSION TO_STR(VERSION_MAJOR) "." TO_STR(VERSION_MINOR) "." TO_STR(VERSION_PATCH)

#define COPYRIGHT "2024 Droc101 Development"
#define GAME_TITLE "Game." // Used for window title

// The level ID to load when pause exiting
#define PAUSE_EXIT_LEVEL 2

// Number of sound effect channels
// (sound effects that can play at the same time)
#define SFX_CHANNEL_COUNT 16

// Target frame rate (the game is tied to this, so be careful! :D)
#define TARGET_FPS 60

#endif //GAME_CONFIG_H
