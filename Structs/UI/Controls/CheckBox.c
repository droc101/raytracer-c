//
// Created by droc101 on 10/27/2024.
//

#include "CheckBox.h"
#include "../../../Assets/Assets.h"
#include "../../../Helpers/Core/Input.h"
#include "../../GlobalState.h"
#include "../../../Helpers/Graphics/Drawing.h"
#include "../../../Helpers/Graphics/Font.h"

Control *CreateCheckboxControl(Vector2 position, Vector2 size, char *label, void (*callback)(bool), ControlAnchor anchor, bool checked) {
    Control *checkbox = CreateEmptyControl();
    checkbox->type = CHECKBOX;
    checkbox->position = position;
    checkbox->size = size;
    checkbox->anchor = anchor;

    checkbox->ControlData = malloc(sizeof(CheckBoxData));
    CheckBoxData *data = (CheckBoxData *) checkbox->ControlData;
    data->label = label;
    data->checked = checked;
    data->callback = callback;

    return checkbox;
}

void DestroyCheckbox(Control *c) {
    CheckBoxData *data = (CheckBoxData *) c->ControlData;
    free(data);
}

void UpdateCheckbox(UiStack *stack, Control *c, Vector2 localMousePos, uint ctlIndex) {
    CheckBoxData *data = (CheckBoxData *) c->ControlData;

    if (HasActivation(stack, c)) {
        PlaySoundEffect(gzwav_sfx_click);
        data->checked = !data->checked;

        ConsumeMouseButton(SDL_BUTTON_LEFT);
        ConsumeKey(SDL_SCANCODE_SPACE);

        if (data->callback != NULL) {
            data->callback(data->checked);
        }
    }
}

void DrawCheckbox(Control *c, ControlState state, Vector2 position) {
    CheckBoxData *data = (CheckBoxData *) c->ControlData;
    DrawTextAligned(data->label, 16, 0xFFFFFFFF, c->anchoredPosition, c->size, FONT_HALIGN_LEFT, FONT_VALIGN_MIDDLE, true);

    setColorUint(0xFF000000);

    Vector2 boxSize = v2s(32);
    Vector2 boxPos = v2(position.x + c->size.x - boxSize.x - 2, position.y + c->size.y / 2 - boxSize.y / 2);
    DrawTexture(boxPos, boxSize, data->checked ? gztex_interface_checkbox_checked : gztex_interface_checkbox_unchecked);
}
