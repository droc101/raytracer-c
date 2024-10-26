//
// Created by droc101 on 10/7/2024.
//

#include "Button.h"
#include "../../Input.h"
#include "../../Font.h"
#include "../../Drawing.h"

Control *CreateButtonControl(Vector2 position, Vector2 size, char *text, void (*callback)(), ControlAnchor anchor) {
    Control *btn = CreateEmptyControl();
    btn->type = BUTTON;
    btn->position = position;
    btn->size = size;
    btn->anchor = anchor;

    btn->ControlData = malloc(sizeof(ButtonData));
    ButtonData *data = (ButtonData *) btn->ControlData;
    data->text = text;
    data->callback = callback;
    data->enabled = true;

    return btn;
}

void DestroyButton(Control *c) {
    ButtonData *data = (ButtonData *) c->ControlData;
    free(data);
}

void UpdateButton(UiStack *stack, Control *c, Vector2 localMousePos, uint ctlIndex) {
    ButtonData *data = (ButtonData *) c->ControlData;
    if (data->enabled && (IsMouseButtonJustReleased(SDL_BUTTON_LEFT) || IsKeyJustPressed(SDL_SCANCODE_SPACE))) {
        data->callback();
    }
}

void DrawButton(Control *c, ControlState state, Vector2 position) {
    uint color = 0xff000000;
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

    ButtonData *data = (ButtonData *) c->ControlData;

    if (!data->enabled) {
        color = 0xFF808080;
    }

    setColorUint(color);
    draw_rect(position.x, position.y, c->size.x, c->size.y);
    setColorUint(0xFF000000);
    DrawOutlineRect(position, c->size);

    DrawTextAligned(data->text, 16, 0xFF000000, position, c->size, FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE, true);
}
