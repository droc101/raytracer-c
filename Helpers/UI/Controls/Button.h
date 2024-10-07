//
// Created by droc101 on 10/7/2024.
//

#ifndef GAME_BUTTON_H
#define GAME_BUTTON_H

#include "../UiStack.h"
#include "../../../Structs/Vector2.h"

typedef struct {
    char *text;

    void (*callback)();

    bool enabled;
} ButtonData;

Control *CreateButtonControl(Vector2 position, Vector2 size, char *text, void (*callback)(), ControlAnchor anchor);

void DestroyButton(Control *c);

void UpdateButton(UiStack *stack, Control *c, Vector2 localMousePos, uint ctlIndex);

void DrawButton(Control *c, ControlState state, Vector2 position);

#endif //GAME_BUTTON_H
