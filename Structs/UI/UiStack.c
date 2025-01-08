//
// Created by droc101 on 10/7/2024.
//

#include "UiStack.h"

// #include "../../Assets/Assets.h"
#include "../../Helpers/Core/AssetReader.h"
#include "../../Helpers/Core/Error.h"
#include "../../Helpers/Core/Input.h"
#include "../../Helpers/Graphics/Drawing.h"
#include "../GlobalState.h"
#include "../Vector2.h"

#include "Controls/Button.h"
#include "Controls/CheckBox.h"
#include "Controls/RadioButton.h"
#include "Controls/Slider.h"
#include "Controls/TextBox.h"

typedef void (*ControlDrawFunc)(const Control *, ControlState state, Vector2 position);
typedef void (*ControlUpdateFunc)(UiStack *stack, Control *, Vector2 localMousePos, uint ctlIndex);
typedef void (*ControlDestroyFunc)(const Control *);

/**
 * Destroy functions for each control type
 */
const ControlDestroyFunc ControlDestroyFuncs[CONTROL_TYPE_COUNT] = {
	DestroyButton, // BUTTON
	DestroySlider, // SLIDER
	DestroyCheckbox, // CHECKBOX
	DestroyRadioButton, // RADIO_BUTTON
	DestroyTextBox, // TEXTBOX
};

/**
 * Draw functions for each control type
 */
const ControlDrawFunc ControlDrawFuncs[CONTROL_TYPE_COUNT] = {
	DrawButton, // BUTTON
	DrawSlider, // SLIDER
	DrawCheckbox, // CHECKBOX
	DrawRadioButton, // RADIO_BUTTON
	DrawTextBox, // TEXTBOX
};

/**
 * Update functions for each control type
 */
const ControlUpdateFunc ControlUpdateFuncs[CONTROL_TYPE_COUNT] = {
	UpdateButton, // BUTTON
	UpdateSlider, // SLIDER
	UpdateCheckbox, // CHECKBOX
	UpdateRadioButton, // RADIO_BUTTON
	UpdateTextBox, // TEXTBOX
};

UiStack *CreateUiStack()
{
	UiStack *stack = malloc(sizeof(UiStack));
	chk_malloc(stack);
	stack->Controls = CreateList();
	stack->ActiveControl = -1;
	stack->ActiveControlState = NORMAL;
	UiStackResetFocus(stack);
	return stack;
}

void DestroyUiStack(UiStack *stack)
{
	for (int i = 0; i < stack->Controls->size; i++)
	{
		const Control *c = ListGet(stack->Controls, i);
		ControlDestroyFuncs[c->type](c);
	}
	ListFreeWithData(stack->Controls);
	free(stack);
	stack = NULL;
}

bool ProcessUiStack(UiStack *stack)
{
	const Vector2 mousePos = GetMousePos();

	if (stack->focusedControl != -1)
	{
		Control *c = ListGet(stack->Controls, stack->focusedControl);
		ControlUpdateFuncs[c->type](stack,
									c,
									v2(mousePos.x - c->position.x, mousePos.y - c->position.y),
									stack->focusedControl);
	}
	if (stack->ActiveControl != -1)
	{
		Control *c = ListGet(stack->Controls, stack->ActiveControl);
		ControlUpdateFuncs[c->type](stack,
									c,
									v2(mousePos.x - c->position.x, mousePos.y - c->position.y),
									stack->ActiveControl);
	}


	for (int i = stack->Controls->size - 1; i >= 0; i--)
	{
		Control *c = ListGet(stack->Controls, i);

		c->anchoredPosition = CalculateControlPosition(c);
	}

	if (IsMouseButtonPressed(SDL_BUTTON_LEFT) || IsButtonPressed(CONTROLLER_OK))
	{
		stack->focusedControl = stack->ActiveControl;
		stack->ActiveControlState = ACTIVE;
		return stack->ActiveControl != -1;
	}

	stack->ActiveControl = -1;
	stack->ActiveControlState = NORMAL;

	if (UseController())
	{
		stack->ActiveControl = stack->focusedControl;
		stack->ActiveControlState = HOVER;
		if (IsButtonPressed(CONTROLLER_OK))
		{
			stack->ActiveControlState = ACTIVE;
		}
	} else
	{
		// iterate through the controls in reverse order so that the last control is on top and gets priority
		for (int i = stack->Controls->size - 1; i >= 0; i--)
		{
			const Control *c = (Control *)ListGet(stack->Controls, i);

			const Vector2 localMousePos = v2(mousePos.x - c->anchoredPosition.x, mousePos.y - c->anchoredPosition.y);
			if (localMousePos.x >= 0 &&
				localMousePos.x <= c->size.x &&
				localMousePos.y >= 0 &&
				localMousePos.y <= c->size.y)
			{
				stack->ActiveControl = i;
				if (IsMouseButtonPressed(SDL_BUTTON_LEFT) ||
					IsKeyJustPressed(SDL_SCANCODE_SPACE) ||
					IsButtonJustPressed(CONTROLLER_OK))
				{
					stack->ActiveControlState = ACTIVE;
					// make this control the focused control
					stack->focusedControl = i;
				} else
				{
					stack->ActiveControlState = HOVER;
				}
				break;
			}
		}
	}

	// process tab and shift+tab to cycle through controls
	if ((IsKeyJustPressed(SDL_SCANCODE_TAB) && !IsKeyPressed(SDL_SCANCODE_LSHIFT)) ||
		IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_DOWN))
	{
		if (stack->focusedControl == -1)
		{
			stack->focusedControl = 0;
		} else
		{
			stack->focusedControl = (stack->focusedControl + 1) % stack->Controls->size;
		}
	} else if ((IsKeyJustPressed(SDL_SCANCODE_TAB) && IsKeyPressed(SDL_SCANCODE_LSHIFT)) ||
			   IsButtonJustPressed(SDL_CONTROLLER_BUTTON_DPAD_UP))
	{
		if (stack->focusedControl == -1)
		{
			stack->focusedControl = stack->Controls->size - 1;
		} else
		{
			stack->focusedControl = (stack->focusedControl - 1) % stack->Controls->size;
		}
		// ensure the index is positive
		if (stack->focusedControl < 0)
		{
			stack->focusedControl += stack->Controls->size;
		}
	}


	// return whether the mouse is over a control
	return stack->ActiveControl != -1;
}

