//
// Created by droc101 on 6/25/2024.
//

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// Default window size
#define DEF_WIDTH 1280
#define DEF_HEIGHT 720

#define MOVE_SPEED 0.125 // Player movement speed
#define MOUSE_SENSITIVITY 100 // higher is less sensitive

//#define KEYBOARD_ROTATION // Uncomment to enable keyboard rotation and disable mouse rotation
#define ROT_SPEED 0.04 // Keyboard rotation speed

#define STR(x) #x
#define TO_STR(x) STR(x)
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 1
#define VERSION TO_STR(VERSION_MAJOR) "." TO_STR(VERSION_MINOR) "." TO_STR(VERSION_PATCH)

#define COPYRIGHT "2024 Droc101 Development"
#define GAME_TITLE "Game."

// See /Debug/FrameGrapher.h for frame grapher settings

#define ENABLE_LEVEL_EDITOR

#define ENABLE_DEBUG_PRINT

#define USE_LEVEL_SELECT

#define PAUSE_EXIT_LEVEL 0

#endif //GAME_CONFIG_H
