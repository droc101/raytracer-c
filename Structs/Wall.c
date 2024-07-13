//
// Created by droc101 on 4/21/2024.
//

#include "../defines.h"
#include "Wall.h"
#include <math.h>
#include "../Helpers/Drawing.h"
#include "../Helpers/CommonAssets.h"

Wall *CreateWall(Vector2 a, Vector2 b, SDL_Texture *tex, float uvScale, float uvOffset) {
    Wall *w = malloc(sizeof(Wall));
    w->a = a;
    w->b = b;
    w->tex = tex;
    w->texId = FindWallTextureIndex(tex);
    w->uvScale = uvScale;
    w->uvOffset = uvOffset;
    return w;
}

void FreeWall(Wall *w) {
    // no longer need to free the texture
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

