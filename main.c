#include <SDL.h>
#include <stdio.h>
#include "Helpers/drawing.h"
#include "defines.h"
#include "input.h"
#include "Helpers/font.h"
#include "assets/assets.h"
#include "Structs/level.h"
#include "Structs/ray.h"
#include "Helpers/mathex.h"
#include "error.h"
#include "Helpers/LevelLoader.h"

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SInit Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WIDTH, HEIGHT, 0);
    if (window == NULL) {
        printf("SCreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *tr = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (tr == NULL) {
        SDL_DestroyWindow(window);
        printf("SCreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SetRenderer(tr);

    FontInit();

    byte levelData[] = { 0x00, 0xc0, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf9, 0x21, 0xfb, 0x55, 0x20, 0x6d, 0xdf, 0x03, 0xff, 0xeb, 0x40, 0x34, 0xff, 0x33, 0x28, 0x00, 0x04};

    Level l = LoadLevel(levelData);

    SDL_Event e;
    bool quit = false;
    ulong frameStart, frameTime;

    ulong frameCount = 0;

    while (!quit) {
        frameStart = SDL_GetTicks64();

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYUP) {
                SDL_Scancode scancode = e.key.keysym.scancode;
                HandleKeyUp(scancode);
            } else if (e.type == SDL_KEYDOWN) {
                SDL_Scancode scancode = e.key.keysym.scancode;
                HandleKeyDown(scancode);
            }
        }

        Vector2 oldPos = l.position;
        Vector2 moveVec = vec2(0, 0);
        if (IsKeyPressed(SDL_SCANCODE_W)) {
            moveVec.x += MOVE_SPEED;
        } else if (IsKeyPressed(SDL_SCANCODE_S)) {
            moveVec.x -= MOVE_SPEED;
        }

        if (IsKeyPressed(SDL_SCANCODE_Q)) {
            moveVec.y -= MOVE_SPEED;
        } else if (IsKeyPressed(SDL_SCANCODE_E)) {
            moveVec.y += MOVE_SPEED;
        }

        if (moveVec.x != 0 || moveVec.y != 0) {
            moveVec = Vector2Normalize(moveVec);
        }
        moveVec = Vector2Scale(moveVec, MOVE_SPEED);
        moveVec = Vector2Rotated(moveVec, l.rotation);
        moveVec = Vector2Add(l.position, moveVec);

        double angle = atan2(moveVec.y - oldPos.y, moveVec.x - oldPos.x);

        RayCastResult moveCheck = HitscanLevel(l, oldPos, angle);
        if (moveCheck.Collided) {
            double distance = fabs(Vector2Distance(oldPos, moveCheck.CollisonPoint));
            if (distance <= WALL_HITBOX_EXTENTS) {
                // push 0.5 units out of the wall
                l.position = PushPointOutOfWallHitbox(moveCheck.CollisionWall, moveCheck.CollisonPoint);
            } else {
                l.position = moveVec; // not close enough to the wall to collide
            }
        } else {
            l.position = moveVec; // no collision, move freely
        }

        if (IsKeyPressed(SDL_SCANCODE_A)) {
            l.rotation -= ROT_SPEED;
        } else if (IsKeyPressed(SDL_SCANCODE_D)) {
            l.rotation += ROT_SPEED;
        }

        if (IsKeyJustPressed(SDL_SCANCODE_C)) {
            Error("Manually triggered error.");
        }

        l.rotation = wrap(l.rotation, 0, 2*PI);

        // Frame draw begin

        setColorUint(l.SkyColor);
        SDL_RenderClear(GetRenderer());

        setColorUint(l.FloorColor);
        draw_rect(0, HEIGHT/2, WIDTH, HEIGHT/2);


        for (int col = 0; col < WIDTH; col++) {
            RenderCol(l, col);
        }

        char buffer[64];
        sprintf(buffer, "Position %.2f, %.2f\nRotation %.4f", l.position.x, l.position.y, l.rotation);
        FontDrawString(vec2(20, 20), buffer, 16);

        // Frame draw end

        SDL_RenderPresent(GetRenderer());

        UpdateKeyStates();
        frameCount++;

        frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < TARGET_MS) {
            SDL_Delay(TARGET_MS - frameTime);
        }

        // Calculate and print FPS
        float fps = 1000.0 / (SDL_GetTicks64() - frameStart);
        sprintf(buffer, "GAME! - FPS: %.2f Time: %lu ms\n", fps, frameTime);
        SDL_SetWindowTitle(window, buffer);

    }

    DestroyLevel(l);

    SDL_DestroyRenderer(GetRenderer());
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}