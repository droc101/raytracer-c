//
// Created by droc101 on 6/22/2024.
//

#include "Collision.h"
#include "../Structs/Wall.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "../Structs/Actor.h"

Vector2 CollideWall(Wall *w, Vector2 position, Vector2 moveVec) {
    double dx = w->b.x - w->a.x;
    double dy = w->b.y - w->a.y;
    int mult = (position.x - w->a.x) * (w->b.y - w->a.y) - (position.y - w->a.y) * (w->b.x - w->a.x) < 0 ? -1 : 1;
    double hitboxSize = mult * WALL_HITBOX_EXTENTS;
    Vector2 pos = Vector2Add(position, moveVec);
    Vector2 hitboxOffset = vec2(hitboxSize * dy / WallGetLength(*w), -hitboxSize * dx / WallGetLength(*w));
    if (
            (mult * ((pos.x - w->a.x - hitboxOffset.x) * (w->b.y - w->a.y) - (pos.y - w->a.y - hitboxOffset.y) * (w->b.x - w->a.x)) <= 0) &&
            (mult * ((pos.x - w->a.x - hitboxOffset.x) * hitboxOffset.y - (pos.y - w->a.y - hitboxOffset.y) * hitboxOffset.x) <= 0) &&
            (mult * ((pos.y - w->b.y - hitboxOffset.y) * hitboxOffset.x - (pos.x - w->b.x - hitboxOffset.x) * hitboxOffset.y) <= 0)
            ) {
        double dydx = dy / (dx ? dx : 1);
        double dxdy = dx / (dy ? dy : 1);
        double wallLength = WallGetLength(*w);
        moveVec.x = hitboxSize * dy / wallLength + (dx == 0 ? w->a.x : dy == 0 ? pos.x : (pos.y - w->a.y + w->a.x * dydx + pos.x * dxdy) / (dydx + dxdy)) - position.x;
        moveVec.y = -hitboxSize * dx / wallLength + (dx == 0 ? pos.y : dy == 0 ? w->a.y : (pos.x - w->a.x + w->a.y * dxdy + pos.y * dydx) / (dxdy + dydx)) - position.y;
    }
    return moveVec;
}

Vector2 Move(Vector2 position, Vector2 moveVec) {
    Level *l = GetState()->level;
    for (int i = 0; i < l->walls->size; i++) {
        Wall *w = ListGet(l->walls, i);
        moveVec = CollideWall(w, position, moveVec);
    }
    for (int i = 0; i < l->actors->size; i++) {
        Actor *a = ListGet(l->actors, i);
        if (a->solid) {
            Wall aWall = GetTransformedWall(a);
            moveVec = CollideWall(&aWall, position, moveVec);
        }
    }
    position = Vector2Add(position, moveVec);
    return position;
}
