//
// Created by droc101 on 4/24/24.
//

#include "FrameGrapher.h"
#include <stdio.h>
#include "../Helpers/drawing.h"
#include "SDL.h"
#include "../Structs/GlobalState.h"
#include "../Helpers/font.h"

double framerates[FRAMEGRAPH_HISTORY_SIZE];

void FG_PushIntoArray(double value) {
    for (int i = 0; i < FRAMEGRAPH_HISTORY_SIZE - 1; i++) {
        framerates[i] = framerates[i+1];
    }
    framerates[FRAMEGRAPH_HISTORY_SIZE-1] = value;
}

void FrameGraphUpdate(int ms) {
    if (GetState()->frame % FRAMEGRAPH_INTERVAL == 0) {
        if (ms == 0) { ms = 1; }
        double fps = 1000 / ms;
        FG_PushIntoArray(fps);
    }
}

void FrameGraphDraw() {
    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);
    int x = 10;
    for (int i = 0; i < FRAMEGRAPH_HISTORY_SIZE; i++) {
        int height = framerates[i] / 2;
        uint color = 0x8000ff00;
        if (framerates[i] < FRAMEGRAPH_THRESHOLD_BAD) {
            color = 0x80ff0000;
        } else if (framerates[i] < FRAMEGRAPH_THRESHOLD_GOOD) {
            color = 0x80ff8000;
        }
        setColorUint(color);
        int y = WindowHeight() - height;
        draw_rect(x+(i*2), y, 2, height);
    }
    char fps[20];
    sprintf(fps, "FPS: %.2f", framerates[FRAMEGRAPH_HISTORY_SIZE - 1]);
    FontDrawString(vec2(10, WindowHeight() - 64), fps, 24);
}
