//
// Created by droc101 on 4/24/24.
//

#ifndef GAME_FRAMEGRAPHER_H
#define GAME_FRAMEGRAPHER_H

#include "../defines.h"

// How many frames to store in the graph
#define FRAMEGRAPH_HISTORY_SIZE 120

// How often to update the graph
#define FRAMEGRAPH_INTERVAL 30

// Thresholds for coloring the graph
#define FRAMEGRAPH_THRESHOLD_GOOD 60 // Anything above this is good (green)
#define FRAMEGRAPH_THRESHOLD_BAD 30 // Anything below this is bad (red)
// Anything between these two are orange

// Enable or disable capping the graph at 2x the target FPS
#define FRAMEGRAPH_ENABLE_CAPPING

// How much to scale the graph by (60fps target and 2x scale makes it a nice square)
#define FRAMEGRAPH_V_SCALE 2

// Enable or disable the physicsFrame graph (just the rendering)
#define FRAMEGRAPH_ENABLE

// Disable to draw the graph
// Drawing the graph has a significant performance impact
#define FRAMEGRAPH_FPS_ONLY

void FrameGraphUpdate(ulong ns);

void FrameGraphDraw();

#endif //GAME_FRAMEGRAPHER_H
