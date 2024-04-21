#include <SDL.h>
#include <stdio.h>

#include "defines.h"
#include "input.h"
#include "Helpers/drawing.h"
#include "assets/assets.h"
#include "Structs/level.h"
#include "Structs/ray.h"

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SInit Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("surface to draw on with the magic sand",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WIDTH, HEIGHT, 0);
    if (window == NULL) {
        printf("SCreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_DestroyWindow(window);
        printf("SCreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    Level l = CreateLevel();
    l.rotation = PI/2;
    Wall w = CreateWall(vec2(-10, 1), vec2(10, 1), 0);
    ListAdd(l.walls, &w);

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

        setColorUint(renderer, l.SkyColor);
        SDL_RenderClear(renderer);

        setColorUint(renderer, l.FloorColor);
        draw_rect(renderer, 0, HEIGHT/2, WIDTH, HEIGHT/2);
        // Frame loop begin

        for (int col = 0; col < WIDTH; col++) {
            RenderCol(renderer, l, col);
        }

        draw_texture(renderer, tex_actor_BLOB2, frameCount % 0x7f, 10);
        draw_texture(renderer, tex_actor_iq, frameCount % 300, 20);

        // Frame loop end

        SDL_RenderPresent(renderer);

        Vector2 oldPos = l.position;
        if (IsKeyPressed(SDL_SCANCODE_UP)) {
            Vector2 rotAngle = Vector2Rotated(vec2(0.5, 0), l.rotation);
            l.position = Vector2Add(l.position, rotAngle);
        } else if (IsKeyPressed(SDL_SCANCODE_DOWN)) {
            Vector2 rotAngle = Vector2Rotated(vec2(-0.5, 0), l.rotation);
            l.position = Vector2Add(l.position, rotAngle);
        }

        double angle = atan2(l.position.y - oldPos.y, l.position.x - oldPos.x);

        RayCastResult moveCheck = HitscanLevel(l, oldPos, angle);
        if (moveCheck.Collided) {
            double distance = Vector2Distance(oldPos, moveCheck.CollisonPoint);
            if (distance < 0.5) {
                l.position = oldPos;
            }
        }

        if (IsKeyPressed(SDL_SCANCODE_LEFT)) {
            l.rotation -= 0.1;
        } else if (IsKeyPressed(SDL_SCANCODE_RIGHT)) {
            l.rotation += 0.1;
        }

        UpdateKeyStates();
        frameCount++;
        //l.rotation += 0.01;

        frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < TARGET_MS) {
            SDL_Delay(TARGET_MS - frameTime);
        }

        // Calculate and print FPS
        float fps = 1000.0 / (SDL_GetTicks64() - frameStart);
        printf("FPS: %.2f Time: %lu ms\n", fps, frameTime);
        fflush(stdout);

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}