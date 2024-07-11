//
// Created by droc101 on 4/22/2024.
//

#include "TestActor.h"
#include "../Structs/Wall.h"
#include "../Structs/Vector2.h"
#include "../Helpers/Collision.h"

void TestActorInit(Actor *this) {
    this->solid = true;
    this->actorWall = CreateWall(vec2(-0.5, 0), vec2(0.5, 0), 3, 1.0, 0.0);
}

void TestActorUpdate(Actor *this) {
    this->rotation += 0.01;

    Vector2 MoveDir = vec2(0, 0.05);
    MoveDir = Vector2Rotate(MoveDir, this->rotation);

    MoveDir = Move(this->position, MoveDir, this);

    this->position = MoveDir;
}

void TestActorDestroy(Actor *this) {
    FreeWall(this->actorWall);
    free(this->actorWall);
}
