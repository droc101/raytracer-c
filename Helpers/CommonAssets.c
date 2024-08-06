//
// Created by droc101 on 7/7/2024.
//

#include "CommonAssets.h"
#include "../Assets/Assets.h"
#include "../Helpers/Drawing.h"

SDL_Texture *menu_bg_tex;
SDL_Texture *menu_logo_tex;
SDL_Texture *skyTex;
SDL_Texture *menu_bg_tex_red;
SDL_Texture *fontTexture;
SDL_Texture *smallFontTexture;

SDL_Texture *hudCoinTexture;
SDL_Texture *hudBlueCoinTexture;

SDL_Texture  *studioLogoTex;

const byte *rawWallTextures[WALL_TEXTURE_COUNT] = {
        gztex_level_bricks,
        gztex_level_cross,
        gztex_level_wall2,
};

const byte *rawActorTextures[ACTOR_TEXTURE_COUNT] = {
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

SDL_Texture *wallTextures[WALL_TEXTURE_COUNT];
SDL_Texture *actorTextures[ACTOR_TEXTURE_COUNT];

void InitCommonAssets() {
    menu_bg_tex = ToSDLTexture(gztex_interface_menu_bg_tile, FILTER_LINEAR);
    menu_logo_tex = ToSDLTexture(gztex_interface_menu_logo, FILTER_LINEAR);
    skyTex = ToSDLTexture(gztex_level_sky, FILTER_LINEAR);
    menu_bg_tex_red = ToSDLTexture(gztex_interface_menu_bg_tile_red, FILTER_LINEAR);
    fontTexture = ToSDLTexture((const unsigned char *) gztex_interface_font, FILTER_NEAREST);
    smallFontTexture = ToSDLTexture((const unsigned char *) gztex_interface_small_fonts, FILTER_NEAREST);
    hudCoinTexture = ToSDLTexture((const unsigned char *) gztex_interface_hud_ycoin, FILTER_NEAREST);
    hudBlueCoinTexture = ToSDLTexture((const unsigned char *) gztex_interface_hud_bcoin, FILTER_NEAREST);
    studioLogoTex = ToSDLTexture(gztex_interface_studio, FILTER_LINEAR);

    for (int i = 0; i < WALL_TEXTURE_COUNT; i++) {
        wallTextures[i] = ToSDLTexture(rawWallTextures[i], FILTER_NEAREST);
    }

    for (int i = 0; i < ACTOR_TEXTURE_COUNT; i++) {
        actorTextures[i] = ToSDLTexture(rawActorTextures[i], FILTER_NEAREST);
    }
}

int FindWallTextureIndex(SDL_Texture *tex) {
    for (int i = 0; i < WALL_TEXTURE_COUNT; i++) {
        if (wallTextures[i] == tex) {
            return i;
        }
    }
    return -1;
}
