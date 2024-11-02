//
// Created by droc101 on 10/7/2024.
//

#include "UiStack.h"
#include "../../Helpers/Core/Input.h"
#include "../Vector2.h"

#include "Controls/Button.h"
#include "Controls/Slider.h"
#include "Controls/CheckBox.h"
#include "Controls/RadioButton.h"

void (*ControlDestroyFuncs[4])(Control *) = {
        DestroyButton, // BUTTON
        DestroySlider, // SLIDER
        DestroyCheckbox, // CHECKBOX
        DestroyRadioButton, // RADIO_BUTTON
};

void (*ControlDrawFuncs[4])(Control *, ControlState state, Vector2 position) = {
        DrawButton, // BUTTON
        DrawSlider, // SLIDER
        DrawCheckbox, // CHECKBOX
        DrawRadioButton, // RADIO_BUTTON
};

void (*ControlUpdateFuncs[4])(UiStack *stack, Control *, Vector2 localMousePos, uint ctlIndex) = {
        UpdateButton, // BUTTON
        UpdateSlider, // SLIDER
        UpdateCheckbox, // CHECKBOX
        UpdateRadioButton, // RADIO_BUTTON
};

UiStack *CreateUiStack()
{
    UiStack *stack = (UiStack *) malloc(sizeof(UiStack));
    stack->Controls = CreateList();
    stack->ActiveControl = -1;
    stack->ActiveControlState = NORMAL;
    stack->focusedControl = -1;
    return stack;
}

void DestroyUiStack(UiStack *stack)
{
    for (int i = 0; i < stack->Controls->size; i++)
    {
        Control *c = (Control *) ListGet(stack->Controls, i);
        ControlDestroyFuncs[c->type](c);
    }
    ListFreeWithData(stack->Controls);
    free(stack);
}

bool ProcessUiStack(UiStack *stack)
{
    Vector2 mousePos = GetMousePos();

    if (stack->focusedControl != -1)
    {
        Control *c = (Control *) ListGet(stack->Controls, stack->focusedControl);
        ControlUpdateFuncs[c->type](stack, c, v2(mousePos.x - c->position.x, mousePos.y - c->position.y),
                                    stack->focusedControl);
    }
    if (stack->ActiveControl != -1)
    {
        Control *c = (Control *) ListGet(stack->Controls, stack->ActiveControl);
        ControlUpdateFuncs[c->type](stack, c, v2(mousePos.x - c->position.x, mousePos.y - c->position.y),
                                    stack->ActiveControl);
    }

    stack->ActiveControl = -1;
    stack->ActiveControlState = NORMAL;

    for (int i = stack->Controls->size - 1; i >= 0; i--)
    {
        Control *c = (Control *) ListGet(stack->Controls, i);

        c->anchoredPosition = CalculateControlPosition(c);
    }

    // iterate through the controls in reverse order so that the last control is on top and gets priority
    for (int i = stack->Controls->size - 1; i >= 0; i--)
    {
        Control *c = (Control *) ListGet(stack->Controls, i);

        Vector2 localMousePos = v2(mousePos.x - c->anchoredPosition.x, mousePos.y - c->anchoredPosition.y);
        if (localMousePos.x >= 0 && localMousePos.x <= c->size.x && localMousePos.y >= 0 &&
            localMousePos.y <= c->size.y)
        {
            stack->ActiveControl = i;
            if (IsMouseButtonPressed(SDL_BUTTON_LEFT) || IsKeyJustPressed(SDL_SCANCODE_SPACE))
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

    // process tab and shift+tab to cycle through controls
    if (IsKeyJustPressed(SDL_SCANCODE_TAB) && !IsKeyPressed(SDL_SCANCODE_LSHIFT))
    {
        if (stack->focusedControl == -1)
        {
            stack->focusedControl = 0;
        } else
        {
            stack->focusedControl = (stack->focusedControl + 1) % stack->Controls->size;
        }
    } else if (IsKeyJustPressed(SDL_SCANCODE_TAB) && IsKeyPressed(SDL_SCANCODE_LSHIFT))
    {
        if (stack->focusedControl == -1)
        {
            stack->focusedControl = stack->Controls->size - 1;
        } else
        {
            stack->focusedControl = (stack->focusedControl - 1) % stack->Controls->size;
        }
    }

    // return whether the mouse is over a control
    return stack->ActiveControl != -1;
}

void DrawUiStack(UiStack *stack)
{
    for (int i = 0; i < stack->Controls->size; i++)
    {
        Control *c = (Control *) ListGet(stack->Controls, i);
        ControlDrawFuncs[c->type](c, i == stack->ActiveControl ? stack->ActiveControlState : NORMAL,
                                  c->anchoredPosition);

        // if this is the focused control, draw a border around it
        if (i == stack->focusedControl)
        {
            setColorUint(0xFFFFFFFF);
            DrawOutlineRect(v2(c->anchoredPosition.x - 2, c->anchoredPosition.y - 2), v2(c->size.x + 4, c->size.y + 4),
                            3);
        }
    }
}

Vector2 CalculateControlPosition(Control *control)
{
    Vector2 pos = control->position;
    ControlAnchor anchor = control->anchor;

    switch (anchor)
    {
        case TOP_LEFT:
            pos = v2(0, 0);
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

Control *CreateEmptyControl()
{
    Control *c = (Control *) malloc(sizeof(Control));
    c->ControlData = NULL;
    return c;
}

void UiStackPush(UiStack *stack, Control *control)
{
    ListAdd(stack->Controls, control);
}

void UiStackRemove(UiStack *stack, Control *control)
{
    ControlDestroyFuncs[control->type](control);

    ListRemoveAt(stack->Controls, ListFind(stack->Controls, control));
}

bool IsMouseInRect(Vector2 pos, Vector2 size)
{
    Vector2 mousePos = GetMousePos();
    return mousePos.x >= pos.x && mousePos.x <= pos.x + size.x && mousePos.y >= pos.y && mousePos.y <= pos.y + size.y;
}

bool HasMouseActivation(UiStack *stack, Control *Control)
{
    return IsMouseInRect(Control->anchoredPosition, Control->size) &&
           IsMouseButtonJustReleased(SDL_BUTTON_LEFT);

}

bool HasKeyboardActivation(UiStack *stack, Control *Control)
{
    return (IsKeyJustPressed(SDL_SCANCODE_RETURN) || IsKeyJustPressed(SDL_SCANCODE_SPACE));
}

bool HasActivation(UiStack *stack, Control *Control)
{
    int index = ListFind(stack->Controls, Control);
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
