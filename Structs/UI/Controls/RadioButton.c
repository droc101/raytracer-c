//
// Created by droc101 on 10/27/2024.
//

#include "RadioButton.h"
#include "../../../Assets/Assets.h"
#include "../../../Helpers/Core/Input.h"
#include "../../GlobalState.h"
#include "../../../Helpers/Graphics/Drawing.h"
#include "../../../Helpers/Graphics/Font.h"

Control *CreateRadioButtonControl(Vector2 position, Vector2 size, char *label, void (*callback)(bool, byte, byte), ControlAnchor anchor, bool checked, byte groupId, byte id) {
    Control *radio = CreateEmptyControl();
    radio->type = RADIO_BUTTON;
    radio->position = position;
    radio->size = size;
    radio->anchor = anchor;

    radio->ControlData = malloc(sizeof(RadioButtonData));
    RadioButtonData *data = (RadioButtonData *) radio->ControlData;
    data->label = label;
    data->checked = checked;
    data->callback = callback;
    data->groupId = groupId;
    data->id = id;

    return radio;
}

void DestroyRadioButton(Control *c) {
    RadioButtonData *data = (RadioButtonData *) c->ControlData;
    free(data);
}

void UpdateRadioButton(UiStack *stack, Control *c, Vector2 localMousePos, uint ctlIndex) {
    RadioButtonData *data = (RadioButtonData *) c->ControlData;

    if (HasActivation(stack, c)) {
        if (data->checked) {
            return; // do not allow re-checking
        }


        PlaySoundEffect(gzwav_sfx_click);
        data->checked = true;

        // Find all radio buttons with the same group id and uncheck them
        for (uint i = 0; i < ListGetSize(stack->Controls); i++) {
            Control *control = ListGet(stack->Controls, i);
            if (control->type == RADIO_BUTTON) {
                RadioButtonData *radioData = (RadioButtonData *) control->ControlData;
                if (radioData->groupId == data->groupId && radioData->id != data->id) {
                    radioData->checked = false;
                }
            }
        }

        ConsumeMouseButton(SDL_BUTTON_LEFT);
        ConsumeKey(SDL_SCANCODE_SPACE);

        if (data->callback != NULL) {
            data->callback(data->checked, data->groupId, data->id);
        }
    }
}

void DrawRadioButton(Control *c, ControlState state, Vector2 position) {
    RadioButtonData *data = (RadioButtonData *) c->ControlData;
    DrawTextAligned(data->label, 16, 0xFFFFFFFF, c->anchoredPosition, c->size, FONT_HALIGN_LEFT, FONT_VALIGN_MIDDLE, true);

    setColorUint(0xFF0000ff);

    Vector2 boxSize = v2s(32);
    Vector2 boxPos = v2(position.x + c->size.x - boxSize.x - 2, position.y + c->size.y / 2 - boxSize.y / 2);
    DrawTexture(boxPos, boxSize, data->checked ? gztex_interface_radio_checked : gztex_interface_radio_unchecked);
}
