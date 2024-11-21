//
// Created by droc101 on 4/20/2024.
//

#include "Input.h"
#include "../../Structs/GlobalState.h"
#include "../../Structs/Vector2.h"

// every key is tracked, even if it's not used
// this *could* be optimized, but it's not necessary
// on modern systems where memory is not a concern
byte keys[SDL_NUM_SCANCODES];
byte controllerButtons[SDL_CONTROLLER_BUTTON_MAX];

byte mouseButtons[4];

int mouseX, mouseY, mouseXrel, mouseYrel;

Vector2 leftStick, rightStick, triggers; // 0 is left, 1 is right

SDL_GameController *controller;

SDL_GameController *FindGameController() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }
    return NULL;
}

void HandleControllerDisconnect(const Sint32 which)
{
    if (controller == NULL) return;
    if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)) != which) return;
    SDL_GameControllerClose(controller);
    controller = NULL;
    FindGameController(); // try to find another controller
}

void HandleControllerConnect()
{
    if (controller) return;
    controller = FindGameController();
}

void HandleControllerButtonUp(const SDL_GameControllerButton button)
{
    controllerButtons[button] = INP_JUST_RELEASED;
}

void HandleControllerButtonDown(const SDL_GameControllerButton button)
{
    controllerButtons[button] = INP_JUST_PRESSED;
}

void HandleControllerAxis(const SDL_GameControllerAxis axis, const Sint16 value)
{
    const double dValue = value / 32767.0;
    switch (axis)
    {
        case SDL_CONTROLLER_AXIS_LEFTX:
            leftStick.x = dValue;
            break;
        case SDL_CONTROLLER_AXIS_LEFTY:
            leftStick.y = dValue;
            break;
        case SDL_CONTROLLER_AXIS_RIGHTX:
            rightStick.x = dValue;
            break;
        case SDL_CONTROLLER_AXIS_RIGHTY:
            rightStick.y = dValue;
            break;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            triggers.x = dValue;
            break;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            triggers.y = dValue;
            break;
        default:
            break;
    }
}

void HandleMouseMotion(const int x, const int y, const int xrel, const int yrel)
{
    mouseX = x;
    mouseY = y;
    mouseXrel = xrel;
    mouseYrel = yrel;
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
    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
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

    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
    {
        if (controllerButtons[i] == INP_JUST_RELEASED)
        {
            controllerButtons[i] = INP_RELEASED;
        } else if (controllerButtons[i] == INP_JUST_PRESSED)
        {
            controllerButtons[i] = INP_PRESSED;
        }
    }

    mouseXrel = 0;
    mouseYrel = 0;
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
    return v2(mouseXrel, mouseYrel);
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
    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
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

double GetAxis(const SDL_GameControllerAxis axis)
{
    switch (axis)
    {
        case SDL_CONTROLLER_AXIS_LEFTX:
            return leftStick.x;
        case SDL_CONTROLLER_AXIS_LEFTY:
            return leftStick.y;
        case SDL_CONTROLLER_AXIS_RIGHTX:
            return rightStick.x;
        case SDL_CONTROLLER_AXIS_RIGHTY:
            return rightStick.y;
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
            return triggers.x;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            return triggers.y;
        default:
            return 0;
    }
}

bool UseController()
{
    return GetState()->options.controllerMode && controller != NULL;
}
