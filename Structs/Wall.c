//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "Wall.h"
#include <math.h>
#include "../Helpers/Drawing.h"
#include "Vector2.h"

const byte *wallTextures[] = {
        gztex_level_bricks,
        gztex_level_cross,
        gztex_level_wall2,
        gztex_actor_iq,
        gztex_actor_BLOB2,
        gztex_actor_demon,
        gztex_actor_monster1,
        gztex_actor_monster2,
        gztex_actor_monster3,
        gztex_actor_key
};

int GetTextureCount() {
    return sizeof(wallTextures) / sizeof(byte *);
}

SDL_Texture *LoadWallTexture(int index) {
    return ToSDLTexture(wallTextures[index], FILTER_NEAREST);
}

Wall *CreateWall(Vector2 a, Vector2 b, uint tex, float uvScale) {
    Wall *w = malloc(sizeof(Wall));
    w->a = a;
    w->b = b;
    w->tex = ToSDLTexture((const unsigned char *) wallTextures[tex], FILTER_NEAREST);
    w->texId = tex;
    w->uvScale = uvScale;
    return w;
}

void FreeWall(Wall *w) {
    SDL_DestroyTexture(w->tex);
}

double WallGetLength(Wall w) {
    return sqrt(pow(w.b.x - w.a.x, 2) + pow(w.b.y - w.a.y, 2));
}

double WallGetAngle(Wall w) {
    return atan2(w.b.y - w.a.y, w.b.x - w.a.x);
}

double WallBake(Wall *w) {
    w->Length = WallGetLength(*w);
    w->Angle = WallGetAngle(*w);
    w->dx = w->a.x - w->b.x;
    w->dy = w->a.y - w->b.y;
    return w->Length;
}

