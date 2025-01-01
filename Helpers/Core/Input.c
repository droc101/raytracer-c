//
// Created by droc101 on 4/20/2024.
//

#include "Input.h"

#include "../../Structs/GlobalState.h"
#include "../../Structs/Vector2.h"
#include "Logging.h"

// every key is tracked, even if it's not used
// this *could* be optimized, but it's not necessary
// on modern systems where memory is not a concern
byte keys[SDL_SCANCODE_COUNT];
byte controllerButtons[SDL_GAMEPAD_BUTTON_COUNT];

byte mouseButtons[4];

int mouseX;
int mouseY;
int mouseRelativeX;
int mouseRelativeY;

// 0 is left, 1 is right for axes (not 🪓)
Vector2 leftStick;
Vector2 rightStick;
Vector2 triggers;

SDL_Gamepad *controller;
bool ctlHaptic = false;

bool FindGameController()
{
	int stickCount;
	SDL_JoystickID *sticks = SDL_GetGamepads(&stickCount);
	if (sticks == NULL)
	{
		LogError("Failed to get gamepads: %s\n", SDL_GetError());
		return false;
	}
	LogDebug("Stick count: %d\n", stickCount);
	for (int si = 0; si < stickCount; si++)
	{
		const SDL_JoystickID i = sticks[si];
		if (SDL_IsGamepad(i))
		{
			controller = SDL_OpenGamepad(i);
			if (controller == NULL)
			{
				LogError("Failed to open gamepad %d: %s\n", i, SDL_GetError());
				SDL_free(sticks);
				return false;
			}

			SDL_PropertiesID gpp = SDL_GetGamepadProperties(controller);
			ctlHaptic = SDL_GetBooleanProperty(gpp, SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false);

			SDL_free(sticks);
			LogInfo("Using controller \"%s\"\n", SDL_GetGamepadName(controller));
			return true;
		}
		LogDebug("Stick %d is not a gamepad\n", i);

	}
	SDL_free(sticks);
	return false;
}

void Rumble(const float strength, const uint time)
{
	RumbleLR(strength, strength, time);
}

void RumbleLR(const float leftStrength, const float rightStrength, const uint time)
{
	if (UseController() && ctlHaptic)
	{
		ushort hapticStrengthLeft = (leftStrength * GetState()->options.rumbleStrength) * 0xFFFF;
		ushort hapticStrengthRight = (rightStrength * GetState()->options.rumbleStrength) * 0xFFFF;
		SDL_RumbleGamepad(controller, hapticStrengthLeft, hapticStrengthRight, time);
	}
}

void HandleControllerDisconnect(const Sint32 which)
{
	if (controller == NULL)
	{
		return;
	}
	if (SDL_GetJoystickID(SDL_GetGamepadJoystick(controller)) != which)
	{
		return;
	}
	SDL_CloseGamepad(controller);
	controller = NULL;
	FindGameController(); // try to find another controller
}

void HandleControllerConnect()
{
	if (controller)
	{
		// disconnect the current controller to use the new one
		HandleControllerDisconnect(SDL_GetJoystickID(SDL_GetGamepadJoystick(controller)));
	}
	FindGameController();
}

void HandleControllerButtonUp(const SDL_GamepadButton button)
{
	controllerButtons[button] = INP_JUST_RELEASED;
}

void HandleControllerButtonDown(const SDL_GamepadButton button)
{
	controllerButtons[button] = INP_JUST_PRESSED;
}

void HandleControllerAxis(const SDL_GamepadAxis axis, const Sint16 value)
{
	const double dValue = value / 32767.0;
	switch (axis)
	{
		case SDL_GAMEPAD_AXIS_LEFTX:
			leftStick.x = dValue;
			break;
		case SDL_GAMEPAD_AXIS_LEFTY:
			leftStick.y = dValue;
			break;
		case SDL_GAMEPAD_AXIS_RIGHTX:
			rightStick.x = dValue;
			break;
		case SDL_GAMEPAD_AXIS_RIGHTY:
			rightStick.y = dValue;
			break;
		case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
			triggers.x = dValue;
			break;
		case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
			triggers.y = dValue;
			break;
		default:
			break;
	}
}