void DrawUiStack(const UiStack *stack)
{
	for (int i = 0; i < stack->Controls->size; i++)
	{
		const Control *c = ListGet(stack->Controls, i);
		ControlDrawFuncs[c->type](c,
								  i == stack->ActiveControl ? stack->ActiveControlState : NORMAL,
								  c->anchoredPosition);

		// if this is the focused control, draw a border around it
		if (i == stack->focusedControl)
		{
			SetColorUint(0xFFFFFFFF);
			DrawNinePatchTexture(v2(c->anchoredPosition.x - 4, c->anchoredPosition.y - 4),
								 v2(c->size.x + 8, c->size.y + 8),
								 16,
								 16,
								 TEXTURE("interface_focus_rect"));
		}
	}
}

Vector2 CalculateControlPosition(const Control *control)
{
	Vector2 pos = control->position;
	const ControlAnchor anchor = control->anchor;

	switch (anchor)
	{
		case TOP_LEFT:
			pos = v2(0, 0);
			break;
		case TOP_CENTER:
			pos.x = (WindowWidth() - control->size.x) / 2;
			pos.y = 0;
			break;
		case TOP_RIGHT:
			pos.x = WindowWidth() - control->size.x;
			pos.y = 0;
			break;
		case MIDDLE_LEFT:
			pos.y = (WindowHeight() - control->size.y) / 2;
			pos.x = 0;
			break;
		case MIDDLE_CENTER:
			pos = v2((WindowWidth() - control->size.x) / 2, (WindowHeight() - control->size.y) / 2);
			break;
		case MIDDLE_RIGHT:
			pos = v2(WindowWidth() - control->size.x, (WindowHeight() - control->size.y) / 2);
			break;
		case BOTTOM_LEFT:
			pos.y = WindowHeight() - control->size.y;
			pos.x = 0;
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

Control *CreateEmptyControl()
{
	Control *c = malloc(sizeof(Control));
	chk_malloc(c);
	c->ControlData = NULL;
	return c;
}

void UiStackPush(const UiStack *stack, Control *control)
{
	ListAdd(stack->Controls, control);
}

void UiStackRemove(const UiStack *stack, const Control *control)
{
	ControlDestroyFuncs[control->type](control);

	ListRemoveAt(stack->Controls, ListFind(stack->Controls, control));
}

bool IsMouseInRect(const Vector2 pos, const Vector2 size)
{
	const Vector2 mousePos = GetMousePos();
	return mousePos.x >= pos.x && mousePos.x <= pos.x + size.x && mousePos.y >= pos.y && mousePos.y <= pos.y + size.y;
}

bool HasMouseActivation(UiStack * /*stack*/, const Control *Control)
{
	return IsMouseInRect(Control->anchoredPosition, Control->size) && IsMouseButtonJustReleased(SDL_BUTTON_LEFT);
}

bool HasKeyboardActivation(UiStack * /*stack*/, Control * /*Control*/)
{
	return IsKeyJustPressed(SDL_SCANCODE_RETURN) ||
		   IsKeyJustPressed(SDL_SCANCODE_SPACE) ||
		   IsButtonJustReleased(CONTROLLER_OK);
}

bool HasActivation(UiStack *stack, Control *Control)
{
	const int index = ListFind(stack->Controls, Control);
	bool focus = false;
	if (index == stack->ActiveControl)
	{
		focus |= HasMouseActivation(stack, Control);
	}
	if (index == stack->focusedControl)
	{
		focus |= HasKeyboardActivation(stack, Control);
	}
	return focus;
}

void UiStackResetFocus(UiStack *stack)
{
	stack->focusedControl = UseController() ? 0 : -1;
}
