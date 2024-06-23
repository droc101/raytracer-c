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
#include "../Structs/Actor.h"
#include <math.h>

double EditorZoom = 20.0;
double EditorPanX = 0.0;
double EditorPanY = 0.0;
bool EditorInitComplete = false;
bool EditorSnapToGrid = true;

int EditorSelectedNode = -1;

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

typedef enum {
    NODE_WALL_A,
    NODE_WALL_B,
    NODE_ACTOR,
    NODE_PLAYER
} EditorNodeType;

typedef struct {
    EditorNodeType type;
    int index;
    Vector2 position;
    double rotation;
    uint extra; // actor type or wall texture
} EditorNode;

List *EditorNodes;

void GEditorStateUpdate() {
    if (IsKeyJustPressed(SDL_SCANCODE_F6)) {

        Level *l = CreateLevel();

        // reconstruct the level from the editor nodes
        for (int i = 0; i < EditorNodes->size; i++) {
            EditorNode *node = ListGet(EditorNodes, i);
            switch (node->type) {
                case NODE_PLAYER:
                    l->position = node->position;
                    l->rotation = node->rotation;
                    break;
                case NODE_ACTOR: {
                    Actor *a = CreateActor(node->position, node->rotation, node->extra);
                    ListAdd(l->actors, a);
                    break;
                }
                case NODE_WALL_A: {
                    Wall *w = CreateWall(node->position, vec2(0, 0), node->extra);
                    ListAdd(l->walls, w);
                    break;
                }
                case NODE_WALL_B: {
                    Wall *w = ListGet(l->walls, ListGetSize(l->walls) - 1);
                    w->b = node->position;
                    break;
                }
            }
        }
        ChangeLevel(l);

        GMainStateSet();
    }

    if (IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
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

    // check if we are hovering over a node
    for (int i = 0; i < EditorNodes->size; i++) {
        EditorNode *node = ListGet(EditorNodes, i);
        Vector2 screenPos = vec2((node->position.x * EditorZoom) + EditorPanX, (node->position.y * EditorZoom) + EditorPanY);

        bool hovered = false;
        Vector2 mousePos = GetMousePos();
        if (mousePos.x >= screenPos.x - 5 && mousePos.x <= screenPos.x + 5 &&
            mousePos.y >= screenPos.y - 5 && mousePos.y <= screenPos.y + 5) {
            hovered = true;
        }

        if (hovered && IsMouseButtonJustPressed(SDL_BUTTON_LEFT)) {
            EditorSelectedNode = i;
        }
    }

    if (IsMouseButtonJustReleased(SDL_BUTTON_LEFT)) {
        EditorSelectedNode = -1;
    }

    // move the selected node to the mouse position
    if (EditorSelectedNode != -1) {
        EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
        Vector2 mousePos = GetMousePos();
        node->position = vec2((mousePos.x - EditorPanX) / EditorZoom, (mousePos.y - EditorPanY) / EditorZoom);

        if (EditorSnapToGrid) {
            node->position.x = round(node->position.x);
            node->position.y = round(node->position.y);
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

    // Draw nodes
    for (int i = 0; i < EditorNodes->size; i++) {
        EditorNode *node = ListGet(EditorNodes, i);
        Vector2 screenPos = vec2((node->position.x * EditorZoom) + EditorPanX, (node->position.y * EditorZoom) + EditorPanY);

        if (node->type == NODE_WALL_A) {
            // draw a line to the other wall node
            EditorNode *nodeB = ListGet(EditorNodes, i + 1);
            Vector2 screenPosB = vec2((nodeB->position.x * EditorZoom) + EditorPanX, (nodeB->position.y * EditorZoom) + EditorPanY);
            setColorUint(0xFFFFFFFF);
            SDL_RenderDrawLine(GetRenderer(), screenPos.x, screenPos.y, screenPosB.x, screenPosB.y);
        }

        uint color;
        switch (node->type) {
            case NODE_PLAYER:
                color = 0xFF00FF00;
                break;
            case NODE_ACTOR:
                color = 0xFFFF0000;
                break;
            case NODE_WALL_A:
                color = 0xFF0000FF;
                break;
            case NODE_WALL_B:
                color = 0xFF0000FF;
                break;
        }

        // check if hovered
        bool hovered = false;
        Vector2 mousePos = GetMousePos();
        if (mousePos.x >= screenPos.x - 5 && mousePos.x <= screenPos.x + 5 &&
            mousePos.y >= screenPos.y - 5 && mousePos.y <= screenPos.y + 5) {
            hovered = true;
        }

        // if hovered, color is white
        if (hovered) {
            color = 0xFFFFFFFF;
        }

        // if selected, color is yellow
        if (EditorSelectedNode == i) {
            color = 0xFFFFFF00;
        }

        setColorUint(color);
        draw_rect(screenPos.x - 5, screenPos.y - 5, 10, 10);

        // for player and actor nodes, draw a line indicating rotation
        if (node->type == NODE_PLAYER || node->type == NODE_ACTOR) {
            Vector2 lineEnd = vec2(screenPos.x + (cos(node->rotation) * 20), screenPos.y + (sin(node->rotation) * 20));
            SDL_RenderDrawLine(GetRenderer(), screenPos.x, screenPos.y, lineEnd.x, lineEnd.y);
        }
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
        EditorNodes = CreateList(); // will be freed immediately after this function, but we create it here to avoid nullptr free

        // create buttons for zooming
        CreateButton("ZP", vec2(10, 50), vec2(40, 24), BtnZoomIn, true, false);
        CreateButton("ZM", vec2(10, 78), vec2(40, 24), BtnZoomOut, true, false);

        EditorInitComplete = true;
    }

    // clear all editor nodes
    ListFreeWithData(EditorNodes);
    EditorNodes = CreateList();

    Level *l = GetState()->level;

    // add a node for the player
    EditorNode *playerNode = malloc(sizeof(EditorNode));
    playerNode->type = NODE_PLAYER;
    playerNode->position = l->position;
    playerNode->rotation = l->rotation;
    ListAdd(EditorNodes, playerNode);

    // add a node for each actor
    for (int i = 0; i < ListGetSize(l->actors); i++) {
        Actor *a = ListGet(l->actors, i);
        EditorNode *actorNode = malloc(sizeof(EditorNode));
        actorNode->type = NODE_ACTOR;
        actorNode->position = a->position;
        actorNode->rotation = a->rotation;
        actorNode->index = i;
        actorNode->extra = a->actorType;
        ListAdd(EditorNodes, actorNode);
    }

    // add a node for each wall
    for (int i = 0; i < ListGetSize(l->walls); i++) {
        Wall *w = ListGet(l->walls, i);
        EditorNode *wallNodeA = malloc(sizeof(EditorNode));
        wallNodeA->type = NODE_WALL_A;
        wallNodeA->index = i;
        wallNodeA->position = w->a;
        wallNodeA->extra = w->texId;
        ListAdd(EditorNodes, wallNodeA);

        EditorNode *wallNodeB = malloc(sizeof(EditorNode));
        wallNodeB->type = NODE_WALL_B;
        wallNodeB->index = i;
        wallNodeB->position = w->b;
        ListAdd(EditorNodes, wallNodeB);
    }

    SetRenderCallback(GEditorStateRender);
    SetUpdateCallback(GEditorStateUpdate);
}

