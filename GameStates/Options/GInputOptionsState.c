//
// Created by droc101 on 11/23/2024.
//

#include "GInputOptionsState.h"
#include "../GOptionsState.h"
#include "../../Assets/Assets.h"
#include "../../Helpers/Core/Input.h"
#include "../../Helpers/Graphics/Drawing.h"
#include "../../Helpers/Graphics/Font.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/UI/UiStack.h"
#include "../../Structs/UI/Controls/Button.h"
#include "../../Structs/UI/Controls/CheckBox.h"
#include "../../Structs/UI/Controls/Slider.h"

UiStack *inputOptionsStack;

void BtnInputOptionsBack()
{
    GOptionsStateSet();
}

void GInputOptionsStateUpdate(GlobalState */*State*/)
{
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_B))
    {
        BtnInputOptionsBack();
    }
}

void SldOptionsMouseSensitivity(const double value)
{
    GetState()->options.mouseSpeed = value;
}

void CbOptionsControllerMode(const bool value)
{
    GetState()->options.controllerMode = value;
}

void GInputOptionsStateRender(GlobalState */*State*/)
{
    // sorry for the confusing variable names
    const Vector2 bg_tile_size = v2(320, 240); // size on screen
    const Vector2 bg_tex_size = GetTextureSize(gztex_interface_menu_bg_tile); // actual size of the texture

    const Vector2 tilesOnScreen = v2(WindowWidth() / bg_tile_size.x, WindowHeight() / bg_tile_size.y);
    const Vector2 tileRegion = v2(tilesOnScreen.x * bg_tex_size.x, tilesOnScreen.y * bg_tex_size.y);
    DrawTextureRegion(v2(0, 0), v2(WindowWidth(), WindowHeight()), gztex_interface_menu_bg_tile, v2(0, 0), tileRegion);

    DrawTextAligned("Input Options", 32, 0xFFFFFFFF, v2s(0), v2(WindowWidth(), 100), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE,
                    false);

    ProcessUiStack(inputOptionsStack);
    DrawUiStack(inputOptionsStack);
}

void GInputOptionsStateSet()
{
    if (inputOptionsStack == NULL)
    {
        inputOptionsStack = CreateUiStack();
        int opy = 40;
        const int ops = 25;

         UiStackPush(inputOptionsStack,
                     CreateSliderControl(v2(0, opy), v2(480, 40), "Camera Sensitivity", SldOptionsMouseSensitivity,
                                         TOP_CENTER, 0.01, 2.00,
                                         GetState()->options.mouseSpeed, 0.01, 0.1, SliderLabelPercent));
         opy += ops;
         UiStackPush(inputOptionsStack,
                     CreateCheckboxControl(v2(0, opy), v2(480, 40), "Controller Mode", CbOptionsControllerMode, TOP_CENTER,
                                           GetState()->options.controllerMode));
        opy += ops;


        UiStackPush(inputOptionsStack, CreateButtonControl(v2(0, -40), v2(480, 40), "Back", BtnInputOptionsBack, BOTTOM_CENTER));
    }
    UiStackResetFocus(inputOptionsStack);

    SetRenderCallback(GInputOptionsStateRender);
    SetUpdateCallback(GInputOptionsStateUpdate, NULL, INPUT_OPTIONS_STATE); // Fixed update is not needed for this state
}
