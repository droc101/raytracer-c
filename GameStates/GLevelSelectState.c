//
// Created by droc101 on 7/4/2024.
//

#include "GLevelSelectState.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/GlobalState.h"
#include <stdio.h>
#include "GMainState.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/LevelEntries.h"
#include "GMenuState.h"
#include "../Helpers/CommonAssets.h"

int GLevelSelectState_SelectedLevel = 0;

void GLevelSelectStateUpdate(GlobalState * State) {
    if (IsKeyJustPressed(SDL_SCANCODE_DOWN)) {
        GLevelSelectState_SelectedLevel--;
        GLevelSelectState_SelectedLevel = wrap(GLevelSelectState_SelectedLevel, 0, LEVEL_COUNT);
    } else if (IsKeyJustPressed(SDL_SCANCODE_UP)) {
        GLevelSelectState_SelectedLevel++;
        GLevelSelectState_SelectedLevel = wrap(GLevelSelectState_SelectedLevel, 0, LEVEL_COUNT);
    } else if (IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
        // check if the level is a stub
        if (gLevelEntries[GLevelSelectState_SelectedLevel].levelData == NULLPTR) {
            GMenuStateSet();
            return;
        }
        ChangeLevelByID(GLevelSelectState_SelectedLevel);
        GMainStateSet();
    }
}

void GLevelSelectStateRender(GlobalState * State) {
    setColorUint(0xFF123456);
    ClearColor(0xFF123456);

    Vector2 bg_tile_size = v2(320, 240);
    for (int x = 0; x < WindowWidth(); x += bg_tile_size.x) {
        for (int y = 0; y < WindowHeight(); y += bg_tile_size.y) {
            DrawTexture(v2(x, y), v2(bg_tile_size.x, bg_tile_size.y), gztex_interface_menu_bg_tile);
        }
    }

    FontDrawString(v2(20, 20), GAME_TITLE, 128, 0xFFFFFFFF, false);
    FontDrawString(v2(20, 150), "Press Space to start.", 32, 0xFFa0a0a0, false);

    char *levelName = gLevelEntries[GLevelSelectState_SelectedLevel].internalName;
    char levelNameBuffer[64];
    sprintf(levelNameBuffer, "%02d %s", GLevelSelectState_SelectedLevel+1, levelName);
    DrawTextAligned(levelNameBuffer, 32, 0xFFFFFFFF, v2(50, 300), v2(WindowWidth() - 50, 300), FONT_HALIGN_LEFT, FONT_VALIGN_MIDDLE, false);
}

void GLevelSelectStateSet() {
    SetRenderCallback(GLevelSelectStateRender);
    SetUpdateCallback(GLevelSelectStateUpdate, NULL, LEVEL_SELECT_STATE); // Fixed update is not needed for this state
}

