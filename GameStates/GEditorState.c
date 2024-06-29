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
#include "../Helpers/LevelLoader.h"

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

typedef struct {
    char *label;
    double min;
    double max;
    double value;
    double step;
    double altStep; // step when shift is held
    Vector2 position;
    Vector2 size;
    void (*callback)(double value);
} EditorSlider;

List *EditorSliders;

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
    uint extra2; // wall uv (float) or actor params (4 bytes)
} EditorNode;

List *EditorNodes;

bool isAddModeDragging = false;

byte level_fogR;
byte level_fogG;
byte level_fogB;

double level_fogStart;
double level_fogEnd;

byte level_floorR;
byte level_floorG;
byte level_floorB;

byte level_skyR;
byte level_skyG;
byte level_skyB;

Level* NodesToLevel() {
    Level *l = CreateLevel();

    l->FogColor = 0xFF << 24 | level_fogR << 16 | level_fogG << 8 | level_fogB;
    l->FogStart = level_fogStart;
    l->FogEnd = level_fogEnd;

    l->FloorColor = 0xFF << 24 | level_floorR << 16 | level_floorG << 8 | level_floorB;
    l->SkyColor = 0xFF << 24 | level_skyR << 16 | level_skyG << 8 | level_skyB;

    // reconstruct the level from the editor nodes
    for (int i = 0; i < EditorNodes->size; i++) {
        EditorNode *node = ListGet(EditorNodes, i);
        switch (node->type) {
            case NODE_PLAYER:
                l->position = node->position;
                l->rotation = node->rotation;
                break;
            case NODE_ACTOR: {
                byte paramA = (node->extra2 >> 24) & 0xFF;
                byte paramB = (node->extra2 >> 16) & 0xFF;
                byte paramC = (node->extra2 >> 8) & 0xFF;
                byte paramD = node->extra2 & 0xFF;
                Actor *a = CreateActor(node->position, node->rotation, node->extra, paramA, paramB, paramC, paramD);
                ListAdd(l->actors, a);
                break;
            }
            case NODE_WALL_A: {
                Wall *w = CreateWall(node->position, vec2(0, 0), node->extra, node->extra2);
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

    return l;
}

void CreateSlider(char *label, double min, double max, double value, double step, double altStep, Vector2 position, Vector2 size, void (*callback)(double value)){
    EditorSlider *slider = malloc(sizeof(EditorSlider));
    slider->label = label;
    slider->min = min;
    slider->max = max;
    slider->value = value;
    slider->step = step;
    slider->altStep = altStep;
    slider->position = position;
    slider->size = size;
    slider->size.y = 48; // 24px for the label, 24px for the slider
    slider->callback = callback;
    ListAdd(EditorSliders, slider);
}

void slider_setNodeRotation(double value) {
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->rotation = degToRad(value);
}

void slider_setNodeExtra(double value) {
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra = (uint)value;
}

void slider_setFogR(double value) {
    level_fogR = (byte)value;
}

void slider_setFogG(double value) {
    level_fogG = (byte)value;
}

void slider_setFogB(double value) {
    level_fogB = (byte)value;
}

void slider_setFogStart(double value) {
    level_fogStart = value;
}

void slider_setFogEnd(double value) {
    level_fogEnd = value;
}

void slider_setFloorR(double value) {
    level_floorR = (byte)value;
}

void slider_setFloorG(double value) {
    level_floorG = (byte)value;
}

void slider_setFloorB(double value) {
    level_floorB = (byte)value;
}

void slider_setSkyR(double value) {
    level_skyR = (byte)value;
}

void slider_setSkyG(double value) {
    level_skyG = (byte)value;
}

void slider_setSkyB(double value) {
    level_skyB = (byte)value;
}

void slider_setWallUv(double value) {
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra2 = value;
}

void slider_setActorParamA(double value) {
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra2 = (node->extra2 & 0x00FFFFFF) | ((byte)value << 24);
}

void slider_setActorParamB(double value) {
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra2 = (node->extra2 & 0xFF00FFFF) | ((byte)value << 16);
}

void slider_setActorParamC(double value) {
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra2 = (node->extra2 & 0xFFFF00FF) | ((byte)value << 8);
}

void slider_setActorParamD(double value) {
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra2 = (node->extra2 & 0xFFFFFF00) | (byte)value;
}

void GEditorStateUpdate() {
#ifdef ENABLE_LEVEL_EDITOR
    if (IsKeyJustPressed(SDL_SCANCODE_F6)) {
        Level *l = NodesToLevel();
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

    for (int i = 0; i < EditorSliders->size; i++) {
        EditorSlider *slider = ListGet(EditorSliders, i);
        bool hovered = false;
        bool pressed = false;
        bool rpressed = false;
        Vector2 mousePos = GetMousePos();
        if (mousePos.x >= slider->position.x && mousePos.x <= slider->position.x + slider->size.x &&
            mousePos.y >= slider->position.y && mousePos.y <= slider->position.y + slider->size.y) {
            hovered = true;
        }
        pressed = IsMouseButtonPressed(SDL_BUTTON_LEFT) && hovered;
        rpressed = IsMouseButtonJustPressed(SDL_BUTTON_RIGHT) && hovered;

        if (rpressed) {
            // increment or decrement the slider value by 1 depending on which side of the slider box the mouse is on (go from midpoint, not knob)
            double localMouseX = mousePos.x - slider->position.x;
            localMouseX /= slider->size.x;
            localMouseX -= 0.5;
            if (localMouseX < 0) {
                slider->value -= min(1, slider->step);
                slider->value = max(slider->min, slider->value);
                if (slider->callback != NULL) {
                    slider->callback(slider->value);
                }
                return;
            } else {
                slider->value += max(1, slider->step);
                slider->value = min(slider->max, slider->value);
                if (slider->callback != NULL) {
                    slider->callback(slider->value);
                }
                return;
            }
        } else if (pressed) {
            double knobPos = remap(slider->value, slider->min, slider->max, 0, slider->size.x);
            double newValue = remap(mousePos.x - slider->position.x, 0, slider->size.x, slider->min, slider->max);


            if (IsKeyPressed(SDL_SCANCODE_LSHIFT) || IsKeyPressed(SDL_SCANCODE_RSHIFT)) {
                newValue = round(newValue / slider->altStep) * slider->altStep;
            } else {
                newValue = round(newValue / slider->step) * slider->step;
            }

            if (newValue < slider->min) {
                newValue = slider->min;
            } else if (newValue > slider->max) {
                newValue = slider->max;
            }

            slider->value = newValue;
            if (slider->callback != NULL) {
                slider->callback(newValue);
            }

            return; // don't allow anything else to be clicked while dragging a slider
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
        EditorSlider *modeSld = ListGet(EditorSliders, 0);
        int mode = modeSld->value;
        switch (mode) {
            case 0: {
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
                    float uv = 1.0;
                    nodeA->extra2 = uv;
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
                break;
            }
            case 1: {
                if (IsMouseButtonJustPressed(SDL_BUTTON_LEFT)) {
                    EditorNode *node = malloc(sizeof(EditorNode));
                    node->type = NODE_ACTOR;
                    Vector2 mousePos = GetMousePos();
                    Vector2 worldPos = vec2((mousePos.x - EditorPanX) / EditorZoom, (mousePos.y - EditorPanY) / EditorZoom);

                    if (EditorSnapToGrid) {
                        worldPos.x = round(worldPos.x);
                        worldPos.y = round(worldPos.y);
                    }

                    node->position = worldPos;
                    node->rotation = 0;
                    node->extra = 1;
                    node->extra2 = 0;
                    ListAdd(EditorNodes, node);
                }
                break;
            }
        }

    } else if (CurrentEditorMode == EDITOR_MODE_PROPERTIES) {
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

        ListClear(EditorSliders);

        EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
        switch (node->type) {
            case NODE_PLAYER:
                CreateSlider("ang", 0, 359, radToDeg(node->rotation), 1, 45, vec2(10, 250), vec2(200, 24), slider_setNodeRotation);
                break;
            case NODE_ACTOR:
                CreateSlider("ang", 0, 359, radToDeg(node->rotation), 1, 45, vec2(10, 250), vec2(200, 24), slider_setNodeRotation);
                CreateSlider("Type", 0, GetActorTypeCount() - 1, node->extra, 1, 16, vec2(10, 300), vec2(200, 24), slider_setNodeExtra);
                CreateSlider("Param A", 0, 255, (node->extra2 >> 24) & 0xFF, 1, 16, vec2(10, 350), vec2(200, 24), slider_setActorParamA);
                CreateSlider("Param B", 0, 255, (node->extra2 >> 16) & 0xFF, 1, 16, vec2(10, 400), vec2(200, 24), slider_setActorParamB);
                CreateSlider("Param C", 0, 255, (node->extra2 >> 8) & 0xFF, 1, 16, vec2(10, 450), vec2(200, 24), slider_setActorParamC);
                CreateSlider("Param D", 0, 255, node->extra2 & 0xFF, 1, 16, vec2(10, 500), vec2(200, 24), slider_setActorParamD);
                break;
            case NODE_WALL_A:
                CreateSlider("Tex", 0, GetTextureCount() - 1, node->extra, 1, 16, vec2(10, 250), vec2(200, 24), slider_setNodeExtra);
                break;
            default:
                break;
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

void DrawEditorSlider(EditorSlider *sld) {
    SDL_SetRenderDrawBlendMode(GetRenderer(), SDL_BLENDMODE_BLEND);

    setColorUint(0x80000000);
    draw_rect(sld->position.x, sld->position.y, sld->size.x, sld->size.y);

    setColorUint(0xFF808080);
    draw_rect(sld->position.x, sld->position.y + 32, sld->size.x, 2);

    bool hovered = false;
    bool pressed = false;
    Vector2 mousePos = GetMousePos();
    if (mousePos.x >= sld->position.x && mousePos.x <= sld->position.x + sld->size.x &&
        mousePos.y >= sld->position.y && mousePos.y <= sld->position.y + sld->size.y) {
        hovered = true;
    }
    pressed = IsMouseButtonPressed(SDL_BUTTON_LEFT) && hovered;

    uint btnColor;
    if (pressed) {
        btnColor = 0xFF8ac9ff;
    } else if (hovered) {
        btnColor = 0xFFa1d4ff;
    } else {
        btnColor = 0xFFc2e3ff;
    }

    double knobPos = remap(sld->value, sld->min, sld->max, 0, sld->size.x);
    setColorUint(btnColor);
    draw_rect(sld->position.x + knobPos - 8, sld->position.y + 24, 16, 16);

    char buf[32];
    if (sld->step >= 1 && sld->altStep > 1) {
        sprintf(buf, "%s: %.0f", sld->label, sld->value);
    } else {
        sprintf(buf, "%s: %.2f", sld->label, sld->value);
    }

    DrawTextAligned(buf, 16, 0xFFFFFFFF, vec2(sld->position.x, sld->position.y), vec2(sld->size.x, 24), FONT_HALIGN_CENTER, FONT_VALIGN_MIDDLE);
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

    // Draw sliders
    for (int i = 0; i < EditorSliders->size; i++) {
        EditorSlider *slider = ListGet(EditorSliders, i);
        DrawEditorSlider(slider);
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

void BtnCopyBytecode() {
    LevelBytecode *bc = GenerateBytecode(GetState()->level);
    char *buf = malloc(bc->size * 2 + 1);
    for (int i = 0; i < bc->size; i++) {
        sprintf(buf + i * 2, "%02x", bc->data[i]);
    }
    buf[bc->size * 2] = '\0';

    SDL_SetClipboardText(buf);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Bytecode copied", "The level bytecode has been copied to the clipboard.", NULL);

    free(bc->data);
    free(bc);
    free(buf);
}

void ToggleSnapToGrid(EditorButton *btn) {
    EditorSnapToGrid = btn->toggled;
}

void SetEditorMode(EditorButton *btn) {
    for (int i = 0; i < 5; i++) {
        EditorButton *button = ListGet(EditorButtons, i);
        button->toggled = false;
    }
    btn->toggled = true;

    ListClear(EditorSliders);

    if (strcmp(btn->text, "Add") == 0) {
        CurrentEditorMode = EDITOR_MODE_ADD;
        CreateSlider("Add Actor?", 0, 1, 0, 1, 1, vec2(10, 250), vec2(200, 24), NULL);
    } else if (strcmp(btn->text, "Move") == 0) {
        EditorSelectedNode = -1;
        CurrentEditorMode = EDITOR_MODE_MOVE;
    } else if (strcmp(btn->text, "Delete") == 0) {
        CurrentEditorMode = EDITOR_MODE_DELETE;
    } else if (strcmp(btn->text, "Prop") == 0) {
        CurrentEditorMode = EDITOR_MODE_PROPERTIES;
        EditorSelectedNode = 0;
    } else if (strcmp(btn->text, "Level") == 0) {
        CurrentEditorMode = EDITOR_MODE_LEVEL;
        CreateSlider("Fog R", 0, 255, level_fogR, 1, 16, vec2(10, 250), vec2(200, 24), slider_setFogR);
        CreateSlider("Fog G", 0, 255, level_fogG, 1, 16, vec2(10, 300), vec2(200, 24), slider_setFogG);
        CreateSlider("Fog B", 0, 255, level_fogB, 1, 16, vec2(10, 350), vec2(200, 24), slider_setFogB);
        CreateSlider("Fog Start", -50, 200, level_fogStart, 1, 50, vec2(10, 400), vec2(200, 24), slider_setFogStart);
        CreateSlider("Fog End", 0, 300, level_fogEnd, 1, 50, vec2(10, 450), vec2(200, 24), slider_setFogEnd);
        CreateSlider("Floor R", 0, 255, level_floorR, 1, 16, vec2(10, 500), vec2(200, 24), slider_setFloorR);
        CreateSlider("Floor G", 0, 255, level_floorG, 1, 16, vec2(10, 550), vec2(200, 24), slider_setFloorG);
        CreateSlider("Floor B", 0, 255, level_floorB, 1, 16, vec2(10, 600), vec2(200, 24), slider_setFloorB);
        CreateSlider("Sky R", 0, 255, level_skyR, 1, 16, vec2(10, 650), vec2(200, 24), slider_setSkyR);
        CreateSlider("Sky G", 0, 255, level_skyG, 1, 16, vec2(10, 700), vec2(200, 24), slider_setSkyG);
        CreateSlider("Sky B", 0, 255, level_skyB, 1, 16, vec2(10, 750), vec2(200, 24), slider_setSkyB);
    }
}

void GEditorStateSet() {
    if (!EditorInitComplete) {
        // center the view to 0,0
        EditorPanX = WindowWidth() / 2;
        EditorPanY = WindowHeight() / 2;

        EditorButtons = CreateList();
        EditorSliders = CreateList();
        EditorNodes = CreateList(); // will be freed immediately after this function, but we create it here to avoid nullptr free

        CreateButton("Add", vec2(10, 10), vec2(100, 24), SetEditorMode, true, true);
        CreateButton("Move", vec2(120, 10), vec2(100, 24), SetEditorMode, true, true);
        CreateButton("Delete", vec2(230, 10), vec2(100, 24), SetEditorMode, true, true);
        CreateButton("Prop", vec2(340, 10), vec2(100, 24), SetEditorMode, true, true);
        CreateButton("Level", vec2(450, 10), vec2(100, 24), SetEditorMode, true, true);

        CreateButton("+", vec2(10, 50), vec2(80, 24), BtnZoomIn, true, false);
        CreateButton("-", vec2(10, 78), vec2(80, 24), BtnZoomOut, true, false);
        CreateButton("0", vec2(10, 106), vec2(80, 24), BtnZoomReset, true, false);

        EditorButton *moveButton = ListGet(EditorButtons, 4);
        moveButton->toggled = true;

        CreateButton("Snap", vec2(10, 154), vec2(80, 24), ToggleSnapToGrid, true, true);
        EditorButton *snapButton = ListGet(EditorButtons, EditorButtons->size - 1);
        snapButton->toggled = EditorSnapToGrid;

        CreateButton("Build", vec2(10, 182), vec2(80, 24), BtnCopyBytecode, true, false);

        SetEditorMode(ListGet(EditorButtons, 1));

        EditorInitComplete = true;
    }

    // clear all editor nodes
    ListFreeWithData(EditorNodes);
    EditorNodes = CreateList();

    Level *l = GetState()->level;

    // load level properties
    level_fogR = l->FogColor >> 16;
    level_fogG = l->FogColor >> 8;
    level_fogB = l->FogColor;
    level_fogStart = l->FogStart;
    level_fogEnd = l->FogEnd;
    level_floorR = l->FloorColor >> 16;
    level_floorG = l->FloorColor >> 8;
    level_floorB = l->FloorColor;
    level_skyR = l->SkyColor >> 16;
    level_skyG = l->SkyColor >> 8;
    level_skyB = l->SkyColor;

    // add a node for the player
    EditorNode *playerNode = malloc(sizeof(EditorNode));
    playerNode->type = NODE_PLAYER;
    playerNode->position = l->position;
    playerNode->rotation = fmod(l->rotation, 2*PI);
    ListAdd(EditorNodes, playerNode);

    // add a node for each actor
    for (int i = 0; i < ListGetSize(l->actors); i++) {
        Actor *a = ListGet(l->actors, i);
        EditorNode *actorNode = malloc(sizeof(EditorNode));
        actorNode->type = NODE_ACTOR;
        actorNode->position = a->position;
        actorNode->rotation = fmod(a->rotation, 2*PI);
        actorNode->index = i;
        actorNode->extra = a->actorType;
        uint extra2 = 0;
        extra2 |= (a->paramA & 0xFF) << 24;
        extra2 |= (a->paramB & 0xFF) << 16;
        extra2 |= (a->paramC & 0xFF) << 8;
        extra2 |= (a->paramD & 0xFF);
        actorNode->extra2 = extra2;
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
        wallNodeA->extra2 = w->uvScale;
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

