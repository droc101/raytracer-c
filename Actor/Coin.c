//
// Created by droc101 on 7/11/2024.
//

#include "Coin.h"
#include "../Structs/Wall.h"
#include "../Structs/Vector2.h"
#include "../Helpers/Collision.h"
#include "../Structs/GlobalState.h"
#include <math.h>
#include "../Structs/Level.h"

void CoinInit(Actor *this) {
    this->solid = false;
    this->actorWall = CreateWall(vec2(0, -0.5), vec2(0, 0.5), (this->paramB == 1) ? 11 : 10, 0.25, 0.0);
    this->paramA = 0;
}

void CoinUpdate(Actor *this) {

    if (GetState()->frame % 8 == 0) {
        this->paramA++;
        this->paramA = this->paramA % 4;

        double uvo = 0.25 * this->paramA;
        this->actorWall->uvOffset = uvo;
    }

    Vector2 dir = Vector2Sub(GetState()->level->position, this->position);
    this->rotation = atan2(dir.y, dir.x);
    this->rotation += PI;

    if (CollideActorCylinder(this, GetState()->level->position)) {
        if (this->paramB == 0) {
            GetState()->coins++;
        } else {
            GetState()->blueCoins++;
            GetState()->coins += 5;

        }
        RemoveActor(this);
    }
}

void CoinDestroy(Actor *this) {
    FreeWall(this->actorWall);
    free(this->actorWall);
}
