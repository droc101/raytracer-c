//
// Created by droc101 on 7/7/2024.
//

#ifndef GAME_COMMONASSETS_H
#define GAME_COMMONASSETS_H

#include <SDL.h>

#define WALL_TEXTURE_COUNT 3
#define ACTOR_TEXTURE_COUNT 11

extern SDL_Texture *menu_bg_tex;
extern SDL_Texture *menu_logo_tex;
extern SDL_Texture *skyTex;
extern SDL_Texture *menu_bg_tex_red;
extern SDL_Texture *fontTexture;
extern SDL_Texture *smallFontTexture;
extern SDL_Texture *hudCoinTexture;
extern SDL_Texture *hudBlueCoinTexture;

extern SDL_Texture *wallTextures[WALL_TEXTURE_COUNT];
extern SDL_Texture *actorTextures[ACTOR_TEXTURE_COUNT];

void InitCommonAssets();

int FindWallTextureIndex(SDL_Texture *tex);

#endif //GAME_COMMONASSETS_H
