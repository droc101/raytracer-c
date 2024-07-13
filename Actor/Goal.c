//
// Created by droc101 on 7/12/2024.
//

#include "Goal.h"
#include "../Structs/wall.h"
#include "../Structs/Vector2.h"
#include "../Helpers/Collision.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Helpers/TextBox.h"

void GoalInit(Actor *this) {
    this->solid = false;
    this->actorWall = CreateWall(vec2(0, -0.5), vec2(0, 0.5), 12, 1, 0.0);
}

void GoalUpdate(Actor *this) {
    Vector2 dir = Vector2Sub(GetState()->level->position, this->position);
    this->rotation = atan2(dir.y, dir.x);
    this->rotation += PI;

    if (CollideActorCylinder(this, GetState()->level->position)) {
        RemoveActor(this);
        TextBox tb = DEFINE_TEXT("Goal!", 2, 20, 0, 70, TEXT_BOX_H_ALIGN_CENTER, TEXT_BOX_V_ALIGN_TOP, TEXT_BOX_THEME_WHITE);
        ShowTextBox(tb);
    }
}

void GoalDestroy(Actor *this) {
    FreeWall(this->actorWall);
    free(this->actorWall);
}
