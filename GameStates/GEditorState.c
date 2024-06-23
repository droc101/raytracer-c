//
// Created by droc101 on 6/23/2024.
//

#include "GEditorState.h"
#include "../Structs/GlobalState.h"
#include "../Helpers/Input.h"
#include "../Helpers/Drawing.h"
#include "GMainState.h"
#include "../Helpers/MathEx.h"
#include <stdio.h>
#include "../Helpers/Font.h"
#include <math.h>

double EditorZoom = 20.0;
double EditorPanX = 0.0;
double EditorPanY = 0.0;
bool EditorInitComplete = false;

typedef enum {
    EDITOR_MODE_ADD,
    EDITOR_MODE_SELECT,
    EDITOR_MODE_DELETE
} EditorMode;

EditorMode CurrentEditorMode = EDITOR_MODE_SELECT;

typedef struct {
    void *icon;
    char *text;
    void (*callback)();
    bool enabled;
    bool toggled;
    bool toggle_mode;
    Vector2 position;
    Vector2 size;
} EditorButton;

List *EditorButtons;

void GEditorStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_F6)) {
        GMainStateSet();
    }

    if (IsMouseButtonPressed(SDL_BUTTON_LEFT)) {
        Vector2 mouseDelta = GetMouseRel();
        EditorPanX += mouseDelta.x;
        EditorPanY += mouseDelta.y;
    }

    for (int i = 0; i < EditorButtons->size; i++) {
        EditorButton *button = ListGet(EditorButtons, i);
        bool hovered = false;
        bool pressed = false;
        Vector2 mousePos = GetMousePos();
        if (mousePos.x >= button->position.x && mousePos.x <= button->position.x + button->size.x &&
            mousePos.y >= button->position.y && mousePos.y <= button->position.y + button->size.y) {
            hovered = true;
        }
        pressed = IsMouseButtonJustPressed(SDL_BUTTON_LEFT) && hovered;

        if (pressed) {
            if (button->toggle_mode) {
                button->toggled = !button->toggled;
            }
            if (button->callback != NULL) {
                button->callback();
            }
        }
    }
}

void DrawEditorButton(EditorButton *btn) {
    // check if hovered
    bool hovered = false;
    bool pressed = false;
    Vector2 mousePos = GetMousePos();
    if (mousePos.x >= btn->position.x && mousePos.x <= btn->position.x + btn->size.x &&
        mousePos.y >= btn->position.y && mousePos.y <= btn->position.y + btn->size.y) {
        hovered = true;
    }
    pressed = IsMouseButtonPressed(SDL_BUTTON_RIGHT) && hovered;

    uint btnColor;
    if (btn->toggled) {
        if (pressed) {
            btnColor = 0xFFFF0000;
        } else if (hovered) {
            btnColor = 0xFF0000FF;
        } else {
            btnColor = 0xFF000080;
        }
    } else if (pressed) {
        btnColor = 0xFF00FF00;
    } else if (hovered) {
        btnColor = 0xFF8080FF;
    } else {
        btnColor = 0xFF808080;
    }

    setColorUint(btnColor);
    draw_rect(btn->position.x, btn->position.y, btn->size.x, btn->size.y);

    FontDrawString(vec2(btn->position.x + 5, btn->position.y + 5), btn->text, 16, 0xFFFFFFFF);
}

