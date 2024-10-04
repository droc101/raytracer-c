//
// Created by droc101 on 7/7/2024.
//

#ifndef GAME_COMMONASSETS_H
#define GAME_COMMONASSETS_H

#include <SDL.h>
#include "../defines.h"

#define WALL_TEXTURE_COUNT 4
#define ACTOR_TEXTURE_COUNT 11

extern SDL_Texture *menu_bg_tex;
extern SDL_Texture *menu_logo_tex;
extern SDL_Texture *skyTex;
extern SDL_Texture *menu_bg_tex_red;
extern SDL_Texture *fontTexture;
extern SDL_Texture *smallFontTexture;
extern SDL_Texture *hudCoinTexture;
extern SDL_Texture *hudBlueCoinTexture;
extern SDL_Texture  *studioLogoTex;

extern const byte *wallTextures[WALL_TEXTURE_COUNT];
extern const byte *actorTextures[ACTOR_TEXTURE_COUNT];

void InitCommonAssets();

int FindWallTextureIndex(const byte *tex);

#endif //GAME_COMMONASSETS_H
