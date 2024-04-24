//
// Created by droc101 on 4/24/24.
//

#ifndef GAME_FRAMEGRAPHER_H
#define GAME_FRAMEGRAPHER_H

#define FRAMEGRAPH_HISTORY_SIZE 120
#define FRAMEGRAPH_INTERVAL 30

#define FRAMEGRAPH_THRESHOLD_GOOD 60
#define FRAMEGRAPH_THRESHOLD_BAD 30

void FrameGraphUpdate(int ms);
void FrameGraphDraw();

#endif //GAME_FRAMEGRAPHER_H
