//
// Created by droc101 on 10/7/2024.
//

#include "Slider.h"
#include "../../Input.h"
#include "../../Font.h"
#include "../../Drawing.h"
#include "../../MathEx.h"
#include <math.h>
#include <stdio.h>

Control *CreateSliderControl(Vector2 position, Vector2 size, char *label, void (*callback)(double), ControlAnchor anchor, double min, double max, double value, double step, double altStep) {
    Control *slider = CreateEmptyControl();
    slider->type = SLIDER;
    slider->position = position;
    slider->size = size;
    slider->anchor = anchor;

    slider->ControlData = malloc(sizeof(SliderData));
    SliderData *data = (SliderData *) slider->ControlData;
    data->label = label;
    data->callback = callback;
    data->min = min;
    data->max = max;
    data->value = value;
    data->step = step;
    data->altStep = altStep;

    return slider;
}

void DestroySlider(Control *c) {
    SliderData *data = (SliderData *) c->ControlData;
    free(data);
}

void UpdateSlider(UiStack *stack, Control *c, Vector2 localMousePos, uint ctlIndex) {
    SliderData *data = (SliderData *) c->ControlData;

    // handle l and r arrow keys
    if (stack->focusedControl == ctlIndex) {
        if (IsKeyJustPressed(SDL_SCANCODE_LEFT)) {
            data->value -= data->step;
            if (data->value < data->min) {
                data->value = data->min;
            }
            if (data->callback != NULL) {
                data->callback(data->value);
            }
        } else if (IsKeyJustPressed(SDL_SCANCODE_RIGHT)) {
            data->value += data->step;
            if (data->value > data->max) {
                data->value = data->max;
            }
            if (data->callback != NULL) {
                data->callback(data->value);
            }
        }
    }

    bool pressed = IsMouseButtonPressed(SDL_BUTTON_LEFT);

    if (pressed) {
        double newVal = remap(GetMousePos().x - c->anchoredPosition.x, 0.0, c->size.x, data->min, data->max);
        data->value = newVal;

        // snap to step
        double step = data->step;
        if (IsKeyPressed(SDL_SCANCODE_LSHIFT) || IsKeyPressed(SDL_SCANCODE_RSHIFT)) {
            step = data->altStep;
        }

       data->value = round(data->value / step) * step;

        if (data->value < data->min) {
            data->value = data->min;
        } else if (data->value > data->max) {
            data->value = data->max;
        }

        if (data->callback != NULL) {
            data->callback(data->value);
        }
    }
}

void DrawSlider(Control *c, ControlState state, Vector2 position) {
    uint color = 0xff252525;
    setColorUint(color);
    draw_rect(position.x, position.y, c->size.x, c->size.y);
    setColorUint(0xFFFFFFFF);
    DrawOutlineRect(position, c->size);

    SliderData *data = (SliderData *) c->ControlData;
    double handlePos = remap(data->value, data->min, data->max, 0, c->size.x - 10);

    // draw handle
    switch (state) {
        case NORMAL:
            color = 0xFFc2e3ff;
            break;
        case HOVER:
            color = 0xFFa1d4ff;
            break;
        case ACTIVE:
            color = 0xFF8ac9ff;
            break;
    }
    setColorUint(color);
    draw_rect(position.x + handlePos, position.y, 10, c->size.y);

    char buf[64];
    sprintf(buf, "%s: %.2f", data->label, data->value);
    DrawTextAligned(buf, 16, 0xFFFFFFFF, position, c->size, FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
}
