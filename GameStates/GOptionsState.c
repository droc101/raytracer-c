//
// Created by droc101 on 10/27/24.
//

#include "GOptionsState.h"
#include "GMenuState.h"
#include "../Helpers/Core/Input.h"
#include "../Structs/Ray.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/GlobalState.h"
#include "GLevelSelectState.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/TextBox.h"
#include "GMainState.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/Controls/Slider.h"

UiStack *optionsStack;

void BtnOptionsBack() {
    GMenuStateSet();
}

void SldOptionsMasterVolume(double value) {
    GetState()->options.masterVolume = value;
    UpdateVolume();
}

void SldOptionsMusicVolume(double value) {
    GetState()->options.musicVolume = value;
    UpdateVolume();
}

void SldOptionsSfxVolume(double value) {
    GetState()->options.sfxVolume = value;
    UpdateVolume();
}

void GOptionsStateUpdate(GlobalState * State) {
}

void GOptionsStateRender(GlobalState * State) {
    // sorry for the confusing variable names
    Vector2 bg_tile_size = v2(320, 240); // size on screen
    Vector2 bg_tex_size = texture_size(gztex_interface_menu_bg_tile); // actual size of the texture

    Vector2 tilesOnScreen = v2(WindowWidth() / bg_tile_size.x, WindowHeight() / bg_tile_size.y);
    Vector2 tileRegion = v2(tilesOnScreen.x * bg_tex_size.x, tilesOnScreen.y * bg_tex_size.y);
    DrawTextureRegion(v2(0, 0), v2(WindowWidth(), WindowHeight()), gztex_interface_menu_bg_tile, v2(0, 0), tileRegion);

    DrawTextAligned("Options", 32, 0xFFFFFFFF, v2s(0), v2(WindowWidth(), 200), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, false);

    ProcessUiStack(optionsStack);
    DrawUiStack(optionsStack);
}

void GOptionsStateSet() {

    if (optionsStack == NULLPTR) {
        optionsStack = CreateUiStack();
        UiStackPush(optionsStack, CreateSliderControl(v2(0, 80), v2(480, 40), "Master Volume", SldOptionsMasterVolume, TOP_CENTER, 0.0, 1.0, GetState()->options.masterVolume, 0.01, 0.1));
        UiStackPush(optionsStack, CreateSliderControl(v2(0, 105), v2(480, 40), "Music Volume", SldOptionsMusicVolume, TOP_CENTER, 0.0, 1.0, GetState()->options.musicVolume, 0.01, 0.1));
        UiStackPush(optionsStack, CreateSliderControl(v2(0, 130), v2(480, 40), "SFX Volume", SldOptionsSfxVolume, TOP_CENTER, 0.0, 1.0, GetState()->options.sfxVolume, 0.01, 0.1));
        UiStackPush(optionsStack, CreateButtonControl(v2(0, -80), v2(480, 40), "Done", BtnOptionsBack, BOTTOM_CENTER));
    }

    SetRenderCallback(GOptionsStateRender);
    SetUpdateCallback(GOptionsStateUpdate, NULL, OPTIONS_STATE); // Fixed update is not needed for this state
}

