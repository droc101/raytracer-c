//
// Created by droc101 on 4/24/24.
//

#include "FrameGrapher.h"
#include <stdio.h>
#include "SDL.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Structs/GlobalState.h"

double framerates[FRAMEGRAPH_HISTORY_SIZE];

void FG_PushIntoArray(const double value)
{
    for (int i = 0; i < FRAMEGRAPH_HISTORY_SIZE - 1; i++)
    {
        framerates[i] = framerates[i + 1];
    }
    framerates[FRAMEGRAPH_HISTORY_SIZE - 1] = value;
}

void FrameGraphUpdate(ulong ns)
{
    if (GetState()->physicsFrame % FRAMEGRAPH_INTERVAL == 0)
    {
        if (ns == 0) { ns = 1; }
        FG_PushIntoArray(1000000000.0 / ns);
    }
}

void FrameGraphDraw()
{
#ifdef FRAMEGRAPH_ENABLE
#ifndef FRAMEGRAPH_FPS_ONLY
    int x = 10;
    uint color;
    for (int i = 0; i < FRAMEGRAPH_HISTORY_SIZE; i++) {
        color = 0x4000ff00;
        int height = framerates[i]*FRAMEGRAPH_V_SCALE;

#ifdef FRAMEGRAPH_ENABLE_CAPPING
        if (height > (FRAMEGRAPH_THRESHOLD_GOOD*2)*FRAMEGRAPH_V_SCALE) {
            height = (FRAMEGRAPH_THRESHOLD_GOOD*2)*FRAMEGRAPH_V_SCALE;
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

    // draw a line at the target physicsFrame time
    setColorUint(0x80808080);
    int y = WindowHeight() - (FRAMEGRAPH_THRESHOLD_GOOD*FRAMEGRAPH_V_SCALE) - 10;
    draw_rect(x, y, FRAMEGRAPH_HISTORY_SIZE * 2, 2);
    FontDrawString(v2(10, y - 5), "Target FPS", 12, 0xff00ffff, true);

    // draw a line at the bottom
    setColorUint(0x80808080);
    draw_rect(x, WindowHeight() - 10, FRAMEGRAPH_HISTORY_SIZE * 2, 2);
#else
    uint color = 0x4000ff00;
#endif
    // set the alpha to 255
    color |= 0xff000000;
    SetColorUint(color);
    char fps[20];
    sprintf(fps, "FPS: %.2f", framerates[FRAMEGRAPH_HISTORY_SIZE - 1]);
    FontDrawString(v2(10, WindowHeight() - 32), fps, 16, color, true);
#endif
}
