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

void FrameGraphUpdate(unsigned long ns) {
    if (GetState()->frame % FRAMEGRAPH_INTERVAL == 0) {
        if (ns == 0) { ns = 1; }
        double fps = 1000000000.0 / ns;
        FG_PushIntoArray(fps);
    }
}

void FrameGraphDraw() {
#ifdef FRAMEGRAPH_ENABLE
    int x = 10;
    uint color;
    for (int i = 0; i < FRAMEGRAPH_HISTORY_SIZE; i++) {
        color = 0x4000ff00;
        int height = framerates[i]*FRAMEGRAPH_V_SCALE;

#ifdef FRAMEGRAPH_ENABLE_CAPPING
        if (height > (TARGET_FPS*2)*FRAMEGRAPH_V_SCALE) {
            height = (TARGET_FPS*2)*FRAMEGRAPH_V_SCALE;
        }
#endif

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
    int y = WindowHeight() - (TARGET_FPS*FRAMEGRAPH_V_SCALE) - 10;
    draw_rect(x, y, FRAMEGRAPH_HISTORY_SIZE * 2, 2);
    FontDrawString(v2(10, y - 5), "Target FPS", 12, 0xff00ffff, true);

    // draw a line at the bottom
    setColorUint(0x80808080);
    draw_rect(x, WindowHeight() - 10, FRAMEGRAPH_HISTORY_SIZE * 2, 2);

    // set the alpha to 255
    color |= 0xff000000;
    setColorUint(color);
    char fps[20];
    sprintf(fps, "FPS: %.2f", framerates[FRAMEGRAPH_HISTORY_SIZE - 1]);
    FontDrawString(v2(10, WindowHeight() - 32), fps, 16, color, true);
#endif
}
