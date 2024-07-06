//
// Created by droc101 on 4/22/2024.
//

#include "GMainState.h"
#include <math.h>
#include "../Helpers/Input.h"
#include "../Helpers/Error.h"
#include "../Helpers/MathEx.h"
#include "../Helpers/Drawing.h"
#include "../Helpers/Collision.h"
#include "../config.h"
#include "GPauseState.h"
#include "GEditorState.h"

SDL_Texture *skyTex;

void GMainStateUpdate() {
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

    //Vector2 oldPos = l->position;
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
    moveVec = Vector2Scale(moveVec, MOVE_SPEED);
    moveVec = Vector2Rotate(moveVec, l->rotation);

    l->position = Move(l->position, moveVec, NULL);

    // view bobbing (scam edition) 💀
    if (isMoving) {
        GetState()->FakeHeight = sin(GetState()->frame / 7.0) * 40;
    } else {
        GetState()->FakeHeight = lerp(GetState()->FakeHeight, 0, 0.1); // NOLINT(*-narrowing-conversions)
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

void GMainStateRender(VkInstance* instance, VkSurfaceKHR* surface) {
    Level *l = GetState()->level;

    byte *sc = getColorUint(l->SkyColor);
    SDL_SetTextureColorMod(skyTex, sc[0], sc[1], sc[2]);
    free(sc);

    int skyPos = (int)(l->rotation * 256 / (2 * PI)) % 256;

//    SDL_Rect src1 = {0, 0, skyPos, 128};
//    SDL_Rect dest1 = {(int)(WindowWidth() * (1 - skyPos / 256.0)), 0, WindowWidth() * skyPos / 256, WindowHeight() / 2};
//    SDL_Rect src2 = {skyPos, 0, 256 - skyPos, 128};
//    SDL_Rect dest2 = {0, 0, (int)(WindowWidth() * (1 - skyPos / 256.0)), WindowHeight() / 2};
//    SDL_RenderCopy(GetRenderer(), skyTex, &src1, &dest1);
//    SDL_RenderCopy(GetRenderer(), skyTex, &src2, &dest2);



//    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);
//    for (int col = 0; col < WindowWidth(); col++) {
//        RenderCol(l, col);
//        RenderActorCol(l, col);
//    }
//    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_NONE);
//    DPrintF("Position: (%.2f, %.2f)\nRotation: %.4f (%.2fdeg)", 0xFFFFFFFF, false, l->position.x, l->position.y, l->rotation, radToDeg(l->rotation));
//
//    DPrintF("Mouse Buttons: %d %d %d", 0xFFFFFFFF, false, IsMouseButtonPressed(1), IsMouseButtonPressed(2), IsMouseButtonPressed(3));
//
//    DPrintF("Walls: %d", 0xFFFFFFFF, false, l->staticWalls->size);
//    DPrintF("Actors: %d", 0xFFFFFFFF, false, l->staticActors->size);
}

void GMainStateSet() {
    SetRenderCallback(GMainStateRender);
    SetUpdateCallback(GMainStateUpdate);
}

void InitSkyTex() {
    skyTex = ToSDLTexture((const unsigned char *) gztex_level_sky, FILTER_LINEAR);
}

