//
// Created by droc101 on 7/4/2024.
//

#include "GLevelSelectState.h"
#include "../Helpers/Input.h"
#include "../Helpers/Drawing.h"
#include "../Helpers/Font.h"
#include "../Structs/GlobalState.h"
#include <stdio.h>
#include "GMainState.h"
#include "../Assets/AssetReader.h"
#include "../Helpers/LevelLoader.h"
#include "../config.h"
#include "../Helpers/MathEx.h"
#include "../Helpers/LevelEntries.h"

int GLevelSelectState_SelectedLevel = 0;

void GLevelSelectStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        GMainStateSet();
    } else if (IsKeyJustPressed(SDL_SCANCODE_DOWN)) {
        GLevelSelectState_SelectedLevel--;
        GLevelSelectState_SelectedLevel = wrap(GLevelSelectState_SelectedLevel, 0, LEVEL_COUNT);
    } else if (IsKeyJustPressed(SDL_SCANCODE_UP)) {
        GLevelSelectState_SelectedLevel++;
        GLevelSelectState_SelectedLevel = wrap(GLevelSelectState_SelectedLevel, 0, LEVEL_COUNT);
    } else if (IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        ChangeLevelByID(GLevelSelectState_SelectedLevel);
        GMainStateSet();
    }
}

void GLevelSelectStateRender() {
    setColorUint(0xFF123456);
    SDL_RenderClear(GetRenderer());

    FontDrawString(vec2(20, 20), GAME_TITLE, 128, 0xFFFFFFFF);
    FontDrawString(vec2(20, 150), "Press Space to start.", 32, 0xFFa0a0a0);

    char * levelName = gLevelEntries[GLevelSelectState_SelectedLevel].internalName;
    char levelNameBuffer[64];
    sprintf(levelNameBuffer, "%02x  %s", GLevelSelectState_SelectedLevel+1, levelName);
    DrawTextAligned(levelNameBuffer, 32, 0xFFFFFFFF, vec2(50, 300), vec2(WindowWidth() - 50, 300), FONT_HALIGN_LEFT, FONT_VALIGN_MIDDLE);
}

void GLevelSelectStateSet() {
    SetRenderCallback(GLevelSelectStateRender);
    SetUpdateCallback(GLevelSelectStateUpdate);
}

