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

void InitCommonAssets() {
    menu_bg_tex = ToSDLTexture(gztex_interface_menu_bg_tile, FILTER_LINEAR);
    menu_logo_tex = ToSDLTexture(gztex_interface_menu_logo, FILTER_LINEAR);
    skyTex = ToSDLTexture(gztex_level_sky, FILTER_LINEAR);
    menu_bg_tex_red = ToSDLTexture(gztex_interface_menu_bg_tile_red, FILTER_LINEAR);
    fontTexture = ToSDLTexture((const unsigned char *) gztex_interface_font, FILTER_NEAREST);
    smallFontTexture = ToSDLTexture((const unsigned char *) gztex_interface_small_fonts, FILTER_NEAREST);
    hudCoinTexture = ToSDLTexture((const unsigned char *) gztex_interface_hud_ycoin, FILTER_NEAREST);
    hudBlueCoinTexture = ToSDLTexture((const unsigned char *) gztex_interface_hud_bcoin, FILTER_NEAREST);
}
