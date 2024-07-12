//
// Created by droc101 on 7/11/2024.
//

#ifndef GAME_TEXTBOX_H
#define GAME_TEXTBOX_H

typedef struct {
    char *text;
    int rows;
    int cols;
    int x;
    int y;

    int h_align;
    int v_align;

    int theme;
} TextBox;

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

void TextBoxRender(TextBox box, int page);

#endif //GAME_TEXTBOX_H
