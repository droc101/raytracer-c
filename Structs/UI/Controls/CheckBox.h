//
// Created by droc101 on 10/27/2024.
//

#ifndef GAME_CHECKBOX_H
#define GAME_CHECKBOX_H

#include "../UiStack.h"
#include "../../Vector2.h"

typedef struct CheckBoxData {
    char *label;

    bool checked;

    void (*callback)(bool);
} CheckBoxData;

Control *CreateCheckboxControl(Vector2 position, Vector2 size, char *label, void (*callback)(bool), ControlAnchor anchor, bool checked);

void DestroyCheckbox(Control *c);

void UpdateCheckbox(UiStack *stack, Control *c, Vector2 localMousePos, uint ctlIndex);

void DrawCheckbox(Control *c, ControlState state, Vector2 position);

#endif //GAME_CHECKBOX_H
