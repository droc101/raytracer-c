//
// Created by droc101 on 4/22/2024.
//

#include "TestActor.h"
#include "../defines.h"
#include "../Structs/wall.h"
#include "../Structs/Vector2.h"

void TestActorInit(Actor *this) {
    this->solid = true;
    this->actorWall = *CreateWall(vec2(-0.5, 0), vec2(0.5, 0), 3);
}

void TestActorUpdate(Actor *this) {
    this->rotation += 0.01;
}

void TestActorDestroy(Actor *this) {
    FreeWall(this->actorWall);
}
