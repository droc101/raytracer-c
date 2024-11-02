//
// Created by droc101 on 7/11/2024.
//

#ifndef GAME_TEXTBOX_H
#define GAME_TEXTBOX_H

#include "../defines.h"

#define TEXT_BOX_H_ALIGN_LEFT 0
#define TEXT_BOX_H_ALIGN_CENTER 1
#define TEXT_BOX_H_ALIGN_RIGHT 2

#define TEXT_BOX_V_ALIGN_TOP 0
#define TEXT_BOX_V_ALIGN_CENTER 1
#define TEXT_BOX_V_ALIGN_BOTTOM 2

#define TEXT_BOX_THEME_BLACK 0
#define TEXT_BOX_THEME_WHITE 1
#define TEXT_BOX_THEME_RED 2

#define DEFINE_TEXT(text, rows, cols, x, y, h_align, v_align, theme) \
    {text, rows, cols, x, y, h_align, v_align, theme}

/**
 * Render a text box
 * @param box The text box to render
 * @param page Page number to render
 */
void TextBoxRender(const TextBox *box, const int page);

#endif //GAME_TEXTBOX_H
