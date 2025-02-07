//
// Created by droc101 on 4/21/2024.
//

#include "Font.h"
#include <string.h>
#include "../../Structs/Vector2.h"
#include "../Core/Error.h"
#include "../Core/MathEx.h"
#include "RenderingHelpers.h"

int FontFindChar(char target, const Font *font)
{
	if (font->uppercase_only)
	{
		target = (char)toupper(target);
	}
	int i = 0;
	while (font->chars[i] != 0)
	{
		if (font->chars[i] == target)
		{
			return i;
		}
		i++;
	}
	return -1; // Character not found
}

Vector2 FontDrawString(const Vector2 pos, const char *str, const uint size, const uint color, const Font *font)
{
	const ulong stringLength = strlen(str);
	float *verts = calloc(stringLength, sizeof(float[4][4]));
	CheckAlloc(verts);
	uint *indices = calloc(stringLength, sizeof(uint[6]));
	CheckAlloc(indices);

	int x = (int)pos.x;
	int y = (int)pos.y;
	int i = 0;
	const double sizeMultiplier = (double)size / font->default_size;
	const int width = (int)(font->width * sizeMultiplier);
	const int quadHeight = (int)(font->texture_height * sizeMultiplier);
	const int baselineHeight = (int)(font->baseline * sizeMultiplier);
	const double uvPixel = 1.0 / font->image->width;
	while (str[i] != '\0')
	{
		const int fSize = (int)((font->char_widths[(int)str[i]] + font->char_spacing) * sizeMultiplier);

		if (str[i] == ' ')
		{
			i++;
			x += (int)((font->space_width + font->char_spacing) * sizeMultiplier);
			continue;
		}

		if (str[i] == '\n')
		{
			i++;
			x = (int)pos.x;
			y += (int)((baselineHeight + font->line_spacing) * sizeMultiplier);
			continue;
		}

		const double uvPerChar = 1.0 / (double)strlen(font->chars);
		// ReSharper disable once CppRedundantCastExpression
		int index = FontFindChar((char)str[i], font);
		if (index == -1)
		{
			index = FontFindChar('?', font);
		}

		const Vector2 ndcPos = v2(X_TO_NDC((float)x), Y_TO_NDC((float)y));
		const Vector2 ndcPosEnd = v2(X_TO_NDC((float)(x + width)), Y_TO_NDC((float)(y + quadHeight)));
		const double charUV = uvPerChar * index;
		const double charUVEnd = uvPerChar * (index + 1) - uvPixel;

		const mat4 quad = {
			{(float)ndcPos.x, (float)ndcPos.y, (float)charUV, 0},
			{(float)ndcPos.x, (float)ndcPosEnd.y, (float)charUV, 1},
			{(float)ndcPosEnd.x, (float)ndcPosEnd.y, (float)charUVEnd, 1},
			{(float)ndcPosEnd.x, (float)ndcPos.y, (float)charUVEnd, 0},
		};

		memcpy(verts + i * 16, quad, sizeof(quad));

		uint quadIndices[6] = {0, 1, 2, 0, 2, 3};
		for (int j = 0; j < 6; j++)
		{
			quadIndices[j] += i * 4;
		}

		memcpy(indices + i * 6, quadIndices, sizeof(quadIndices));

		x += fSize;
		i++;
	}

	BatchedQuadArray quads;
	quads.verts = verts;
	quads.indices = indices;
	quads.quad_count = (int)stringLength;
	DrawBatchedQuadsTextured(&quads, font->texture, color);

	free(verts);
	free(indices);

	return v2((float)(x + width), (float)(y + size)); // Return the bottom right corner of the text
}

Vector2 MeasureText(const char *str, const uint size, const Font *font)
{
	int textWidth = 0;
	int textHeight = (int)size;
	int tempWidth = 0;
	const double sizeMultiplier = (double)size / font->default_size;
	for (int j = 0; j < strlen(str); j++)
	{
		const int fSize = (int)((font->char_widths[(int)str[j]] + font->char_spacing) * sizeMultiplier);
		tempWidth += fSize;
		if (str[j] == ' ')
		{
			tempWidth -= fSize;
			tempWidth += (int)(font->space_width * sizeMultiplier);
		}
		else if (str[j+1] == '\0')
        {
			tempWidth -= (int)(font->char_spacing * sizeMultiplier); // fix extra spacing at the end of the string
        }
		else if (str[j] == '\n')
		{
			tempWidth -= fSize;
			textWidth = max(textWidth, tempWidth);
			tempWidth = 0;
			textHeight += (int)((size + font->line_spacing) * sizeMultiplier);
		}
	}

	textWidth = max(textWidth, tempWidth);

	return v2((float)textWidth, (float)textHeight);
}

int StringLineCount(const char *str)
{
	int count = 1;
	for (int i = 0; i < strlen(str); i++)
	{
		if (str[i] == '\n')
		{
			count++;
		}
	}
	return count;
}

int MeasureLine(const char *str, const int line)
{
	int i = 0;
	int count = 0;
	while (str[i] != '\0')
	{
		if (str[i] == '\n')
		{
			count++;
		}
		if (count == line)
		{
			return i;
		}
		i++;
	}
	return i;
}

void TextGetLine(const char *str, const int line, char *out)
{
	int start = MeasureLine(str, line);

	// if the start is a newline, skip it
	if (str[start] == '\n')
	{
		start++;
	}

	const int end = MeasureLine(str, line + 1);

	// ensure start is less than end
	if (start > end)
	{
		out[0] = '\0';
		return;
	}

	strncpy(out, str + start, end - start);
	out[end - start] = '\0';
}

void DrawTextAligned(const char *str,
					 const uint size,
					 const uint color,
					 const Vector2 rect_pos,
					 const Vector2 rect_size,
					 const byte h_align,
					 const byte v_align,
					 const Font *font)
{
	const int lines = StringLineCount(str);
	int x;
	int y = (int)rect_pos.y;
	if (v_align == FONT_VALIGN_MIDDLE)
	{
		y += ((int)rect_size.y - lines * (int)size) / 2;
	} else if (v_align == FONT_VALIGN_BOTTOM)
	{
		y += (int)rect_size.y - lines * (int)size;
	}

	for (int i = 0; i < lines; i++)
	{
		char line[256];
		TextGetLine(str, i, line);
		const Vector2 textSize = MeasureText(line, size, font);
		if (h_align == FONT_HALIGN_CENTER)
		{
			x = (int)(rect_pos.x + (rect_size.x - textSize.x) / 2);
		} else if (h_align == FONT_HALIGN_RIGHT)
		{
			x = (int)(rect_pos.x + rect_size.x - textSize.x);
		} else
		{
			x = (int)rect_pos.x;
		}
		FontDrawString(v2((float)x, (float)y), line, size, color, font);
		y += (int)(size + font->line_spacing);
	}
}
