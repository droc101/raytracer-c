//
// Created by droc101 on 6/23/2024.
//

#include "../config.h"
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
    EDITOR_MODE_ADD, // click to add a node
    EDITOR_MODE_MOVE, // move nodes around
    EDITOR_MODE_DELETE, // delete nodes
    EDITOR_MODE_PROPERTIES, // edit node properties
    EDITOR_MODE_LEVEL // edit level properties
} EditorMode;

EditorMode CurrentEditorMode = EDITOR_MODE_MOVE;

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

bool isAddModeDragging = false;

void GEditorStateUpdate() {
#ifdef ENABLE_LEVEL_EDITOR
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
            ConsumeMouseButton(SDL_BUTTON_LEFT);
            if (button->toggle_mode) {
                button->toggled = !button->toggled;
            }
            if (button->callback != NULL) {
                button->callback(button);
            }
        }
    }

    if (CurrentEditorMode == EDITOR_MODE_MOVE) {
        // check if we are hovering over a node
        for (int i = 0; i < EditorNodes->size; i++) {
            EditorNode *node = ListGet(EditorNodes, i);
            Vector2 screenPos = vec2((node->position.x * EditorZoom) + EditorPanX,
                                     (node->position.y * EditorZoom) + EditorPanY);

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
    } else if (CurrentEditorMode == EDITOR_MODE_DELETE) {
        for (int i = 0; i < EditorNodes->size; i++) {
            EditorNode *node = ListGet(EditorNodes, i);
            Vector2 screenPos = vec2((node->position.x * EditorZoom) + EditorPanX,
                                     (node->position.y * EditorZoom) + EditorPanY);

            bool hovered = false;
            Vector2 mousePos = GetMousePos();
            if (mousePos.x >= screenPos.x - 5 && mousePos.x <= screenPos.x + 5 &&
                mousePos.y >= screenPos.y - 5 && mousePos.y <= screenPos.y + 5) {
                hovered = true;
            }

            if (hovered && IsMouseButtonJustPressed(SDL_BUTTON_LEFT)) {
                if (node->type != NODE_PLAYER) {
                    ListRemoveAt(EditorNodes, i);
                    // if we are deleting a wall, we need to delete both nodes
                    if (node->type == NODE_WALL_A) {
                        ListRemoveAt(EditorNodes, i);
                        i--;
                    } else if (node->type == NODE_WALL_B) {
                        ListRemoveAt(EditorNodes, i - 1);
                        i--;
                    }
                }
                break; // don't delete more than one node per click
            }
        }
    } else if (CurrentEditorMode == EDITOR_MODE_ADD) {
        if (IsMouseButtonJustPressed(SDL_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePos();
            Vector2 worldPos = vec2((mousePos.x - EditorPanX) / EditorZoom, (mousePos.y - EditorPanY) / EditorZoom);

            if (EditorSnapToGrid) {
                worldPos.x = round(worldPos.x);
                worldPos.y = round(worldPos.y);
            }

            // Create 2 nodes for a wall
            EditorNode *nodeA = malloc(sizeof(EditorNode));
            nodeA->type = NODE_WALL_A;
            nodeA->position = worldPos;
            nodeA->extra = 0;
            ListAdd(EditorNodes, nodeA);

            EditorNode *nodeB = malloc(sizeof(EditorNode));
            nodeB->type = NODE_WALL_B;
            nodeB->position = worldPos;
            ListAdd(EditorNodes, nodeB);

            isAddModeDragging = true;
        } else if (IsMouseButtonJustReleased(SDL_BUTTON_LEFT)) {
            isAddModeDragging = false;
        } else if (isAddModeDragging) {

            // delete the 2 nodes we just created if escape is pressed
            if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE)) {
                ListRemoveAt(EditorNodes, EditorNodes->size - 1);
                ListRemoveAt(EditorNodes, EditorNodes->size - 1);
                isAddModeDragging = false;
                return;
            }

            Vector2 mousePos = GetMousePos();
            Vector2 worldPos = vec2((mousePos.x - EditorPanX) / EditorZoom, (mousePos.y - EditorPanY) / EditorZoom);

            if (EditorSnapToGrid) {
                worldPos.x = round(worldPos.x);
                worldPos.y = round(worldPos.y);
            }

            EditorNode *nodeB = ListGet(EditorNodes, EditorNodes->size - 1);
            nodeB->position = worldPos;
        }
    }
