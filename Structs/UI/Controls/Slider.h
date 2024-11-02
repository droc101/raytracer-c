//
// Created by droc101 on 10/7/2024.
//

#ifndef GAME_SLIDER_H
#define GAME_SLIDER_H

#include "../UiStack.h"
#include "../../Vector2.h"

typedef struct SliderData
{
    char *label;

    double min;
    double max;
    double value;

    double step;
    double altStep; // Step when holding shift

    void (*callback)(double);

    char *(*getLabel)(Control *slider);
} SliderData;

char *SliderLabelPercent(const Control *slider);

char *SliderLabelInteger(const Control *slider);

/**
 * Create a new Slider Control
 * @param position The position of the slider
 * @param size The size of the slider
 * @param label The label of the slider
 * @param callback The callback function to call when the slider value changes
 * @param anchor The anchor of the slider
 * @param min The minimum value of the slider
 * @param max The maximum value of the slider
 * @param value The initial value of the slider
 * @param step The step value of the slider
 * @param altStep The step value of the slider when holding shift
 * @param getLabel The function to get the label of the slider
 * @return The new Slider Control
 */
Control *
CreateSliderControl(Vector2 position, Vector2 size, char *label, void (*callback)(double), ControlAnchor anchor,
                    double min, double max, double value, double step, double altStep,
                    char *(*getLabel)(Control *slider));

void DestroySlider(const Control *c);

void UpdateSlider(const UiStack *stack, const Control *c, Vector2 localMousePos, const uint ctlIndex);

void DrawSlider(Control *c, ControlState state, Vector2 position);

#endif //GAME_SLIDER_H
