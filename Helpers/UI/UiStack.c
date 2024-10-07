//
// Created by droc101 on 10/7/2024.
//

#include "UiStack.h"
#include "../Input.h"
#include "../../Structs/Vector2.h"

#include "Controls/Button.h"
#include "Controls/Slider.h"

void (*ControlDestroyFuncs[4])(Control *) = {
        DestroyButton, // BUTTON
        DestroySlider, // SLIDER
};

void (*ControlDrawFuncs[4])(Control *, ControlState state, Vector2 position) = {
        DrawButton, // BUTTON
        DrawSlider, // SLIDER
};

void (*ControlUpdateFuncs[4])(UiStack *stack, Control *, Vector2 localMousePos, uint ctlIndex) = {
        UpdateButton, // BUTTON
        UpdateSlider, // SLIDER
};

UiStack *CreateUiStack() {
    UiStack *stack = (UiStack *) malloc(sizeof(UiStack));
    stack->Controls = CreateList();
    stack->ActiveControl = -1;
    stack->ActiveControlState = NORMAL;
    return stack;
}

void DestroyUiStack(UiStack *stack) {
    for (int i = 0; i < stack->Controls->size; i++) {
        Control *c = (Control *) ListGet(stack->Controls, i);
        ControlDestroyFuncs[c->type](c);
    }
    ListFreeWithData(stack->Controls);
    free(stack);
}

void ProcessUiStack(UiStack *stack) {
    stack->ActiveControl = -1;
    stack->ActiveControlState = NORMAL;

    Vector2 mousePos = GetMousePos();

    // iterate through the controls in reverse order so that the last control is on top and gets priority
    for (int i = stack->Controls->size - 1; i >= 0; i--) {
        Control *c = (Control *) ListGet(stack->Controls, i);

        c->anchoredPosition = CalculateControlPosition(c);

        Vector2 localMousePos = v2(mousePos.x - c->anchoredPosition.x, mousePos.y - c->anchoredPosition.y);
        if (localMousePos.x >= 0 && localMousePos.x <= c->size.x && localMousePos.y >= 0 && localMousePos.y <= c->size.y) {
            stack->ActiveControl = i;
            if (IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
                stack->ActiveControlState = ACTIVE;
            } else {
                stack->ActiveControlState = HOVER;
            }
            break;
        }
    }

    if (stack->ActiveControl != -1) {
        Control *c = (Control *) ListGet(stack->Controls, stack->ActiveControl);
        ControlUpdateFuncs[c->type](stack, c, v2(mousePos.x - c->position.x, mousePos.y - c->position.y), stack->ActiveControl);
    }
}

void DrawUiStack(UiStack *stack) {
    for (int i = 0; i < stack->Controls->size; i++) {
        Control *c = (Control *) ListGet(stack->Controls, i);
        ControlDrawFuncs[c->type](c, i == stack->ActiveControl ? stack->ActiveControlState : NORMAL, c->anchoredPosition);
    }
}

Vector2 CalculateControlPosition(Control *control) {
    Vector2 pos = control->position;
    ControlAnchor anchor = control->anchor;

    switch (anchor) {
        case TOP_LEFT:
            break;
        case TOP_CENTER:
            pos.x = (WindowWidth() - control->size.x) / 2;
            break;
        case TOP_RIGHT:
            pos.x = WindowWidth() - control->size.x;
            break;
        case MIDDLE_LEFT:
            pos.y = (WindowHeight() - control->size.y) / 2;
            break;
        case MIDDLE_CENTER:
            pos = v2((WindowWidth() - control->size.x) / 2, (WindowHeight() - control->size.y) / 2);
            break;
        case MIDDLE_RIGHT:
            pos = v2(WindowWidth() - control->size.x, (WindowHeight() - control->size.y) / 2);
            break;
        case BOTTOM_LEFT:
            pos.y = WindowHeight() - control->size.y;
            break;
        case BOTTOM_CENTER:
            pos = v2((WindowWidth() - control->size.x) / 2, WindowHeight() - control->size.y);
            break;
        case BOTTOM_RIGHT:
            pos = v2(WindowWidth() - control->size.x, WindowHeight() - control->size.y);
            break;
    }

    pos = Vector2Add(pos, control->position);

    return pos;
}

Control *CreateEmptyControl() {
    Control *c = (Control *) malloc(sizeof(Control));
    c->ControlData = NULL;
    return c;
}

void UiStackPush(UiStack *stack, Control *control) {
    ListAdd(stack->Controls, control);
}
