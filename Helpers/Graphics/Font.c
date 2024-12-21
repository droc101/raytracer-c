//
// Created by droc101 on 4/21/2024.
//

#include "Font.h"
#include <ctype.h>
#include <string.h>
#include "../../Assets/Assets.h"
#include "../../Structs/Vector2.h"
#include "../Core/Error.h"
#include "../Core/MathEx.h"
#include "RenderingHelpers.h"

const char fontChars[] = "abcdefghijklmnopqrstuvwxyz0123456789.:-,/\\|[]{}();'\"<>`~!@#$%^*_=+?";

int FontFindChar(const char target)
{
	int i = 0;
	while (fontChars[i] != 0)
	{
		if (fontChars[i] == target)
		{
			return i;
		}
		i++;
	}
	return -1; // Character not found
}

Vector2 FontDrawString(const Vector2 pos, const char *str, const uint size, const uint color, const bool small)
{
	const int str_len = strlen(str);
	float *verts = malloc(sizeof(float[4][4]) * str_len);
	chk_malloc(verts);
	uint *indices = malloc(sizeof(uint[6]) * str_len);
	chk_malloc(indices);
	memset(verts, 0, sizeof(float[4][4]) * str_len);
	memset(indices, 0, sizeof(uint[6]) * str_len);

	int x = pos.x;
	int y = pos.y;
	int i = 0;
	const int sizeX = small ? size * 0.75 : size;
	while (str[i] != '\0')
	{
		if (str[i] == ' ')
		{
			i++;
			x += sizeX;
		} else if (str[i] == '\n')
		{
			i++;
			x = pos.x;
			y += size;
		}

		const float uv_per_char = 1.0f / strlen(fontChars);
		int index = FontFindChar(tolower(str[i]));
		if (index == -1)
		{
			index = FontFindChar('U');
		}

		const Vector2 ndc_pos = v2(X_TO_NDC(x), Y_TO_NDC(y));
		const Vector2 ndc_pos_end = v2(X_TO_NDC(x + sizeX), Y_TO_NDC(y + size));
		const float charUV = uv_per_char * index;
		const float charUVEnd = uv_per_char * (index + 1);

		const mat4 quad = {
			{ndc_pos.x, ndc_pos.y, charUV, 0},
			{ndc_pos.x, ndc_pos_end.y, charUV, 1},
			{ndc_pos_end.x, ndc_pos_end.y, charUVEnd, 1},
			{ndc_pos_end.x, ndc_pos.y, charUVEnd, 0},
		};

		memcpy(verts + i * 16, quad, sizeof(quad));

		uint quad_indices[6] = {0, 1, 2, 0, 2, 3};
		for (int j = 0; j < 6; j++)
		{
			quad_indices[j] += i * 4;
		}

		memcpy(indices + i * 6, quad_indices, sizeof(quad_indices));

		x += sizeX;
		i++;
	}

	BatchedQuadArray quads;
	quads.verts = verts;
	quads.indices = indices;
	quads.quad_count = str_len;
	DrawBatchedQuadsTextured(&quads, small ? gztex_interface_small_fonts : gztex_interface_font, color);

	free(verts);
	free(indices);

	return v2(x + sizeX, y + size); // Return the bottom right corner of the text
}

Vector2 MeasureText(const char *str, const uint size, const bool small)
{
	int textWidth = 0;
	int textHeight = size;
	int tempWidth = 0;
	const int sizeX = small ? size * 0.75 : size;
	for (int j = 0; j < strlen(str); j++)
	{
		tempWidth += sizeX;
		if (str[j] == '\n')
		{
			tempWidth -= sizeX;
			textWidth = max(textWidth, tempWidth);
			tempWidth = 0;
			textHeight += size;
		}
	}

	textWidth = max(textWidth, tempWidth);

	return v2(textWidth, textHeight);
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
					 const bool small)
{
	const int lines = StringLineCount(str);
	int x;
	int y = rect_pos.y;
	if (v_align == FONT_VALIGN_MIDDLE)
	{
		y += (rect_size.y - lines * size) / 2;
	} else if (v_align == FONT_VALIGN_BOTTOM)
	{
		y += rect_size.y - lines * size;
	}

	for (int i = 0; i < lines; i++)
	{
		char line[256];
		TextGetLine(str, i, line);
		const Vector2 textSize = MeasureText(line, size, small);
		if (h_align == FONT_HALIGN_CENTER)
		{
			x = rect_pos.x + (rect_size.x - textSize.x) / 2;
		} else if (h_align == FONT_HALIGN_RIGHT)
		{
			x = rect_pos.x + rect_size.x - textSize.x;
		} else
		{
			x = rect_pos.x;
		}
		FontDrawString(v2(x, y), line, size, color, small);
		y += size;
	}
}
