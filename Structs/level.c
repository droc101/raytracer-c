//
// Created by droc101 on 4/21/2024.
//
#include <math.h>
#include "../defines.h"
#include "Vector2.h"
#include "wall.h"
#include "level.h"
#include "../Helpers/drawing.h"
#include "ray.h"
#include "../Helpers/mathex.h"

//#define TEXTURED

Level CreateLevel() {
    Level l;
    l.actors = CreateList();
    l.walls = CreateList();
    l.position = vec2s(0);
    l.rotation = 0;
    l.SkyColor = 0xff82c5ff;
    l.FloorColor = 0xff36322f;
    return l;
}

void RenderCol(SDL_Renderer *renderer, Level l, int col) {
    setColorUint(renderer, 0xFFFFFFFF);
    double angle = atan2(col - WIDTH / 2, WIDTH / 2) + l.rotation;

    RayCastResult raycast = HitscanLevel(l, l.position, angle);

    if (!raycast.Collided) {
        return; // nothing else to do
    }

#pragma region RayCast

    double distance = Vector2Distance(l.position, raycast.CollisonPoint) * cos(angle - l.rotation);

    double height = HEIGHT / distance;
    double y = (HEIGHT - height) / 2;
#pragma  endregion
#pragma region Shade
    double shade = fabs(cos((l.rotation + (1.5 * PI)) - WallGetAngle(raycast.CollisionWall)));
    shade *= (1 - (distance / (WIDTH / 2)));
    shade = max(0.4, min(1, shade));
    shade = floor(shade * 16) / 16;

    byte shadeByte = 255 * shade;
#pragma endregion
#ifdef TEXTURED
#pragma region Texture

    uint *texture = raycast.CollisionWall.tex;
    uint texH = texture[2];
    uint texW = texture[1];

    double wallLength = WallGetLength(raycast.CollisionWall);
    double localX = Vector2Distance(raycast.CollisionWall.a, raycast.CollisonPoint);
    double texCol = (localX / WallGetLength(raycast.CollisionWall)) * texW;

    texCol *= (wallLength / 2);
    texCol = fmod(texCol, texW);

    texCol = wrap(texCol, 0, texW - 1);
#pragma endregion

    int typ = height * texH;

    for (int i = 0; i < height; i++) {

        if (y+i > HEIGHT || y+i < 0) {
            continue;
        }

        int texY = (i / typ);

        uint color = texture_get_pixel(texture, texCol, texY);
        byte r = (color >> 16) & 0xFF;
        byte g = (color >> 8) & 0xFF;
        byte b = (color >> 0) & 0xFF;
        r *= shadeByte;
        g *= shadeByte;
        b *= shadeByte;

        SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);

        SDL_RenderDrawPoint(renderer, col, y+i);
    }
#else
    SDL_SetRenderDrawColor(renderer, shadeByte, shadeByte, shadeByte, SDL_ALPHA_OPAQUE);
    draw_rect(renderer, col, y, 1, height);
#endif
}