void GEditorStateRender() {
    setColorUint(0xFF123456);
    SDL_RenderClear(GetRenderer());

    // Draw a unit grid with the scale of 20px = 1 unit at 1.0 zoom
    // account for zoom and pan
    int gridSpacing = EditorZoom;
    int gridOffsetX = (int)EditorPanX % gridSpacing;
    int gridOffsetY = (int)EditorPanY % gridSpacing;

    setColorUint(0xFF808080);
    for (int x = gridOffsetX; x < WindowWidth(); x += gridSpacing) {
        draw_rect(x, 0, 1, WindowHeight());
    }
    for (int y = gridOffsetY; y < WindowHeight(); y += gridSpacing) {
        draw_rect(0, y, WindowWidth(), 1);
    }

    // draw special grid lines for 0,0
    setColorUint(0xFF0000FF);
    draw_rect((int)EditorPanX, 0, 1, WindowHeight());
    setColorUint(0xFFFF0000);
    draw_rect(0, (int)EditorPanY, WindowWidth(), 1);

    // draw world space numbers along bottom and right
    char buf[32];
    for (int x = gridOffsetX; x < WindowWidth(); x += gridSpacing) {

        int worldSpaceX = (int)((x - EditorPanX) / EditorZoom);

        if (worldSpaceX % 5 != 0) {
            continue; // only draw every 5th unit
        }
        sprintf(buf, "%d", worldSpaceX);
        FontDrawString(vec2(x, WindowHeight() - 20), buf, 16, 0xFFFFFFFF);
    }
    for (int y = gridOffsetY; y < WindowHeight(); y += gridSpacing) {
        int worldSpaceY = (int)((y - EditorPanY) / EditorZoom);

        if (worldSpaceY % 5 != 0) {
            continue; // only draw every 5th unit
        }
        sprintf(buf, "%d", worldSpaceY);
        FontDrawString(vec2(WindowWidth() - 60, y), buf, 16, 0xFFFFFFFF);
    }

    sprintf(buf, "Position: (%.2f, %.2f)\nZoom: %.1f", EditorPanX, EditorPanY, EditorZoom);
    FontDrawString(vec2(10, 10), buf, 16, 0xFFFFFFFF);

    Level *l = GetState()->level;

    for (int i = 0; i < ListGetSize(l->walls); i++) {
        Wall *w = ListGet(l->walls, i);

        setColorUint(0xFFFFFFFF);
        SDL_RenderDrawLine(GetRenderer(), w->a.x * EditorZoom + EditorPanX, w->a.y * EditorZoom + EditorPanY,
                           w->b.x * EditorZoom + EditorPanX, w->b.y * EditorZoom + EditorPanY);

        setColorUint(0xFF00FF00);
        draw_rect((w->a.x * EditorZoom + EditorPanX) - 5, (w->a.y * EditorZoom + EditorPanY) - 5, 10, 10);
        draw_rect((w->b.x * EditorZoom + EditorPanX) - 5, (w->b.y * EditorZoom + EditorPanY) - 5, 10, 10);

        // calculate the midpoint
        Vector2 mid = vec2((w->a.x + w->b.x) / 2, (w->a.y + w->b.y) / 2);

        char buf[32];
        sprintf(buf, "Wall %d", i);

        mid.x *= EditorZoom;
        mid.y *= EditorZoom;

        mid.x += EditorPanX;
        mid.y += EditorPanY;

        // offset midpoint by text size
        mid.x -= strlen(buf) * 8 / 2;
        mid.y -= 8 / 2;

        FontDrawString(vec2(mid.x, mid.y), buf, 16, 0xFFFFFFFF);
    }

    // draw the player
    Vector2 player = l->position;
    player.x *= EditorZoom;
    player.y *= EditorZoom;
    player.x += EditorPanX;
    player.y += EditorPanY;

    setColorUint(0xFFFF0000);
    draw_rect(player.x - 5, player.y - 5, 10, 10);
    // draw the player's direction
    setColorUint(0xFF00FF00);
    Vector2 playerDir = vec2(cos(l->rotation), sin(l->rotation));
    playerDir = Vector2Scale(playerDir, 20);
    Vector2 playerDirEnd = Vector2Add(player, playerDir);
    SDL_RenderDrawLine(GetRenderer(), player.x, player.y, playerDirEnd.x, playerDirEnd.y);

    // draw each actor
    for (int i = 0; i < ListGetSize(l->actors); i++) {
        Actor *a = ListGet(l->actors, i);
        Vector2 actorPos = a->position;
        actorPos.x *= EditorZoom;
        actorPos.y *= EditorZoom;
        actorPos.x += EditorPanX;
        actorPos.y += EditorPanY;

        setColorUint(0xFF00FFFF);
        draw_rect(actorPos.x - 5, actorPos.y - 5, 10, 10);

        // draw the actor's direction
        setColorUint(0xFF00FF00);
        Vector2 actorDir = vec2(cos(a->rotation), sin(a->rotation));
        actorDir = Vector2Scale(actorDir, 20);
        Vector2 actorDirEnd = Vector2Add(actorPos, actorDir);
        SDL_RenderDrawLine(GetRenderer(), actorPos.x, actorPos.y, actorDirEnd.x, actorDirEnd.y);
    }

    // Draw buttons
    for (int i = 0; i < EditorButtons->size; i++) {
        EditorButton *button = ListGet(EditorButtons, i);
        DrawEditorButton(button);
    }
}

void CreateButton(char *text, Vector2 position, Vector2 size, void (*callback)(), bool enabled, bool toggle_mode) {
    EditorButton *button = malloc(sizeof(EditorButton));
    button->text = text;
    button->position = position;
    button->size = size;
    button->callback = callback;
    button->enabled = enabled;
    button->toggled = false;
    button->toggle_mode = toggle_mode;
    ListAdd(EditorButtons, button);
}

void BtnZoomIn() {
    EditorZoom += 2.0;
    EditorZoom = clampf(EditorZoom, 10.0, 60.0);
}

void BtnZoomOut() {
    EditorZoom -= 2.0;
    EditorZoom = clampf(EditorZoom, 10.0, 60.0);
}

void GEditorStateSet() {

    if (!EditorInitComplete) {
        // center the view to 0,0
        EditorPanX = WindowWidth() / 2;
        EditorPanY = WindowHeight() / 2;

        EditorButtons = CreateList();

        // create buttons for zooming
        CreateButton("ZP", vec2(10, 50), vec2(40, 24), BtnZoomIn, true, false);
        CreateButton("ZM", vec2(10, 78), vec2(40, 24), BtnZoomOut, true, false);

        EditorInitComplete = true;
    }

    SetRenderCallback(GEditorStateRender);
    SetUpdateCallback(GEditorStateUpdate);
}