#endif
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
            btnColor = 0xFF6ebcff;
        } else if (hovered) {
            btnColor = 0xFF7dc3ff;
        } else {
            btnColor = 0xFF8ac9ff;
        }
    } else if (pressed) {
        btnColor = 0xFF8ac9ff;
    } else if (hovered) {
        btnColor = 0xFFa1d4ff;
    } else {
        btnColor = 0xFFc2e3ff;
    }

    if (!btn->enabled) {
        btnColor = 0xFF808080;
    }

    setColorUint(btnColor);
    draw_rect(btn->position.x, btn->position.y, btn->size.x, btn->size.y);

    DrawTextAligned(btn->text, 16, 0xFFFFFFFF, btn->position, btn->size, FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE);
}

void GEditorStateRender() {
#ifdef ENABLE_LEVEL_EDITOR
    setColorUint(0xFF123456);
    SDL_RenderClear(GetRenderer());

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

    setColorUint(0xFF0000FF);
    draw_rect((int)EditorPanX, 0, 1, WindowHeight());
    setColorUint(0xFFFF0000);
    draw_rect(0, (int)EditorPanY, WindowWidth(), 1);

    // draw world space numbers along bottom and right
    char buf[32];
    for (int x = gridOffsetX; x < WindowWidth(); x += gridSpacing) {

        int worldSpaceX = (int)((x - EditorPanX) / EditorZoom);

        if (worldSpaceX % 5 != 0) {
            continue;
        }
        sprintf(buf, "%d", worldSpaceX);
        DrawTextAligned(buf, 16, 0xFFFFFFFF, vec2(x-50, WindowHeight() - 25), vec2(100, 20), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE);
    }
    for (int y = gridOffsetY; y < WindowHeight(); y += gridSpacing) {
        int worldSpaceY = (int)((y - EditorPanY) / EditorZoom);

        if (worldSpaceY % 5 != 0) {
            continue;
        }
        sprintf(buf, "%d", worldSpaceY);
        DrawTextAligned(buf, 16, 0xFFFFFFFF, vec2(WindowWidth() - 110, y - 10), vec2(100, 20), FONT_HALIGN_RIGHT, FONT_VALIGN_MIDDLE);
    }

    double worldSpaceX = (WindowWidth() / 2 - EditorPanX) / EditorZoom;
    double worldSpaceY = (WindowHeight() / 2 - EditorPanY) / EditorZoom;
    sprintf(buf, "Position: (%.2f, %.2f)", worldSpaceX, worldSpaceY);
    DrawTextAligned(buf, 16, 0xFFFFFFFF, vec2(560, 10), vec2(200, 24), FONT_HALIGN_LEFT, FONT_VALIGN_MIDDLE);

    // Draw nodes
    int hoveredNode = -1;
    for (int i = 0; i < EditorNodes->size; i++) {
        EditorNode *node = ListGet(EditorNodes, i);
        Vector2 screenPos = vec2((node->position.x * EditorZoom) + EditorPanX, (node->position.y * EditorZoom) + EditorPanY);

        if (node->type == NODE_WALL_A) { // Draw a line to the next node, which should be the wall's other end
            EditorNode *nodeB = ListGet(EditorNodes, i + 1);
            Vector2 screenPosB = vec2((nodeB->position.x * EditorZoom) + EditorPanX, (nodeB->position.y * EditorZoom) + EditorPanY);
            setColorUint(0xFFFFFFFF);
            SDL_RenderDrawLine(GetRenderer(), screenPos.x, screenPos.y, screenPosB.x, screenPosB.y);
        }

        uint color = 0;
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

        bool hovered = false;
        Vector2 mousePos = GetMousePos();
        if (mousePos.x >= screenPos.x - 5 && mousePos.x <= screenPos.x + 5 &&
            mousePos.y >= screenPos.y - 5 && mousePos.y <= screenPos.y + 5) {
            hovered = true;
            hoveredNode = i;
        }

        if (EditorSelectedNode == i) {
            color = 0xFFFFFF00;
        }

        if (hovered || EditorSelectedNode == i) {
            setColorUint(0xFFFFFFFF);
            draw_rect(screenPos.x - 6, screenPos.y - 6, 12, 12);
        }

        setColorUint(color);
        draw_rect(screenPos.x - 5, screenPos.y - 5, 10, 10);

        // for player and actor nodes, draw a line indicating rotation
        if (node->type == NODE_PLAYER || node->type == NODE_ACTOR) {
            Vector2 lineEnd = vec2(screenPos.x + (cos(node->rotation) * 20), screenPos.y + (sin(node->rotation) * 20));
            SDL_RenderDrawLine(GetRenderer(), screenPos.x, screenPos.y, lineEnd.x, lineEnd.y);
        }
    }

    if (hoveredNode != -1) {
        EditorNode *node = ListGet(EditorNodes, hoveredNode);
        Vector2 screenPos = vec2((node->position.x * EditorZoom) + EditorPanX, (node->position.y * EditorZoom) + EditorPanY);

        char nodeInfo[96];
        switch (node->type) {
            case NODE_PLAYER:
                sprintf(nodeInfo, "Player: %.2f, %.2f\nRotation: %.2f", node->position.x, node->position.y, radToDeg(node->rotation));
                break;
            case NODE_ACTOR:
                sprintf(nodeInfo, "Actor: 0x%04x\nPosition: %.2f, %.2f\nRotation: %.2f", node->extra, node->position.x, node->position.y, radToDeg(node->rotation));
                break;
            case NODE_WALL_A:
                sprintf(nodeInfo, "Wall (A): %.2f, %.2f\nTexture: 0x%04x", node->position.x, node->position.y, node->extra);
                break;
            case NODE_WALL_B:
                sprintf(nodeInfo, "Wall (B): %.2f, %.2f", node->position.x, node->position.y);
                break;
        }

        Vector2 measuredText = MeasureText(nodeInfo, 16);
        int textWidth = measuredText.x;
        int textHeight = measuredText.y;
        SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);
        setColorUint(0x80000000);
        draw_rect(screenPos.x + 10, screenPos.y, textWidth + 20, textHeight + 20);
        SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_NONE);
        FontDrawString(vec2(screenPos.x + 20, screenPos.y + 10), nodeInfo, 16, 0xFFFFFFFF);
    }

    // Draw buttons
    for (int i = 0; i < EditorButtons->size; i++) {
        EditorButton *button = ListGet(EditorButtons, i);
        DrawEditorButton(button);
    }
