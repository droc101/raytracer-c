//
// Created by droc101 on 4/22/2024.
//

#include "GMainState.h"
#include <math.h>
#include "../Helpers/Input.h"
#include "../Helpers/Error.h"
#include "../Helpers/MathEx.h"
#include "../Helpers/Drawing.h"
#include "../Debug/DPrint.h"
#include "../Helpers/Collision.h"
#include "GPauseState.h"
#include "GEditorState.h"
#include <stdio.h>
#include "../Helpers/Font.h"
#include "../Helpers/TextBox.h"
#include "../Helpers/Timing.h"

void GMainStateUpdate(GlobalState * State) {
    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
        GPauseStateSet();
        return;
    }
#ifdef ENABLE_LEVEL_EDITOR
    if (IsKeyJustPressed(SDL_SCANCODE_F6)) {
        GEditorStateSet();
        return;
    }
#endif

    if (State->textBoxActive) {
        if (IsKeyJustPressed(SDL_SCANCODE_SPACE)) {
            State->textBoxPage++;
            if (State->textBoxPage >= State->textBox.rows) {
                State->textBoxActive = false;
            }
        }
        return;
    }

    if (IsKeyJustPressed(SDL_SCANCODE_C)) {
        Error("Manually triggered error.");
    }

    if (IsKeyJustPressed(SDL_SCANCODE_T)) {
        TextBox tb = DEFINE_TEXT("TEXT BOX\n\nPAGE TWO", 2, 20, 0, 60, TEXT_BOX_H_ALIGN_CENTER, TEXT_BOX_V_ALIGN_TOP, TEXT_BOX_THEME_BLACK);
        ShowTextBox(tb);
    }

#ifdef KEYBOARD_ROTATION
    if (IsKeyPressed(SDL_SCANCODE_A)) {
        State->level->rotation -= ROT_SPEED;
    } else if (IsKeyPressed(SDL_SCANCODE_D)) {
        State->level->rotation += ROT_SPEED;
    }
#else
    State->level->rotation += GetMouseRel().x / MOUSE_SENSITIVITY;
#endif
}

uint GMainStateFixedUpdate(const uint interval, GlobalState *State)
{
    if (State->textBoxActive) {
        return interval;
    }

    Level *l = State->level;
    Vector2 moveVec = v2(0, 0);
    if (IsKeyPressed(SDL_SCANCODE_W)) {
        moveVec.x += 1;
    } else if (IsKeyPressed(SDL_SCANCODE_S)) {
        moveVec.x -= 1;
    }

#ifdef KEYBOARD_ROTATION
    if (IsKeyPressed(SDL_SCANCODE_Q)) {
        moveVec.y -= 1;
    } else if (IsKeyPressed(SDL_SCANCODE_E)) {
        moveVec.y += 1;
    }
#else
    if (IsKeyPressed(SDL_SCANCODE_A)) {
        moveVec.y -= 1;
    } else if (IsKeyPressed(SDL_SCANCODE_D)) {
        moveVec.y += 1;
    }
#endif

    const bool isMoving = moveVec.x != 0 || moveVec.y != 0;

    if (isMoving) {
        moveVec = Vector2Normalize(moveVec);
    }

    double spd = MOVE_SPEED;
    if (IsKeyPressed(SDL_SCANCODE_LSHIFT)) {
        spd = SLOW_MOVE_SPEED;
    }

    moveVec = Vector2Scale(moveVec, spd);
    moveVec = Vector2Rotate(moveVec, l->rotation);

    l->position = Move(l->position, moveVec, NULL);

    // view bobbing (scam edition) 💀 (it's better now trust me)
    if (spd == SLOW_MOVE_SPEED) {
        if (isMoving) {
            State->FakeHeight = sin(State->frame / 7.0) * 0.02; // NOLINT(*-narrowing-conversions)
        } else {
            State->FakeHeight = lerp(State->FakeHeight, 0, 0.1); // NOLINT(*-narrowing-conversions)
        }
    } else {
        if (isMoving) {
            State->FakeHeight = sin(State->frame / 7.0) * 0.04; // NOLINT(*-narrowing-conversions)
        } else {
            State->FakeHeight = lerp(State->FakeHeight, 0, 0.1); // NOLINT(*-narrowing-conversions)
        }
    }

    l->rotation = wrap(l->rotation, 0, 2 * PI);

    for (int i = 0; i < l->staticActors->size; i++) {
        Actor *a = SizedArrayGet(l->staticActors, i);
        a->Update(a);
    }

    State->frame++;
    return interval;
}

void GMainStateRender(GlobalState* State) {
    Level *l = State->level;

    RenderLevel(State);

    SDL_Rect coinIconRect = {WindowWidth() - 260, 16, 40, 40};
    DrawTexture(v2(WindowWidth() - 260, 16), v2(40, 40), gztex_interface_hud_ycoin);

    char coinStr[16];
    sprintf(coinStr, "%d", State->coins);
    FontDrawString(v2(WindowWidth() - 210, 16), coinStr, 40, 0xFFFFFFFF, false);

    coinIconRect.y = 64;

    for (int bc = 0; bc < State->blueCoins; bc++) {
        coinIconRect.x = WindowWidth() - 260 + (bc * 48);
        DrawTexture(v2(coinIconRect.x, coinIconRect.y), v2(40, 40), gztex_interface_hud_bcoin);
    }

    if (State->textBoxActive) {
        TextBoxRender(&(State->textBox), State->textBoxPage);
    }
    DPrintF("Position: (%.2f, %.2f)\nRotation: %.4f (%.2fdeg)", 0xFFFFFFFF, false, l->position.x, l->position.y, l->rotation, radToDeg(l->rotation));

    DPrintF("Walls: %d", 0xFFFFFFFF, false, l->staticWalls->size);
    DPrintF("Actors: %d", 0xFFFFFFFF, false, l->staticActors->size);
}

void GMainStateSet() {
    SetRenderCallback(GMainStateRender);
    SetUpdateCallback(GMainStateUpdate, GMainStateFixedUpdate, MAIN_STATE);
}

