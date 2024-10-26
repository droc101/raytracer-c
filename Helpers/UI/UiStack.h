//
// Created by droc101 on 10/7/2024.
//

#ifndef GAME_UISTACK_H
#define GAME_UISTACK_H

#include "../../defines.h"
#include "../Drawing.h"

typedef enum {
    BUTTON,
    SLIDER,
} ControlType;

typedef enum {
    NORMAL,
    HOVER,
    ACTIVE
} ControlState;

typedef enum {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    MIDDLE_LEFT,
    MIDDLE_CENTER,
    MIDDLE_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
} ControlAnchor;

typedef struct {
    ControlType type;
    ControlAnchor anchor;
    Vector2 position;
    Vector2 size;

    Vector2 anchoredPosition;

    void *ControlData;
} Control;

typedef struct {
    List *Controls;

    int ActiveControl;
    ControlState ActiveControlState;

    int focusedControl;
} UiStack;

/**
 * Create a new UiStack
 * @return The new UiStack
 */
UiStack *CreateUiStack();

/**
 * Destroy a UiStack
 * @param stack The UiStack to destroy
 */
void DestroyUiStack(UiStack *stack);

/**
 * Process the UiStack
 * @param stack The UiStack to process
 */
void ProcessUiStack(UiStack *stack);

/**
 * Draw the UiStack
 * @warning Call @c ProcessUiStack before calling this
 * @param stack The UiStack to draw
 */
void DrawUiStack(UiStack *stack);

/**
 * Calculate the position of a control based on its anchor
 * @param control The control to calculate the position for
 * @return The anchored position of the control
 */
Vector2 CalculateControlPosition(Control *control);

/**
 * Create an empty control.
 * @warning This control should not be used directly, use the Create*Control functions instead
 * @return The empty control
 */
Control *CreateEmptyControl();

/**
 * Add a control to the UiStack
 * @param stack The UiStack to add the control to
 * @param control The control to add
 */
void UiStackPush(UiStack *stack, Control *control);

/**
 * Check if the mouse is in a rectangle
 * @param pos The position of the rectangle
 * @param size The size of the rectangle
 * @return Whether the mouse is in the rectangle
 */
bool IsMouseInRect(Vector2 pos, Vector2 size);

#endif //GAME_UISTACK_H
