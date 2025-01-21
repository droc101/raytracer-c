//
// Created by droc101 on 4/22/2024.
//

#include "GMainState.h"
#include <math.h>
#include <stdio.h>
#include "../Debug/DPrint.h"
#include "../Helpers/Collision.h"
#include "../Helpers/CommandParser.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/TextBox.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/Trigger.h"
#include "../Structs/Vector2.h"
#include "GPauseState.h"

void GMainStateUpdate(GlobalState *State)
{
	if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE) || IsButtonJustPressed(SDL_CONTROLLER_BUTTON_START))
	{
		PlaySoundEffect(SOUND("sfx_popup"));
		GPauseStateSet();
		return;
	}

	if (State->textBoxActive)
	{
		if (IsKeyJustPressed(SDL_SCANCODE_SPACE) || IsButtonJustPressed(CONTROLLER_OK))
		{
			State->textBoxPage++;
			if (State->textBoxPage >= StringLineCount(State->textBox.text) / State->textBox.rows)
			{
				State->textBoxActive = false;
			}
		}
		return;
	}

	if (IsKeyJustPressed(SDL_SCANCODE_C))
	{
		Error("Manually triggered error.");
	}

	if (IsKeyJustPressed(SDL_SCANCODE_T))
	{
		const TextBox tb = DEFINE_TEXT("TEXT BOX",
									   2,
									   20,
									   0,
									   60,
									   TEXT_BOX_H_ALIGN_CENTER,
									   TEXT_BOX_V_ALIGN_TOP,
									   TEXT_BOX_THEME_BLACK);
		ShowTextBox(tb);
	}

	State->level->player.angle += GetMouseRel().x * (State->options.mouseSpeed / 120.0);
}

void GMainStateFixedUpdate(GlobalState *state, double delta)
{
	if (state->textBoxActive)
	{
		return;
	}

	Level *l = state->level;
	Vector2 moveVec = v2(0, 0);
	if (UseController())
	{
		moveVec.y = GetAxis(SDL_CONTROLLER_AXIS_LEFTX);
		moveVec.x = -GetAxis(SDL_CONTROLLER_AXIS_LEFTY);
		if (fabs(moveVec.x) < 0.1)
		{
			moveVec.x = 0;
		}
		if (fabs(moveVec.y) < 0.1)
		{
			moveVec.y = 0;
		}

	} else
	{
		if (IsKeyPressed(SDL_SCANCODE_W) || GetAxis(SDL_CONTROLLER_AXIS_LEFTY) < -0.5)
		{
			moveVec.x += 1;
		} else if (IsKeyPressed(SDL_SCANCODE_S) || GetAxis(SDL_CONTROLLER_AXIS_LEFTY) > 0.5)
		{
			moveVec.x -= 1;
		}

		if (IsKeyPressed(SDL_SCANCODE_A) || GetAxis(SDL_CONTROLLER_AXIS_LEFTX) < -0.5)
		{
			moveVec.y -= 1;
		} else if (IsKeyPressed(SDL_SCANCODE_D) || GetAxis(SDL_CONTROLLER_AXIS_LEFTX) > 0.5)
		{
			moveVec.y += 1;
		}
	}


	const bool isMoving = moveVec.x != 0 || moveVec.y != 0;

	if (isMoving && !UseController())
	{
		moveVec = Vector2Normalize(moveVec);
	}


	double speed = MOVE_SPEED;
	if (IsKeyPressed(SDL_SCANCODE_LSHIFT) || GetAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 0.5)
	{
		speed = SLOW_MOVE_SPEED;
	}

	speed *= delta;

	moveVec = Vector2Scale(moveVec, speed);
	moveVec = Vector2Rotate(moveVec, l->player.angle);

	l->player.pos = Move(l->player.pos, moveVec, NULL);

	if (UseController())
	{
		double cx = GetAxis(SDL_CONTROLLER_AXIS_RIGHTX);
		if (state->options.cameraInvertX)
		{
			cx *= -1;
		}
		if (fabs(cx) > 0.1)
		{
			l->player.angle += cx * (state->options.mouseSpeed / 11.25);
		}
	}

	// view bobbing (scam edition) 💀 (it's better now trust me)
	if (speed == SLOW_MOVE_SPEED)
	{
		if (isMoving)
		{
			state->cameraY = sin(state->physicsFrame / 7.0) * 0.005 - 0.1;
		} else
		{
			state->cameraY = lerp(state->cameraY, -0.1, 0.1);
		}
	} else
	{
		if (isMoving)
		{
			state->cameraY = sin(state->physicsFrame / 7.0) * 0.04;
		} else
		{
			state->cameraY = lerp(state->cameraY, 0, 0.1);
		}
	}

	l->player.angle = wrap(l->player.angle, 0, 2 * PI);

	for (int i = 0; i < l->actors->size; i++)
	{
		Actor *a = ListGet(l->actors, i);
		a->Update(a, delta);
	}

	// Check for collisions with triggers
	for (int i = 0; i < l->triggers->size; i++)
	{
		Trigger *t = ListGet(l->triggers, i);
		if (CheckTriggerCollision(t, &l->player))
		{
			ExecuteCommand(t->command);
			break;
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GMainStateRender(GlobalState *State)
{
	const Level *l = State->level;

	RenderLevel(State);

	SDL_Rect coinIconRect = {WindowWidth() - 260, 16, 40, 40};
	DrawTexture(v2(WindowWidth() - 260, 16), v2(40, 40), TEXTURE("interface_hud_ycoin"));

	char coinStr[16];
	sprintf(coinStr, "%d", State->coins);
	FontDrawString(v2(WindowWidth() - 210, 16), coinStr, 40, 0xFFFFFFFF, false);

	coinIconRect.y = 64;

	for (int bc = 0; bc < State->blueCoins; bc++)
	{
		coinIconRect.x = WindowWidth() - 260 + bc * 48;
		DrawTexture(v2(coinIconRect.x, coinIconRect.y), v2(40, 40), TEXTURE("interface_hud_bcoin"));
	}

	if (State->textBoxActive)
	{
		TextBoxRender(&State->textBox, State->textBoxPage);
	}
	DPrintF("Position: (%.2f, %.2f)\nRotation: %.4f (%.2fdeg)",
			0xFFFFFFFF,
			false,
			l->player.pos.x,
			l->player.pos.y,
			l->player.angle,
			radToDeg(l->player.angle));

	DPrintF("Walls: %d", 0xFFFFFFFF, false, l->walls->size);
	DPrintF("Actors: %d", 0xFFFFFFFF, false, l->actors->size);
	DPrintF("Triggers: %d", 0xFFFFFFFF, false, l->triggers->size);
}

void GMainStateSet()
{
	SetStateCallbacks(GMainStateUpdate, GMainStateFixedUpdate, MAIN_STATE, GMainStateRender);
}
