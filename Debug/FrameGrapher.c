//
// Created by droc101 on 4/24/24.
//

#include "FrameGrapher.h"
#include <stdio.h>
#include "../Helpers/Drawing.h"
#include "SDL.h"
#include "../Structs/GlobalState.h"
#include "../Helpers/Font.h"

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
    if (!FRAMEGRAPH_ENABLE) { return; }
    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);
    int x = 10;
    uint color;
    for (int i = 0; i < FRAMEGRAPH_HISTORY_SIZE; i++) {
        color = 0x4000ff00;
        int height = framerates[i]*2;

        if (height > (TARGET_FPS*2)*2) {
            height = (TARGET_FPS*2)*2;
        }

        if (framerates[i] < FRAMEGRAPH_THRESHOLD_BAD) {
            color = 0x40ff0000;
        } else if (framerates[i] < FRAMEGRAPH_THRESHOLD_GOOD) {
            color = 0x40ff8000;
        }
        setColorUint(color);
        int y = WindowHeight() - height - 10;
        draw_rect(x+(i*2), y, 2, height);
    }

    // draw a line at the target frame time
    setColorUint(0x80808080);
    int y = WindowHeight() - (TARGET_FPS*2) - 10;
    draw_rect(x, y, FRAMEGRAPH_HISTORY_SIZE * 2, 2);
    FontDrawString(vec2(10, y - 5), "Target FPS", 12, 0xff00ffff);

    // draw a line at the bottom
    setColorUint(0x80808080);
    draw_rect(x, WindowHeight() - 10, FRAMEGRAPH_HISTORY_SIZE * 2, 2);

    setColorUint(color);
    char fps[20];
    sprintf(fps, "FPS: %.0f", framerates[FRAMEGRAPH_HISTORY_SIZE - 1]);
    FontDrawString(vec2(10, WindowHeight() - 32), fps, 16, color);
}
