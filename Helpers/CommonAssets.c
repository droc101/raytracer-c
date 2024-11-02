//
// Created by droc101 on 7/7/2024.
//

#include "CommonAssets.h"
#include "../Assets/Assets.h"
#include "Graphics/Drawing.h"

const byte *wallTextures[WALL_TEXTURE_COUNT] = {
        gztex_level_bricks,
        gztex_level_cross,
        gztex_level_wall2,
        gztex_level_uvtest,
        gztex_level_floor
};

const byte *actorTextures[ACTOR_TEXTURE_COUNT] = {
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
};

void InitCommonAssets() {
    SetTexParams(gztex_interface_menu_bg_tile, true, true);
    SetTexParams(gztex_interface_menu_bg_tile_red, true, true);
    SetTexParams(gztex_vfx_shadow, false, false);
}

int FindWallTextureIndex(const byte *tex) {
    for (int i = 0; i < WALL_TEXTURE_COUNT; i++) {
        if (wallTextures[i] == tex) {
            return i;
        }
    }
    return -1;
}
