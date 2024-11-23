//
// Created by droc101 on 11/23/2024.
//

#include "GSoundOptionsState.h"
#include "../GOptionsState.h"
#include "../../Assets/Assets.h"
#include "../../Helpers/Core/Input.h"
#include "../../Helpers/Graphics/Drawing.h"
#include "../../Helpers/Graphics/Font.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/UI/UiStack.h"
#include "../../Structs/UI/Controls/Button.h"
#include "../../Structs/UI/Controls/CheckBox.h"
#include "../../Structs/UI/Controls/RadioButton.h"
#include "../../Structs/UI/Controls/Slider.h"

UiStack *soundOptionsStack;

void BtnSoundOptionsBack()
{
    GOptionsStateSet();
}

void SldOptionsMasterVolume(const double value)
{
    GetState()->options.masterVolume = value;
    UpdateVolume();
}

void SldOptionsMusicVolume(const double value)
{
    GetState()->options.musicVolume = value;
    UpdateVolume();
}

void SldOptionsSfxVolume(const double value)
{
    GetState()->options.sfxVolume = value;
    UpdateVolume();
}

void GSoundOptionsStateUpdate(GlobalState */*State*/)
{
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_B))
    {
        BtnSoundOptionsBack();
    }
}

void GSoundOptionsStateRender(GlobalState */*State*/)
{
    // sorry for the confusing variable names
    const Vector2 bg_tile_size = v2(320, 240); // size on screen
    const Vector2 bg_tex_size = GetTextureSize(gztex_interface_menu_bg_tile); // actual size of the texture

    const Vector2 tilesOnScreen = v2(WindowWidth() / bg_tile_size.x, WindowHeight() / bg_tile_size.y);
    const Vector2 tileRegion = v2(tilesOnScreen.x * bg_tex_size.x, tilesOnScreen.y * bg_tex_size.y);
    DrawTextureRegion(v2(0, 0), v2(WindowWidth(), WindowHeight()), gztex_interface_menu_bg_tile, v2(0, 0), tileRegion);

    DrawTextAligned("Sound Options", 32, 0xFFFFFFFF, v2s(0), v2(WindowWidth(), 100), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE,
                    false);

    ProcessUiStack(soundOptionsStack);
    DrawUiStack(soundOptionsStack);
}

void GSoundOptionsStateSet()
{
    if (soundOptionsStack == NULL)
    {
        soundOptionsStack = CreateUiStack();
        int opy = 40;
        const int ops = 25;
         UiStackPush(soundOptionsStack,
                     CreateSliderControl(v2(0, opy), v2(480, 40), "Master Volume", SldOptionsMasterVolume, TOP_CENTER,
                                         0.0, 1.0, GetState()->options.masterVolume, 0.01, 0.1, SliderLabelPercent));
         opy += ops;
         UiStackPush(soundOptionsStack,
                     CreateSliderControl(v2(0, opy), v2(480, 40), "Music Volume", SldOptionsMusicVolume, TOP_CENTER, 0.0,
                                         1.0, GetState()->options.musicVolume, 0.01, 0.1, SliderLabelPercent));
         opy += ops;
         UiStackPush(soundOptionsStack,
                     CreateSliderControl(v2(0, opy), v2(480, 40), "SFX Volume", SldOptionsSfxVolume, TOP_CENTER, 0.0,
                                         1.0,
                                         GetState()->options.sfxVolume, 0.01, 0.1, SliderLabelPercent));
        opy += ops;


        UiStackPush(soundOptionsStack, CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnSoundOptionsBack, BOTTOM_CENTER));
    }
    UiStackResetFocus(soundOptionsStack);

    SetRenderCallback(GSoundOptionsStateRender);
    SetUpdateCallback(GSoundOptionsStateUpdate, NULL, SOUND_OPTIONS_STATE); // Fixed update is not needed for this state
}
