//
// Created by droc101 on 10/7/2024.
//

#ifndef GAME_SLIDER_H
#define GAME_SLIDER_H

#include "../UiStack.h"
#include "../../../Structs/Vector2.h"

typedef struct {
    char *label;

    double min;
    double max;
    double value;

    double step;
    double altStep; // Step when holding shift

    void (*callback)(double);
} SliderData;

Control *CreateSliderControl(Vector2 position, Vector2 size, char *label, void (*callback)(double), ControlAnchor anchor, double min, double max, double value, double step, double altStep);

void DestroySlider(Control *c);

void UpdateSlider(UiStack *stack, Control *c, Vector2 localMousePos, uint ctlIndex);

void DrawSlider(Control *c, ControlState state, Vector2 position);

#endif //GAME_SLIDER_H
