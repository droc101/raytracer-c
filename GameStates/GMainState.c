//
// Created by droc101 on 4/22/2024.
//

#include "GMainState.h"
#include <math.h>
#include <stdio.h>
#include "GEditorState.h"
#include "GPauseState.h"
#include "../Assets/Assets.h"
#include "../Debug/DPrint.h"
#include "../Helpers/Collision.h"
#include "../Helpers/TextBox.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"

void GMainStateUpdate(GlobalState *State)
{
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_START))
    {
        GPauseStateSet();
        return;
    }
#ifdef ENABLE_LEVEL_EDITOR
    if (IsKeyJustPressed(SDL_SCANCODE_F6))
    {
        GEditorStateSet();
        return;
    }
#endif

    if (State->textBoxActive)
    {
        if (IsKeyJustPressed(SDL_SCANCODE_SPACE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_A))
        {
            State->textBoxPage++;
            if (State->textBoxPage >= StringLineCount(State->textBox.text) / State->textBox.rows)
            {
                State->textBoxActive = false;
            }
        }
        return;
    }

    if (IsKeyJustPressed(SDL_SCANCODE_C))
    {
        Error("Manually triggered error.");
    }

    if (IsKeyJustPressed(SDL_SCANCODE_T))
    {
        const TextBox tb = DEFINE_TEXT("TEXT BOX", 2, 20, 0, 60, TEXT_BOX_H_ALIGN_CENTER, TEXT_BOX_V_ALIGN_TOP,
                                       TEXT_BOX_THEME_BLACK);
        ShowTextBox(tb);
    }

    State->level->rotation += GetMouseRel().x * (State->options.mouseSpeed / 120.0);
}

uint GMainStateFixedUpdate(const uint interval, GlobalState *State)
{
    if (State->textBoxActive)
    {
        return interval;
    }

    Level *l = State->level;
    Vector2 moveVec = v2(0, 0);
    if (UseController())
    {
        moveVec.y = GetAxis(SDL_CONTROLLER_AXIS_LEFTX);
        moveVec.x = -GetAxis(SDL_CONTROLLER_AXIS_LEFTY);
        if (fabs(moveVec.x) < 0.1)
        {
            moveVec.x = 0;
        }
        if (fabs(moveVec.y) < 0.1)
        {
            moveVec.y = 0;
        }

    } else
    {
        if (IsKeyPressed(SDL_SCANCODE_W) || GetAxis(SDL_CONTROLLER_AXIS_LEFTY) < -0.5)
        {
            moveVec.x += 1;
        } else if (IsKeyPressed(SDL_SCANCODE_S) || GetAxis(SDL_CONTROLLER_AXIS_LEFTY) > 0.5)
        {
            moveVec.x -= 1;
        }

        if (IsKeyPressed(SDL_SCANCODE_A) || GetAxis(SDL_CONTROLLER_AXIS_LEFTX) < -0.5)
        {
            moveVec.y -= 1;
        } else if (IsKeyPressed(SDL_SCANCODE_D) || GetAxis(SDL_CONTROLLER_AXIS_LEFTX) > 0.5)
        {
            moveVec.y += 1;
        }
    }



    const bool isMoving = moveVec.x != 0 || moveVec.y != 0;

    if (isMoving && !UseController())
    {
        moveVec = Vector2Normalize(moveVec);
    }

    double spd = MOVE_SPEED;
    if (IsKeyPressed(SDL_SCANCODE_LSHIFT) || GetAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0.5)
    {
        spd = SLOW_MOVE_SPEED;
    }

    moveVec = Vector2Scale(moveVec, spd);
    moveVec = Vector2Rotate(moveVec, l->rotation);

    l->position = Move(l->position, moveVec, NULL);

    if (UseController())
    {
        const double cx = GetAxis(SDL_CONTROLLER_AXIS_RIGHTX);
        if (fabs(cx) > 0.1)
        {
            l->rotation += cx * (State->options.mouseSpeed / 11.25);
        }
    }

    // view bobbing (scam edition) 💀 (it's better now trust me)
    if (spd == SLOW_MOVE_SPEED)
    {
        if (isMoving)
        {
            State->CameraY = (sin(State->physicsFrame / 7.0) * 0.005) - 0.1; // NOLINT(*-narrowing-conversions)
        } else
        {
            State->CameraY = lerp(State->CameraY, -0.1, 0.1); // NOLINT(*-narrowing-conversions)
        }
    } else
    {
        if (isMoving)
        {
            State->CameraY = sin(State->physicsFrame / 7.0) * 0.04; // NOLINT(*-narrowing-conversions)
        } else
        {
            State->CameraY = lerp(State->CameraY, 0, 0.1); // NOLINT(*-narrowing-conversions)
        }
    }

    l->rotation = wrap(l->rotation, 0, 2 * PI);

    for (int i = 0; i < l->staticActors->size; i++)
    {
        Actor *a = SizedArrayGet(l->staticActors, i);
        a->Update(a);
    }

    State->physicsFrame++;
    return interval;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GMainStateRender(GlobalState *State)
{
    const Level *l = State->level;

    RenderLevel(State);

    SDL_Rect coinIconRect = {WindowWidth() - 260, 16, 40, 40};
    DrawTexture(v2(WindowWidth() - 260, 16), v2(40, 40), gztex_interface_hud_ycoin);

    char coinStr[16];
    sprintf(coinStr, "%d", State->coins);
    FontDrawString(v2(WindowWidth() - 210, 16), coinStr, 40, 0xFFFFFFFF, false);

    coinIconRect.y = 64;

    for (int bc = 0; bc < State->blueCoins; bc++)
    {
        coinIconRect.x = WindowWidth() - 260 + bc * 48;
        DrawTexture(v2(coinIconRect.x, coinIconRect.y), v2(40, 40), gztex_interface_hud_bcoin);
    }

    if (State->textBoxActive)
    {
        TextBoxRender(&State->textBox, State->textBoxPage);
    }
    DPrintF("Position: (%.2f, %.2f)\nRotation: %.4f (%.2fdeg)", 0xFFFFFFFF, false, l->position.x, l->position.y,
            l->rotation, radToDeg(l->rotation));

    DPrintF("Walls: %d", 0xFFFFFFFF, false, l->staticWalls->size);
    DPrintF("Actors: %d", 0xFFFFFFFF, false, l->staticActors->size);
}

void GMainStateSet()
{
    SetRenderCallback(GMainStateRender);
    SetUpdateCallback(GMainStateUpdate, GMainStateFixedUpdate, MAIN_STATE);
}