void HandleMouseMotion(const int x, const int y, const int xRel, const int yRel)
{
	mouseX = x;
	mouseY = y;
	mouseRelativeX = xRel;
	mouseRelativeY = yRel;
}

void HandleMouseDown(const int button)
{
	mouseButtons[button] = INP_JUST_PRESSED;
}

void HandleMouseUp(const int button)
{
	mouseButtons[button] = INP_JUST_RELEASED;
}

void HandleKeyDown(const int code)
{
	keys[code] = INP_JUST_PRESSED;
}

void HandleKeyUp(const int code)
{
	keys[code] = INP_JUST_RELEASED;
}

void UpdateInputStates()
{
	for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
	{
		if (keys[i] == INP_JUST_RELEASED)
		{
			keys[i] = INP_RELEASED;
		} else if (keys[i] == INP_JUST_PRESSED)
		{
			keys[i] = INP_PRESSED;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (mouseButtons[i] == INP_JUST_RELEASED)
		{
			mouseButtons[i] = INP_RELEASED;
		} else if (mouseButtons[i] == INP_JUST_PRESSED)
		{
			mouseButtons[i] = INP_PRESSED;
		}
	}

	for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++)
	{
		if (controllerButtons[i] == INP_JUST_RELEASED)
		{
			controllerButtons[i] = INP_RELEASED;
		} else if (controllerButtons[i] == INP_JUST_PRESSED)
		{
			controllerButtons[i] = INP_PRESSED;
		}
	}

	mouseRelativeX = 0;
	mouseRelativeY = 0;
}

bool IsButtonPressed(const int button)
{
	return controllerButtons[button] == INP_PRESSED || controllerButtons[button] == INP_JUST_PRESSED;
}

bool IsButtonJustPressed(const int button)
{
	return controllerButtons[button] == INP_JUST_PRESSED;
}

bool IsButtonJustReleased(const int button)
{
	return controllerButtons[button] == INP_JUST_RELEASED;
}

bool IsKeyPressed(const int code)
{
	return keys[code] == INP_PRESSED || keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustPressed(const int code)
{
	return keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustReleased(const int code)
{
	return keys[code] == INP_JUST_RELEASED;
}

bool IsMouseButtonPressed(const int button)
{
	return mouseButtons[button] == INP_PRESSED || mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustPressed(const int button)
{
	return mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustReleased(const int button)
{
	return mouseButtons[button] == INP_JUST_RELEASED;
}

Vector2 GetMousePos()
{
	return Vector2Scale(v2(mouseX, mouseY), 1.0 / GetState()->uiScale);
}

Vector2 GetMouseRel()
{
	return v2(mouseRelativeX, mouseRelativeY);
}

void ConsumeKey(const int code)
{
	keys[code] = INP_RELEASED;
}

void ConsumeButton(const int btn)
{
	controllerButtons[btn] = INP_RELEASED;
}

void ConsumeMouseButton(const int button)
{
	mouseButtons[button] = INP_RELEASED;
}

void ConsumeAllKeys()
{
	for (int i = 0; i < SDL_SCANCODE_COUNT; i++)
	{
		keys[i] = INP_RELEASED;
	}
}

void ConsumeAllMouseButtons()
{
	for (int i = 0; i < 4; i++)
	{
		mouseButtons[i] = INP_RELEASED;
	}
}

double GetAxis(const SDL_GamepadAxis axis)
{
	switch (axis)
	{
		case SDL_GAMEPAD_AXIS_LEFTX:
			return leftStick.x;
		case SDL_GAMEPAD_AXIS_LEFTY:
			return leftStick.y;
		case SDL_GAMEPAD_AXIS_RIGHTX:
			return rightStick.x;
		case SDL_GAMEPAD_AXIS_RIGHTY:
			return rightStick.y;
		case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
			return triggers.x;
		case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
			return triggers.y;
		default:
			return 0;
	}
}

bool UseController()
{
	return GetState()->options.controllerMode && controller != NULL;
}

const char *GetControllerName()
{
	if (!UseController())
	{
		return NULL;
	}
	return SDL_GetGamepadName(controller);
}
