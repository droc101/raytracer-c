//
// Created by droc101 on 1/7/25.
//

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "../../../defines.h"
#include "../UiStack.h"

typedef struct TextBoxData TextBoxData;

struct TextBoxData
{
	uint cursorPos;
	uint maxLength;
	char *text;
	char placeholder[32];
};

Control *CreateTextBoxControl(const char *placeholder, const Vector2 position, const Vector2 size, const ControlAnchor anchor, const uint maxLength);

void DrawTextBox(const Control *c, ControlState state, Vector2 position);

void UpdateTextBox(UiStack *stack, Control *, Vector2 localMousePos, uint ctlIndex);

void DestroyTextBox(const Control *c);

#endif //TEXTBOX_H