#endif
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

void BtnZoomIn(EditorButton *btn) {
    EditorZoom += 2.0;
    EditorZoom = clampf(EditorZoom, 10.0, 60.0);
}

void BtnZoomOut(EditorButton *btn) {
    EditorZoom -= 2.0;
    EditorZoom = clampf(EditorZoom, 10.0, 60.0);
}

void BtnZoomReset(EditorButton *btn) {
    EditorZoom = 20.0;
}

void ToggleSnapToGrid(EditorButton *btn) {
    EditorSnapToGrid = btn->toggled;
}

void SetEditorMode(EditorButton *btn) {
    for (int i = 0; i < EditorButtons->size; i++) {
        EditorButton *button = ListGet(EditorButtons, i);
        button->toggled = false;
    }
    btn->toggled = true;

    if (strcmp(btn->text, "Add") == 0) {
        CurrentEditorMode = EDITOR_MODE_ADD;
    } else if (strcmp(btn->text, "Move") == 0) {
        CurrentEditorMode = EDITOR_MODE_MOVE;
    } else if (strcmp(btn->text, "Delete") == 0) {
        CurrentEditorMode = EDITOR_MODE_DELETE;
    } else if (strcmp(btn->text, "Prop") == 0) {
        CurrentEditorMode = EDITOR_MODE_PROPERTIES;
    } else if (strcmp(btn->text, "Level") == 0) {
        CurrentEditorMode = EDITOR_MODE_LEVEL;
    }
}

void GEditorStateSet() {
    if (!EditorInitComplete) {
        // center the view to 0,0
        EditorPanX = WindowWidth() / 2;
        EditorPanY = WindowHeight() / 2;

        EditorButtons = CreateList();
        EditorNodes = CreateList(); // will be freed immediately after this function, but we create it here to avoid nullptr free

        // create buttons for zooming
        CreateButton("+", vec2(10, 50), vec2(40, 24), BtnZoomIn, true, false);
        CreateButton("-", vec2(10, 78), vec2(40, 24), BtnZoomOut, true, false);
        CreateButton("0", vec2(10, 106), vec2(40, 24), BtnZoomReset, true, false);

        // Create buttons for editor modes horizontal along the top size 100x24
        CreateButton("Add", vec2(10, 10), vec2(100, 24), SetEditorMode, true, true);
        CreateButton("Move", vec2(120, 10), vec2(100, 24), SetEditorMode, true, true);
        CreateButton("Delete", vec2(230, 10), vec2(100, 24), SetEditorMode, true, true);
        CreateButton("Prop", vec2(340, 10), vec2(100, 24), SetEditorMode, true, true);
        CreateButton("Level", vec2(450, 10), vec2(100, 24), SetEditorMode, true, true);
        // Set the move button to toggled
        EditorButton *moveButton = ListGet(EditorButtons, 4);
        moveButton->toggled = true;

        // Create a snap to grid button below the zoom buttons with some space
        CreateButton("Snap", vec2(10, 154), vec2(40, 24), ToggleSnapToGrid, true, true);
        // set snap button to toggled
        EditorButton *snapButton = ListGet(EditorButtons, EditorButtons->size - 1);
        snapButton->toggled = EditorSnapToGrid;

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

