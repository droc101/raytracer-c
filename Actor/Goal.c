//
// Created by droc101 on 7/12/2024.
//

#include "Goal.h"
#include <math.h>
#include "../Helpers/Collision.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/TextBox.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Vector2.h"
#include "../Structs/Wall.h"

void GoalInit(Actor *this)
{
    this->solid = false;
    this->actorWall = CreateWall(v2(0, -0.5), v2(0, 0.5), actorTextures[9], 1, 0.0);
}

void GoalUpdate(Actor *this)
{
    const Vector2 dir = Vector2Sub(GetState()->level->player.pos, this->position);
    this->rotation = atan2(dir.y, dir.x);
    this->rotation += PI;

    if (CollideActorCylinder(this, GetState()->level->player.pos))
    {
        RemoveActor(this);
        const TextBox tb = DEFINE_TEXT("Goal!", 2, 20, 0, 70, TEXT_BOX_H_ALIGN_CENTER, TEXT_BOX_V_ALIGN_TOP,
                                 TEXT_BOX_THEME_WHITE);
        ShowTextBox(tb);
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GoalDestroy(Actor *this)
{
    FreeWall(this->actorWall);
    free(this->actorWall);
}
