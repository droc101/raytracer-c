//
// Created by droc101 on 1/7/25.
//

#include "TextBox.h"
#include "../../../Assets/Assets.h"
#include "../../../Helpers/Core/Error.h"
#include "../../../Helpers/Core/Input.h"
#include "../../../Helpers/Graphics/Drawing.h"
#include "../../../Helpers/Graphics/Font.h"
#include "../../Vector2.h"
#include "../../../Helpers/Core/MathEx.h"

// Use when a key is to be used with shift+another
#define SC_IGNORE_NEEDS_SHIFT SDL_NUM_SCANCODES

// sorry
// I welcome any suggestions for a better way to do this
const char* chars = "abcdefghijklmnopqrstuvwxyz0123456789 .-,/\\[];'`=";
const SDL_Scancode codes[] = {
	SDL_SCANCODE_A,
	SDL_SCANCODE_B,
	SDL_SCANCODE_C,
	SDL_SCANCODE_D,
	SDL_SCANCODE_E,
	SDL_SCANCODE_F,
	SDL_SCANCODE_G,
	SDL_SCANCODE_H,
	SDL_SCANCODE_I,
	SDL_SCANCODE_J,
	SDL_SCANCODE_K,
	SDL_SCANCODE_L,
	SDL_SCANCODE_M,
	SDL_SCANCODE_N,
	SDL_SCANCODE_O,
	SDL_SCANCODE_P,
	SDL_SCANCODE_Q,
	SDL_SCANCODE_R,
	SDL_SCANCODE_S,
	SDL_SCANCODE_T,
	SDL_SCANCODE_U,
	SDL_SCANCODE_V,
	SDL_SCANCODE_W,
	SDL_SCANCODE_X,
	SDL_SCANCODE_Y,
	SDL_SCANCODE_Z,
	SDL_SCANCODE_0,
	SDL_SCANCODE_1,
	SDL_SCANCODE_2,
	SDL_SCANCODE_3,
	SDL_SCANCODE_4,
	SDL_SCANCODE_5,
	SDL_SCANCODE_6,
	SDL_SCANCODE_7,
	SDL_SCANCODE_8,
	SDL_SCANCODE_9,
	SDL_SCANCODE_SPACE,
	SDL_SCANCODE_PERIOD,
	SDL_SCANCODE_MINUS,
	SDL_SCANCODE_COMMA,
	SDL_SCANCODE_SLASH,
	SDL_SCANCODE_BACKSLASH,
	SDL_SCANCODE_LEFTBRACKET,
	SDL_SCANCODE_RIGHTBRACKET,
	SDL_SCANCODE_SEMICOLON,
	SDL_SCANCODE_APOSTROPHE,
	SDL_SCANCODE_GRAVE,
	SDL_SCANCODE_EQUALS,
};

const char shiftReplacements[][3] = {
	";:",
	"\\|",
	"[{",
	"]}",
	"9(",
	"0)",
	"'\"",
	",<",
	".>",
	"`~",
	"1!",
	"2@",
	"3#",
	"4$",
	"5%",
	"6^",
	"8*",
	"-_",
	"=+",
};

Control *CreateTextBoxControl(const char *placeholder, const Vector2 position, const Vector2 size, const ControlAnchor anchor, const uint maxLength)
{
	Control *c = CreateEmptyControl();
	c->type = TEXTBOX;
	c->anchor = anchor;
	c->position = position;
	c->size = size;

	TextBoxData *data = malloc(sizeof(TextBoxData));
	chk_malloc(data);
	data->cursorPos = 0;
	data->maxLength = maxLength;
	data->text = malloc(maxLength + 1);
	chk_malloc(data->text);
	memset(data->text, 0, maxLength + 1);
	strcpy(data->placeholder, placeholder); // up to caller to ensure placeholder is not too long

	c->ControlData = data;

	return c;
}

void DrawTextBox(const Control *c, ControlState state, Vector2 position)
{
	DrawNinePatchTexture(c->anchoredPosition, c->size, 8, 8, gztex_interface_slider);

	const TextBoxData *data = (TextBoxData *)c->ControlData;

	DrawTextAligned(data->text, 16, -1, v2(position.x + 4, position.y + 4), v2(c->size.x - 8, c->size.y - 8), FONT_HALIGN_LEFT, FONT_VALIGN_MIDDLE, true);

	DrawTextAligned("_", 16, -1, v2(position.x + 4 + 12 * data->cursorPos, position.y + 6), v2(12, c->size.y - 8), FONT_HALIGN_LEFT, FONT_VALIGN_MIDDLE, true);
}

void UpdateTextBox(UiStack *stack, Control *c, Vector2 localMousePos, uint ctlIndex)
{
	if (stack->ActiveControl != ctlIndex)
	{
		return;
	}

	TextBoxData *data = (TextBoxData *)c->ControlData;

	if (IsKeyJustPressed(SDL_SCANCODE_LEFT))
	{
		ConsumeKey(SDL_SCANCODE_LEFT);
		data->cursorPos -= 1;
	} else if (IsKeyJustPressed(SDL_SCANCODE_RIGHT))
	{
		ConsumeKey(SDL_SCANCODE_RIGHT);
		data->cursorPos += 1;
	}
	data->cursorPos = clampi(data->cursorPos, 0, strlen(data->text));

	// handle text input
	if (IsKeyJustPressed(SDL_SCANCODE_BACKSPACE))
	{
		if (data->cursorPos > 0)
		{
			ConsumeKey(SDL_SCANCODE_BACKSPACE);
			memmove(data->text + data->cursorPos - 1, data->text + data->cursorPos, strlen(data->text) - data->cursorPos + 1);
			data->cursorPos -= 1;
		}
	} else
	{
		const size_t arrLen = sizeof(codes) / sizeof(SDL_Scancode);
		for (int i = 0; i < arrLen; i++)
		{
			if (IsKeyJustPressed(codes[i]))
			{
				ConsumeKey(codes[i]);
				char chr = chars[i];
				if (IsKeyPressed(SDL_SCANCODE_LSHIFT) || IsKeyPressed(SDL_SCANCODE_RSHIFT))
				{
					for (int j = 0; j < sizeof(shiftReplacements) / 3; j++)
					{
						if (shiftReplacements[j][0] == chr)
						{
							chr = shiftReplacements[j][1];
							break;
						}
					}
				}
				if (strlen(data->text) < data->maxLength)
				{
					memmove(data->text + data->cursorPos + 1, data->text + data->cursorPos, strlen(data->text) - data->cursorPos + 1);
					data->text[data->cursorPos] = chr;
					data->cursorPos += 1;
				}
				break;
			}
		}
	}
}

void DestroyTextBox(const Control *c)
{
	TextBoxData *data = (TextBoxData *)c->ControlData;
	free(data->text);
	free(data);
}
