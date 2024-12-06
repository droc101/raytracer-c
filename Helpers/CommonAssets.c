//
// Created by droc101 on 7/7/2024.
//

#include "CommonAssets.h"
#include "../Assets/AssetReader.h"
#include "../Assets/Assets.h"
#include "Graphics/Drawing.h"

const byte *wallTextures[WALL_TEXTURE_COUNT] = { // NOLINT(*-interfaces-global-init)
    gztex_level_bricks,
    gztex_level_cross,
    gztex_level_wall2,
    gztex_level_uvtest,
    gztex_level_floor
};

const byte *actorTextures[ACTOR_TEXTURE_COUNT] = { // NOLINT(*-interfaces-global-init)
    gztex_actor_iq,
    gztex_actor_BLOB2,
    gztex_actor_demon,
    gztex_actor_monster1,
    gztex_actor_monster2,
    gztex_actor_monster3,
    gztex_actor_key,
    gztex_actor_coin,
    gztex_actor_bluecoin,
    gztex_actor_goal0,
    gztex_actor_goal1,
    gztex_actor_door
};

Model *skyModel;

void InitCommonAssets()
{
    SetTexParams(gztex_interface_menu_bg_tile, true, true);
    SetTexParams(gztex_interface_menu_bg_tile_red, true, true);
    SetTexParams(gztex_interface_font, false, false);
    SetTexParams(gztex_interface_small_fonts, false, false);
    SetTexParams(gztex_interface_button, true, false);
    SetTexParams(gztex_interface_button_hover, true, false);
    SetTexParams(gztex_interface_button_press, true, false);
    SetTexParams(gztex_interface_slider, true, false);
    SetTexParams(gztex_interface_slider_thumb, true, false);
    SetTexParams(gztex_interface_checkbox_checked, true, false);
    SetTexParams(gztex_interface_checkbox_unchecked, true, false);
    SetTexParams(gztex_interface_radio_checked, true, false);
    SetTexParams(gztex_interface_radio_unchecked, true, false);
    SetTexParams(gztex_interface_focus_rect, true, false);
    SetTexParams(gztex_level_sky, true, true);
    SetTexParams(gztex_vfx_shadow, false, false);

    skyModel = LoadModel(gzobj_model_sky);
}

int FindWallTextureIndex(const byte *tex)
{
    for (int i = 0; i < WALL_TEXTURE_COUNT; i++)
    {
        if (wallTextures[i] == tex)
        {
            return i;
        }
    }
    return -1;
}

void DestroyCommonAssets()
{
    FreeModel(skyModel);
}
