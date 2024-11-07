//
// Created by droc101 on 10/27/24.
//

#include "GOptionsState.h"
#include "GLevelSelectState.h"
#include "GMenuState.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Ray.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/Controls/CheckBox.h"
#include "../Structs/UI/Controls/RadioButton.h"
#include "../Structs/UI/Controls/Slider.h"

UiStack *optionsStack;

void BtnOptionsBack()
{
    GMenuStateSet();
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

void CbOptionsFullscreen(const bool value)
{
    GetState()->options.fullscreen = value;
    SDL_SetWindowFullscreen(GetGameWindow(), value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void RbOptionsRenderer(const bool value, const byte groupId, const byte id)
{
    GetState()->options.renderer = id;
    // Renderer change will happen on next restart
}

void CbOptionsVsync(const bool value)
{
    GetState()->options.vsync = value;
    // VSync change will happen on next restart
}

void GOptionsStateUpdate(GlobalState *State)
{
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE))
    {
        BtnOptionsBack();
    }
}

void SldOptionsMouseSensitivity(const double value)
{
    GetState()->options.mouseSpeed = value;
}

void GOptionsStateRender(GlobalState *State)
{
    // sorry for the confusing variable names
    const Vector2 bg_tile_size = v2(320, 240); // size on screen
    const Vector2 bg_tex_size = texture_size(gztex_interface_menu_bg_tile); // actual size of the texture

    const Vector2 tilesOnScreen = v2(WindowWidth() / bg_tile_size.x, WindowHeight() / bg_tile_size.y);
    const Vector2 tileRegion = v2(tilesOnScreen.x * bg_tex_size.x, tilesOnScreen.y * bg_tex_size.y);
    DrawTextureRegion(v2(0, 0), v2(WindowWidth(), WindowHeight()), gztex_interface_menu_bg_tile, v2(0, 0), tileRegion);

    DrawTextAligned("Options", 32, 0xFFFFFFFF, v2s(0), v2(WindowWidth(), 100), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE,
                    false);

    DrawTextAligned("Changing renderer requires a restart", 16, 0xFFa0a0a0, v2(0, 540), v2(WindowWidth(), 40),
                    FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);

    ProcessUiStack(optionsStack);
    DrawUiStack(optionsStack);
}

void GOptionsStateSet()
{
    if (optionsStack == NULLPTR)
    {
        optionsStack = CreateUiStack();
        int opy = 40;
        const int ops = 25;
        UiStackPush(optionsStack,
                    CreateSliderControl(v2(0, opy), v2(480, 40), "Mouse Sensitivity", SldOptionsMouseSensitivity,
                                        TOP_CENTER, 0.01, 2.00,
                                        GetState()->options.mouseSpeed, 0.01, 0.1, SliderLabelPercent));
        opy += ops * 1.5;
        UiStackPush(optionsStack,
                    CreateSliderControl(v2(0, opy), v2(480, 40), "Master Volume", SldOptionsMasterVolume, TOP_CENTER,
                                        0.0, 1.0, GetState()->options.masterVolume, 0.01, 0.1, SliderLabelPercent));
        opy += ops;
        UiStackPush(optionsStack,
                    CreateSliderControl(v2(0, opy), v2(480, 40), "Music Volume", SldOptionsMusicVolume, TOP_CENTER, 0.0,
                                        1.0, GetState()->options.musicVolume, 0.01, 0.1, SliderLabelPercent));
        opy += ops;
        UiStackPush(optionsStack,
                    CreateSliderControl(v2(0, opy), v2(480, 40), "SFX Volume", SldOptionsSfxVolume, TOP_CENTER, 0.0,
                                        1.0,
                                        GetState()->options.sfxVolume, 0.01, 0.1, SliderLabelPercent));
        opy += ops * 1.5;
        UiStackPush(optionsStack,
                    CreateCheckboxControl(v2(0, opy), v2(480, 40), "Fullscreen", CbOptionsFullscreen, TOP_CENTER,
                                          GetState()->options.fullscreen));
        opy += ops;
        UiStackPush(optionsStack, CreateCheckboxControl(v2(0, opy), v2(480, 40), "VSync", CbOptionsVsync, TOP_CENTER,
                                                        GetState()->options.vsync));
        opy += ops * 1.5;
        UiStackPush(optionsStack,
                    CreateRadioButtonControl(v2(0, opy), v2(480, 40), "OpenGL", RbOptionsRenderer, TOP_CENTER,
                                             GetState()->options.renderer == RENDERER_OPENGL, 0, RENDERER_OPENGL));
        opy += ops;
        UiStackPush(optionsStack,
                    CreateRadioButtonControl(v2(0, opy), v2(480, 40), "Vulkan //todo", RbOptionsRenderer, TOP_CENTER,
                                             GetState()->options.renderer == RENDERER_VULKAN, 0, RENDERER_VULKAN));
        opy += ops;


        UiStackPush(optionsStack, CreateButtonControl(v2(0, -40), v2(480, 40), "Done", BtnOptionsBack, BOTTOM_CENTER));
    }
    optionsStack->focusedControl = -1;

    SetRenderCallback(GOptionsStateRender);
    SetUpdateCallback(GOptionsStateUpdate, NULL, OPTIONS_STATE); // Fixed update is not needed for this state
}
