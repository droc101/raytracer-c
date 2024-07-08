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
#include "../helpers/Vulkan.h"

void GMainStateUpdate() {
    return;
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

    Level *l = GetState()->level;

    Vector2 moveVec = vec2(0, 0);
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

    bool isMoving = moveVec.x != 0 || moveVec.y != 0;

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

    // view bobbing (scam edition) 💀
    if (spd == SLOW_MOVE_SPEED) {
        if (isMoving) {
            GetState()->FakeHeight = sin(GetState()->frame / 7.0) * 10;
        } else {
            GetState()->FakeHeight = lerp(GetState()->FakeHeight, 0, 0.1); // NOLINT(*-narrowing-conversions)
        }
    } else {
        if (isMoving) {
            GetState()->FakeHeight = sin(GetState()->frame / 7.0) * 40;
        } else {
            GetState()->FakeHeight = lerp(GetState()->FakeHeight, 0, 0.1); // NOLINT(*-narrowing-conversions)
        }
    }

#ifdef KEYBOARD_ROTATION
    if (IsKeyPressed(SDL_SCANCODE_A)) {
        l->rotation -= ROT_SPEED;
    } else if (IsKeyPressed(SDL_SCANCODE_D)) {
        l->rotation += ROT_SPEED;
    }
#else
    l->rotation += ((double)GetMouseRel().x) / MOUSE_SENSITIVITY;
#endif

    if (IsKeyJustPressed(SDL_SCANCODE_C)) {
        Error("Manually triggered error.");
    }

    l->rotation = wrap(l->rotation, 0, 2 * PI);

    for (int i = 0; i < l->staticActors->size; i++) {
        Actor *a = SizedArrayGet(l->staticActors, i);
        a->Update(a);
    }
}

void GMainStateRender() {

//    GlobalState *state = GetState();
//    Level *l = state->level;

//    RenderLevel(l->position, l->rotation, state->FakeHeight);
    DrawFrame();

//    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_NONE);
//    DPrintF("Position: (%.2f, %.2f)\nRotation: %.4f (%.2fdeg)", 0xFFFFFFFF, false, l->position.x, l->position.y, l->rotation, radToDeg(l->rotation));
//
//    DPrintF("Walls: %d", 0xFFFFFFFF, false, l->staticWalls->size);
//    DPrintF("Actors: %d", 0xFFFFFFFF, false, l->staticActors->size);
}

void GMainStateSet() {
    SetRenderCallback(GMainStateRender);
    SetUpdateCallback(GMainStateUpdate);
}

