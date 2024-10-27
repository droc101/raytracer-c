//
// Created by droc101 on 10/27/2024.
//

#ifndef GAME_RADIOBUTTON_H
#define GAME_RADIOBUTTON_H

#include "../UiStack.h"
#include "../../Vector2.h"

typedef struct RadioButtonData {
    char *label;
    byte groupId;
    byte id;

    bool checked;

    void (*callback)(bool, byte, byte);
} RadioButtonData;

Control *CreateRadioButtonControl(Vector2 position, Vector2 size, char *label, void (*callback)(bool, byte, byte), ControlAnchor anchor, bool checked, byte groupId, byte id);

void DestroyRadioButton(Control *c);

void UpdateRadioButton(UiStack *stack, Control *c, Vector2 localMousePos, uint ctlIndex);

void DrawRadioButton(Control *c, ControlState state, Vector2 position);

#endif //GAME_RADIOBUTTON_H
