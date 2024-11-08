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
    return NULLPTR;
}

void HandleControlerDisconnect(Sint32 which)
{
    if (controller == NULLPTR) return;
    if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)) != which) return;
    SDL_GameControllerClose(controller);
    controller = NULLPTR;
    FindGameController(); // try to find another controller
}

void HandleControllerConnect()
{
    if (controller) return;
    controller = FindGameController();
}

void HandleControllerButtonUp(SDL_GameControllerButton button)
{
    controllerButtons[button] = INP_JUST_RELEASED;
}

void HandleControllerButtonDown(SDL_GameControllerButton button)
{
    controllerButtons[button] = INP_JUST_PRESSED;
}

void HandleControllerAxis(SDL_GameControllerAxis axis, Sint16 value)
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

void HandleMouseMotion(int x, int y, int xrel, int yrel)
{
    mouseX = x;
    mouseY = y;
    mouseXrel = xrel;
    mouseYrel = yrel;
}

void HandleMouseDown(int button)
{
    mouseButtons[button] = INP_JUST_PRESSED;
}

void HandleMouseUp(int button)
{
    mouseButtons[button] = INP_JUST_RELEASED;
}

void HandleKeyDown(int code)
{
    keys[code] = INP_JUST_PRESSED;
}

void HandleKeyUp(int code)
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

bool IsButtonPressed(int button)
{
    return controllerButtons[button] == INP_PRESSED || controllerButtons[button] == INP_JUST_PRESSED;
}

bool IsButtonJustPressed(int button)
{
    return controllerButtons[button] == INP_JUST_PRESSED;
}

bool IsButtonJustReleased(int button)
{
    return controllerButtons[button] == INP_JUST_RELEASED;
}

bool IsKeyPressed(int code)
{
    return keys[code] == INP_PRESSED || keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustPressed(int code)
{
    return keys[code] == INP_JUST_PRESSED;
}

bool IsKeyJustReleased(int code)
{
    return keys[code] == INP_JUST_RELEASED;
}

bool IsMouseButtonPressed(int button)
{
    return mouseButtons[button] == INP_PRESSED || mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustPressed(int button)
{
    return mouseButtons[button] == INP_JUST_PRESSED;
}

bool IsMouseButtonJustReleased(int button)
{
    return mouseButtons[button] == INP_JUST_RELEASED;
}

Vector2 GetMousePos()
{
    return Vector2Scale(v2(mouseX, mouseY), 1.0 / GetState()->options.uiScale);
}

Vector2 GetMouseRel()
{
    return v2(mouseXrel, mouseYrel);
}

void ConsumeKey(int code)
{
    keys[code] = INP_RELEASED;
}

void ConsumeButton(int btn)
{
    controllerButtons[btn] = INP_RELEASED;
}

void ConsumeMouseButton(int button)
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

double GetAxis(SDL_GameControllerAxis axis)
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
    return GetState()->options.controllerMode && controller != NULLPTR;
}
