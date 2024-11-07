//
// Created by droc101 on 7/11/2024.
//

#include "TextBox.h"
#include "Graphics/Drawing.h"
#include "Graphics/Font.h"

#define BOX_OUTER_PADDING 14
#define TEXT_BOX_FONT_SIZE 24
#define TEXT_BOX_FONT_WIDTH 18

ulong textBoxThemes[3] = {
    0x80000000FFFFFFFF,
    0xA0FFFFFFFF000000,
    0x80200000FFFFEEEE
};

void TextBoxRender(const TextBox *box, const int page)
{
    const int startLine = box->rows * page;
    const int lineCount = StringLineCount(box->text);
    int endLine = startLine + box->rows;
    if (endLine > lineCount)
    {
        endLine = lineCount;
    }

    Vector2 topLeft = {0, 0};

    const uint textColor = textBoxThemes[box->theme];
    const uint boxColor = textBoxThemes[box->theme] >> 32;

    const int width = (box->cols * TEXT_BOX_FONT_WIDTH) + (BOX_OUTER_PADDING * 2);
    const int height = (box->rows * TEXT_BOX_FONT_SIZE) + (BOX_OUTER_PADDING * 2);

    if (box->h_align == TEXT_BOX_H_ALIGN_CENTER)
    {
        topLeft.x = (WindowWidth() - width) / 2;
    } else if (box->h_align == TEXT_BOX_H_ALIGN_RIGHT)
    {
        topLeft.x = WindowWidth() - width;
    } else
    {
        topLeft.x = 0;
    }

    if (box->v_align == TEXT_BOX_V_ALIGN_CENTER)
    {
        topLeft.y = (WindowHeight() - height) / 2;
    } else if (box->v_align == TEXT_BOX_V_ALIGN_BOTTOM)
    {
        topLeft.y = WindowHeight() - height;
    } else
    {
        topLeft.y = 0;
    }

    topLeft.x += box->x;
    topLeft.y += box->y;

    setColorUint(boxColor);
    draw_rect(topLeft.x, topLeft.y, box->cols * TEXT_BOX_FONT_WIDTH + BOX_OUTER_PADDING * 2,
              box->rows * TEXT_BOX_FONT_SIZE + BOX_OUTER_PADDING * 2);

    int txtY = topLeft.y + BOX_OUTER_PADDING;
    for (int i = startLine; i < endLine; i++)
    {
        char line[256];
        TextGetLine(box->text, i, line);
        const Vector2 pos = {topLeft.x + BOX_OUTER_PADDING, txtY};
        FontDrawString(pos, line, TEXT_BOX_FONT_SIZE, textColor, true);
        txtY += TEXT_BOX_FONT_SIZE;
    }
}
