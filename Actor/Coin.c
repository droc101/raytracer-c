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
#include "../Helpers/CommonAssets.h"

void CoinInit(Actor *this) {
    this->solid = false;
    this->actorWall = CreateWall(v2(0, -0.125), v2(0, 0.125), (this->paramB == 1) ? actorTextures[8] : actorTextures[7], 1.0, 0.0);
    this->paramA = 0;
    this->actorWall->height = 0.25f;
    this->yPosition = -0.25f;
    this->shadowSize = 0.1;
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

    if (CollideCylinder(this->position, 0.25, GetState()->level->position)) {
        if (this->paramB == 0) {
            GetState()->coins++;
        } else {
            GetState()->blueCoins++;
            GetState()->coins += 5;

        }
        PlaySoundEffect(gzwav_sfx_coincling);
        RemoveActor(this);
    }
}

void CoinDestroy(Actor *this) {
    FreeWall(this->actorWall);
    free(this->actorWall);
}
