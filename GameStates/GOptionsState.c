//
// Created by droc101 on 10/27/24.
//

#include "GOptionsState.h"
#include "GMenuState.h"
#include "../Assets/Assets.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/GlobalState.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/UI/Controls/Button.h"
#include "Options/GInputOptionsState.h"
#include "Options/GSoundOptionsState.h"
#include "Options/GVideoOptionsState.h"

UiStack *optionsStack;

void BtnOptionsBack()
{
    GMenuStateSet();
}

void GOptionsStateUpdate(GlobalState */*State*/)
{
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_B))
    {
        BtnOptionsBack();
    }
}

void GOptionsStateRender(GlobalState */*State*/)
{
    // sorry for the confusing variable names
    const Vector2 bg_tile_size = v2(320, 240); // size on screen
    const Vector2 bg_tex_size = GetTextureSize(gztex_interface_menu_bg_tile); // actual size of the texture

    const Vector2 tilesOnScreen = v2(WindowWidth() / bg_tile_size.x, WindowHeight() / bg_tile_size.y);
    const Vector2 tileRegion = v2(tilesOnScreen.x * bg_tex_size.x, tilesOnScreen.y * bg_tex_size.y);
    DrawTextureRegion(v2(0, 0), v2(WindowWidth(), WindowHeight()), gztex_interface_menu_bg_tile, v2(0, 0), tileRegion);

    DrawTextAligned("Options", 32, 0xFFFFFFFF, v2s(0), v2(WindowWidth(), 100), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE,
                    false);

    ProcessUiStack(optionsStack);
    DrawUiStack(optionsStack);
}

void GOptionsStateSet()
{
    if (optionsStack == NULL)
    {
        optionsStack = CreateUiStack();
        int opy = 40;
        const int ops = 25;

        UiStackPush(optionsStack, CreateButtonControl(v2(0, opy), v2(480, 40), "Video Options", GVideoOptionsStateSet, TOP_CENTER));
        opy += ops;
        UiStackPush(optionsStack, CreateButtonControl(v2(0, opy), v2(480, 40), "Sound Options", GSoundOptionsStateSet, TOP_CENTER));
        opy += ops;
        UiStackPush(optionsStack, CreateButtonControl(v2(0, opy), v2(480, 40), "Input Options", GInputOptionsStateSet, TOP_CENTER));
        opy += ops;

        UiStackPush(optionsStack, CreateButtonControl(v2(0, -40), v2(480, 40), "Done", BtnOptionsBack, BOTTOM_CENTER));
    }
    UiStackResetFocus(optionsStack);

    SetRenderCallback(GOptionsStateRender);
    SetUpdateCallback(GOptionsStateUpdate, NULL, OPTIONS_STATE); // Fixed update is not needed for this state
}
