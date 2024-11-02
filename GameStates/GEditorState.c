//
// Created by droc101 on 6/23/2024.
//

#include "../config.h"
#include "GEditorState.h"
#include "../Structs/GlobalState.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "GMainState.h"
#include "../Helpers/Core/MathEx.h"
#include <stdio.h>
#include "../Helpers/Graphics/Font.h"
#include "../Structs/Actor.h"
#include <math.h>
#include "../Helpers/LevelLoader.h"
#include "../Helpers/CommonAssets.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/Controls/Slider.h"
#include "../Structs/UI/Controls/RadioButton.h"
#include "../Structs/UI/Controls/CheckBox.h"

double EditorZoom = 20.0;
double EditorPanX = 0.0;
double EditorPanY = 0.0;
bool EditorInitComplete = false;
bool EditorSnapToGrid = true;

int EditorSelectedNode = -1;

int EditorBaseControlCount = -1;

typedef enum
{
    EDITOR_MODE_ADD, // click to add a node
    EDITOR_MODE_MOVE, // move nodes around
    EDITOR_MODE_DELETE, // delete nodes
    EDITOR_MODE_PROPERTIES, // edit node properties
    EDITOR_MODE_LEVEL // edit level properties
} EditorMode;

EditorMode CurrentEditorMode = EDITOR_MODE_MOVE;

UiStack *editorUiStack;

typedef enum
{
    NODE_WALL_A,
    NODE_WALL_B,
    NODE_ACTOR,
    NODE_PLAYER
} EditorNodeType;

typedef struct EditorNode
{
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

uint level_floor_tex;
byte level_ceil_tex;

byte level_skyR;
byte level_skyG;
byte level_skyB;

uint musicId;

Level *NodesToLevel()
{
    Level *l = CreateLevel();

    l->FogColor = 0xFF << 24 | level_fogR << 16 | level_fogG << 8 | level_fogB;
    l->FogStart = level_fogStart;
    l->FogEnd = level_fogEnd;

    l->FloorTexture = level_floor_tex;
    l->CeilingTexture = level_ceil_tex;
    l->SkyColor = 0xFF << 24 | level_skyR << 16 | level_skyG << 8 | level_skyB;

    l->MusicID = musicId;

    // reconstruct the level from the editor nodes
    for (int i = 0; i < EditorNodes->size; i++)
    {
        const EditorNode *node = ListGet(EditorNodes, i);
        switch (node->type)
        {
            case NODE_PLAYER:
                l->position = node->position;
                l->rotation = node->rotation;
                break;
            case NODE_ACTOR:
            {
                const byte paramA = (node->extra2 >> 24) & 0xFF;
                const byte paramB = (node->extra2 >> 16) & 0xFF;
                const byte paramC = (node->extra2 >> 8) & 0xFF;
                const byte paramD = node->extra2 & 0xFF;
                Actor *a = CreateActor(node->position, node->rotation, node->extra, paramA, paramB, paramC, paramD);
                ListAdd(l->actors, a);
                break;
            }
            case NODE_WALL_A:
            {
                Wall *w = CreateWall(node->position, v2(0, 0), wallTextures[node->extra], node->extra2, 1.0);
                ListAdd(l->walls, w);
                break;
            }
            case NODE_WALL_B:
            {
                Wall *w = ListGet(l->walls, ListGetSize(l->walls) - 1);
                w->b = node->position;
                break;
            }
        }
    }

    return l;
}

void CreateSlider(char *label, const double min, const double max, const double value, const double step, const double altStep, const Vector2 position,
                  const Vector2 size, void (*callback)(double value), char *(*getLabel)(Control *slider))
{
    Control *slider = CreateSliderControl(position, size, label, callback, TOP_LEFT, min, max, value, step, altStep,
                                          getLabel);
    UiStackPush(editorUiStack, slider);
}

void slider_setMusic(const double value)
{
    musicId = (byte) value;
}

void slider_setNodeRotation(const double value)
{
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->rotation = degToRad(value);
}

void slider_setNodeExtra(const double value)
{
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra = (uint) value;
}

void slider_setFogR(const double value)
{
    level_fogR = (byte) value;
}

void slider_setFogG(const double value)
{
    level_fogG = (byte) value;
}

void slider_setFogB(const double value)
{
    level_fogB = (byte) value;
}

void slider_setFogStart(const double value)
{
    level_fogStart = value;
}

void slider_setFogEnd(const double value)
{
    level_fogEnd = value;
}

void slider_setFloorTex(const double value)
{
    level_floor_tex = (uint) value;
}

void slider_setCeilTex(const double value)
{
    level_ceil_tex = (byte) value;
}

void slider_setSkyR(const double value)
{
    level_skyR = (byte) value;
}

void slider_setSkyG(const double value)
{
    level_skyG = (byte) value;
}

void slider_setSkyB(const double value)
{
    level_skyB = (byte) value;
}

void slider_setActorParamA(const double value)
{
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra2 = (node->extra2 & 0x00FFFFFF) | ((byte) value << 24);
}

void slider_setActorParamB(const double value)
{
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra2 = (node->extra2 & 0xFF00FFFF) | ((byte) value << 16);
}

void slider_setActorParamC(const double value)
{
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra2 = (node->extra2 & 0xFFFF00FF) | ((byte) value << 8);
}

void slider_setActorParamD(const double value)
{
    EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
    node->extra2 = (node->extra2 & 0xFFFFFF00) | (byte) value;
}

void GEditorStateUpdate(GlobalState *State)
{
#ifdef ENABLE_LEVEL_EDITOR
    if (IsKeyJustPressed(SDL_SCANCODE_F6))
    {
        GMainStateSet();
    }

    if (IsMouseButtonPressed(SDL_BUTTON_RIGHT))
    {
        const Vector2 mouseDelta = GetMouseRel();
        EditorPanX += mouseDelta.x;
        EditorPanY += mouseDelta.y;
    }

    if (editorUiStack->ActiveControl != -1)
    {
        return; // eat input if we are interacting with a control
    }

    if (CurrentEditorMode == EDITOR_MODE_MOVE)
    {
        // check if we are hovering over a node
        for (int i = 0; i < EditorNodes->size; i++)
        {
            const EditorNode *node = ListGet(EditorNodes, i);
            const Vector2 screenPos = v2((node->position.x * EditorZoom) + EditorPanX,
                                   (node->position.y * EditorZoom) + EditorPanY);

            bool hovered = false;
            const Vector2 mousePos = GetMousePos();
            if (mousePos.x >= screenPos.x - 5 && mousePos.x <= screenPos.x + 5 &&
                mousePos.y >= screenPos.y - 5 && mousePos.y <= screenPos.y + 5)
            {
                hovered = true;
            }

            if (hovered && IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
            {
                EditorSelectedNode = i;
            }
        }

        if (IsMouseButtonJustReleased(SDL_BUTTON_LEFT))
        {
            EditorSelectedNode = -1;
        }

        // move the selected node to the mouse position
        if (EditorSelectedNode != -1)
        {
            EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
            const Vector2 mousePos = GetMousePos();
            node->position = v2((mousePos.x - EditorPanX) / EditorZoom, (mousePos.y - EditorPanY) / EditorZoom);

            if (EditorSnapToGrid)
            {
                node->position.x = round(node->position.x);
                node->position.y = round(node->position.y);
            }
        }
    } else if (CurrentEditorMode == EDITOR_MODE_DELETE)
    {
        for (int i = 0; i < EditorNodes->size; i++)
        {
            const EditorNode *node = ListGet(EditorNodes, i);
            Vector2 screenPos = v2((node->position.x * EditorZoom) + EditorPanX,
                                   (node->position.y * EditorZoom) + EditorPanY);

            bool hovered = false;
            const Vector2 mousePos = GetMousePos();
            if (mousePos.x >= screenPos.x - 5 && mousePos.x <= screenPos.x + 5 &&
                mousePos.y >= screenPos.y - 5 && mousePos.y <= screenPos.y + 5)
            {
                hovered = true;
            }

            if (hovered && IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
            {
                if (node->type != NODE_PLAYER)
                {
                    ListRemoveAt(EditorNodes, i);
                    // if we are deleting a wall, we need to delete both nodes
                    if (node->type == NODE_WALL_A)
                    {
                        ListRemoveAt(EditorNodes, i);
                        i--;
                    } else if (node->type == NODE_WALL_B)
                    {
                        ListRemoveAt(EditorNodes, i - 1);
                        i--;
                    }
                }
                break; // don't delete more than one node per click
            }
        }
    } else if (CurrentEditorMode == EDITOR_MODE_ADD)
    {
        const Control *modeSld = ListGet(editorUiStack->Controls, 0 + EditorBaseControlCount);
        const SliderData *sldData = (SliderData *) modeSld->ControlData;
        int mode = sldData->value;
        switch (mode)
        {
            case 0:
            {
                if (IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
                {
                    const Vector2 mousePos = GetMousePos();
                    Vector2 worldPos = v2((mousePos.x - EditorPanX) / EditorZoom,
                                          (mousePos.y - EditorPanY) / EditorZoom);

                    if (EditorSnapToGrid)
                    {
                        worldPos.x = round(worldPos.x);
                        worldPos.y = round(worldPos.y);
                    }

                    const Control *texSld = ListGet(editorUiStack->Controls, 1 + EditorBaseControlCount);
                    const SliderData *texData = (SliderData *) texSld->ControlData;
                    const int tex = texData->value;


                    // Create 2 nodes for a wall
                    EditorNode *nodeA = malloc(sizeof(EditorNode));
                    nodeA->type = NODE_WALL_A;
                    nodeA->position = worldPos;
                    nodeA->extra = tex;
                    const float uv = 1.0;
                    nodeA->extra2 = uv;
                    ListAdd(EditorNodes, nodeA);

                    EditorNode *nodeB = malloc(sizeof(EditorNode));
                    nodeB->type = NODE_WALL_B;
                    nodeB->position = worldPos;
                    ListAdd(EditorNodes, nodeB);

                    isAddModeDragging = true;
                } else if (IsMouseButtonJustReleased(SDL_BUTTON_LEFT))
                {
                    isAddModeDragging = false;
                } else if (isAddModeDragging)
                {
                    // delete the 2 nodes we just created if escape is pressed
                    if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE))
                    {
                        ListRemoveAt(EditorNodes, EditorNodes->size - 1);
                        ListRemoveAt(EditorNodes, EditorNodes->size - 1);
                        isAddModeDragging = false;
                        return;
                    }

                    const Vector2 mousePos = GetMousePos();
                    Vector2 worldPos = v2((mousePos.x - EditorPanX) / EditorZoom,
                                          (mousePos.y - EditorPanY) / EditorZoom);

                    if (EditorSnapToGrid)
                    {
                        worldPos.x = round(worldPos.x);
                        worldPos.y = round(worldPos.y);
                    }

                    EditorNode *nodeB = ListGet(EditorNodes, EditorNodes->size - 1);
                    nodeB->position = worldPos;
                }
                break;
            }
            case 1:
            {
                if (IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
                {
                    EditorNode *node = malloc(sizeof(EditorNode));
                    node->type = NODE_ACTOR;
                    const Vector2 mousePos = GetMousePos();
                    Vector2 worldPos = v2((mousePos.x - EditorPanX) / EditorZoom,
                                          (mousePos.y - EditorPanY) / EditorZoom);

                    if (EditorSnapToGrid)
                    {
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
    } else if (CurrentEditorMode == EDITOR_MODE_PROPERTIES)
    {
        for (int i = 0; i < EditorNodes->size; i++)
        {
            const EditorNode *node = ListGet(EditorNodes, i);
            const Vector2 screenPos = v2((node->position.x * EditorZoom) + EditorPanX,
                                   (node->position.y * EditorZoom) + EditorPanY);

            bool hovered = false;
            const Vector2 mousePos = GetMousePos();
            if (mousePos.x >= screenPos.x - 5 && mousePos.x <= screenPos.x + 5 &&
                mousePos.y >= screenPos.y - 5 && mousePos.y <= screenPos.y + 5)
            {
                hovered = true;
            }

            if (hovered && IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
            {
                EditorSelectedNode = i;
            }
        }

        while (editorUiStack->Controls->size > EditorBaseControlCount)
        {
            UiStackRemove(editorUiStack, ListGet(editorUiStack->Controls, EditorBaseControlCount));
        }

        const EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
        switch (node->type)
        {
            case NODE_PLAYER:
                CreateSlider("ang", 0, 359, radToDeg(node->rotation), 1, 45, v2(10, 250), v2(200, 24),
                             slider_setNodeRotation, NULL);
                break;
            case NODE_ACTOR:
                CreateSlider("ang", 0, 359, radToDeg(node->rotation), 1, 45, v2(10, 250), v2(200, 24),
                             slider_setNodeRotation, NULLPTR);
                CreateSlider("Type", 0, GetActorTypeCount() - 1, node->extra, 1, 16, v2(10, 300), v2(200, 24),
                             slider_setNodeExtra, SliderLabelInteger);
                CreateSlider("Param A", 0, 255, (node->extra2 >> 24) & 0xFF, 1, 16, v2(10, 350), v2(200, 24),
                             slider_setActorParamA, SliderLabelInteger);
                CreateSlider("Param B", 0, 255, (node->extra2 >> 16) & 0xFF, 1, 16, v2(10, 400), v2(200, 24),
                             slider_setActorParamB, SliderLabelInteger);
                CreateSlider("Param C", 0, 255, (node->extra2 >> 8) & 0xFF, 1, 16, v2(10, 450), v2(200, 24),
                             slider_setActorParamC, SliderLabelInteger);
                CreateSlider("Param D", 0, 255, node->extra2 & 0xFF, 1, 16, v2(10, 500), v2(200, 24),
                             slider_setActorParamD, SliderLabelInteger);
                break;
            case NODE_WALL_A:
                CreateSlider("Tex", 0, WALL_TEXTURE_COUNT - 1, node->extra, 1, 16, v2(10, 250), v2(200, 24),
                             slider_setNodeExtra, SliderLabelInteger);
                break;
            default:
                break;
        }
    }
#endif
}

void GEditorStateRender(GlobalState *State)
{
#ifdef ENABLE_LEVEL_EDITOR
    setColorUint(0xFF123456);
    ClearColor(0xFF123456);

    const int gridSpacing = EditorZoom;
    const int gridOffsetX = (int) EditorPanX % gridSpacing;
    const int gridOffsetY = (int) EditorPanY % gridSpacing;

    setColorUint(0xFF808080);
    for (int x = gridOffsetX; x < WindowWidth(); x += gridSpacing)
    {
        draw_rect(x, 0, 1, WindowHeight());
    }
    for (int y = gridOffsetY; y < WindowHeight(); y += gridSpacing)
    {
        draw_rect(0, y, WindowWidth(), 1);
    }

    setColorUint(0xFF0000FF);
    draw_rect((int) EditorPanX, 0, 1, WindowHeight());
    setColorUint(0xFFFF0000);
    draw_rect(0, (int) EditorPanY, WindowWidth(), 1);

    // draw world space numbers along bottom and right
    char buf[32];
    for (int x = gridOffsetX; x < WindowWidth(); x += gridSpacing)
    {
        const int worldSpaceX = (int) ((x - EditorPanX) / EditorZoom);

        if (worldSpaceX % 5 != 0)
        {
            continue;
        }
        sprintf(buf, "%d", worldSpaceX);
        DrawTextAligned(buf, 16, 0xFFFFFFFF, v2(x - 50, WindowHeight() - 25), v2(100, 20), FONT_HALIGN_CENTER,
                        FONT_VALIGN_MIDDLE, false);
    }
    for (int y = gridOffsetY; y < WindowHeight(); y += gridSpacing)
    {
        const int worldSpaceY = (int) ((y - EditorPanY) / EditorZoom);

        if (worldSpaceY % 5 != 0)
        {
            continue;
        }
        sprintf(buf, "%d", worldSpaceY);
        DrawTextAligned(buf, 16, 0xFFFFFFFF, v2(WindowWidth() - 110, y - 10), v2(100, 20), FONT_HALIGN_RIGHT,
                        FONT_VALIGN_MIDDLE, false);
    }

    // Draw nodes
    int hoveredNode = -1;
    for (int i = 0; i < EditorNodes->size; i++)
    {
        const EditorNode *node = ListGet(EditorNodes, i);
        const Vector2 screenPos = v2((node->position.x * EditorZoom) + EditorPanX,
                               (node->position.y * EditorZoom) + EditorPanY);

        if (node->type == NODE_WALL_A)
        {
            // Draw a line to the next node, which should be the wall's other end
            const EditorNode *nodeB = ListGet(EditorNodes, i + 1);
            const Vector2 screenPosB = v2((nodeB->position.x * EditorZoom) + EditorPanX,
                                    (nodeB->position.y * EditorZoom) + EditorPanY);
            setColorUint(0xFFFFFFFF);
            DrawLine(v2(screenPos.x, screenPos.y), v2(screenPosB.x, screenPosB.y), 2);
        }

        uint color = 0;
        switch (node->type)
        {
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
        const Vector2 mousePos = GetMousePos();
        if (mousePos.x >= screenPos.x - 5 && mousePos.x <= screenPos.x + 5 &&
            mousePos.y >= screenPos.y - 5 && mousePos.y <= screenPos.y + 5)
        {
            hovered = true;
            hoveredNode = i;
        }

        if (EditorSelectedNode == i)
        {
            color = 0xFFFFFF00;
        }

        if (hovered || EditorSelectedNode == i)
        {
            setColorUint(0xFFFFFFFF);
            draw_rect(screenPos.x - 6, screenPos.y - 6, 12, 12);
        }

        setColorUint(color);
        draw_rect(screenPos.x - 5, screenPos.y - 5, 10, 10);

        // for player and actor nodes, draw a line indicating rotation
        if (node->type == NODE_PLAYER || node->type == NODE_ACTOR)
        {
            const Vector2 lineEnd = v2(screenPos.x + (cos(node->rotation) * 20), screenPos.y + (sin(node->rotation) * 20));
            DrawLine(screenPos, lineEnd, 1);
        }
    }

    if (hoveredNode != -1)
    {
        const EditorNode *node = ListGet(EditorNodes, hoveredNode);
        const Vector2 screenPos = v2((node->position.x * EditorZoom) + EditorPanX,
                               (node->position.y * EditorZoom) + EditorPanY);

        char nodeInfo[96];
        switch (node->type)
        {
            case NODE_PLAYER:
                sprintf(nodeInfo, "Player: %.2f, %.2f\nRotation: %.2f", node->position.x, node->position.y,
                        radToDeg(node->rotation));
                break;
            case NODE_ACTOR:
                sprintf(nodeInfo, "Actor: 0x%04x\nPosition: %.2f, %.2f\nRotation: %.2f", node->extra, node->position.x,
                        node->position.y, radToDeg(node->rotation));
                break;
            case NODE_WALL_A:
                sprintf(nodeInfo, "Wall (A): %.2f, %.2f\nTexture: 0x%04x", node->position.x, node->position.y,
                        node->extra);
                break;
            case NODE_WALL_B:
                sprintf(nodeInfo, "Wall (B): %.2f, %.2f", node->position.x, node->position.y);
                break;
        }

        const Vector2 measuredText = MeasureText(nodeInfo, 16, false);
        const int textWidth = measuredText.x;
        const int textHeight = measuredText.y;
        setColorUint(0x80000000);
        draw_rect(screenPos.x + 10, screenPos.y, textWidth + 20, textHeight + 20);
        FontDrawString(v2(screenPos.x + 20, screenPos.y + 10), nodeInfo, 16, 0xFFFFFFFF, false);
    }

    ProcessUiStack(editorUiStack);
    DrawUiStack(editorUiStack);

    // if we are editing the properties of a wall a node, draw its texture
    if (CurrentEditorMode == EDITOR_MODE_PROPERTIES && EditorSelectedNode != -1)
    {
        const EditorNode *node = ListGet(EditorNodes, EditorSelectedNode);
        if (node->type == NODE_WALL_A)
        {
            const byte *tex = wallTextures[node->extra];
            if (tex != NULL)
            {
                SDL_Rect dst = {10, 310, 64, 64};
                DrawTexture(v2(dst.x, dst.y), v2(dst.w, dst.h), tex);
            }
        }
    } else if (CurrentEditorMode == EDITOR_MODE_ADD)
    {
        const Control *texSld = ListGet(editorUiStack->Controls, 1 + EditorBaseControlCount);
        const SliderData *sliderData = (SliderData *) texSld->ControlData;

        const byte *tex = wallTextures[(int) (sliderData->value)];
        if (tex != NULL)
        {
            const SDL_Rect dst = {10, 360, 64, 64};
            DrawTexture(v2(dst.x, dst.y), v2(dst.w, dst.h), tex);
        }
    }

#endif
}

void CreateButton(char *text, const Vector2 position, const Vector2 size, void (*callback)(), bool enabled)
{
    Control *button = CreateButtonControl(position, size, text, callback, TOP_LEFT);
    UiStackPush(editorUiStack, button);
}

void BtnZoomIn()
{
    EditorZoom += 2.0;
    EditorZoom = clampf(EditorZoom, 10.0, 60.0);
}

void BtnZoomOut()
{
    EditorZoom -= 2.0;
    EditorZoom = clampf(EditorZoom, 10.0, 60.0);
}

void BtnZoomReset()
{
    EditorZoom = 20.0;
}

void BtnCopyBytecode()
{
    LevelBytecode *bc = GenerateBytecode(GetState()->level);
    char *buf = malloc(bc->size * 2 + 1);
    for (int i = 0; i < bc->size; i++)
    {
        sprintf(buf + i * 2, "%02x", bc->data[i]);
    }
    buf[bc->size * 2] = '\0';

    SDL_SetClipboardText(buf);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Bytecode copied",
                             "The level bytecode has been copied to the clipboard.", NULL);

    free(bc->data);
    free(bc);
    free(buf);
}

void ToggleSnapToGrid(bool enabled)
{
    EditorSnapToGrid = enabled;
}

void BtnPrevNode()
{
    if (CurrentEditorMode == EDITOR_MODE_PROPERTIES)
    {
        EditorSelectedNode--;
        if (EditorSelectedNode < 0)
        {
            EditorSelectedNode = EditorNodes->size - 1;
        }
    }
}

void BtnNextNode()
{
    if (CurrentEditorMode == EDITOR_MODE_PROPERTIES)
    {
        EditorSelectedNode++;
        if (EditorSelectedNode >= EditorNodes->size)
        {
            EditorSelectedNode = 0;
        }
    }
}

void SetEditorMode(bool _c, byte _g, byte id)
{
    // Remove all controls that were added for the previous mode
    while (editorUiStack->Controls->size > EditorBaseControlCount)
    {
        UiStackRemove(editorUiStack, ListGet(editorUiStack->Controls, EditorBaseControlCount));
    }

    int sy = 250;
    const int sp = 10;
    const int szy = 24;

    if (id == 0)
    {
        CurrentEditorMode = EDITOR_MODE_ADD;
        CreateSlider("Add Actor?", 0, 1, 0, 1, 1, v2(10, sy), v2(200, szy), NULL, SliderLabelInteger);
        sy += szy + sp;
        CreateSlider("Wall Tex", 0, WALL_TEXTURE_COUNT - 1, 0, 1, 16, v2(10, sy), v2(200, szy), NULL,
                     SliderLabelInteger);
    } else if (id == 1)
    {
        EditorSelectedNode = -1;
        CurrentEditorMode = EDITOR_MODE_MOVE;
    } else if (id == 2)
    {
        CurrentEditorMode = EDITOR_MODE_DELETE;
    } else if (id == 3)
    {
        CurrentEditorMode = EDITOR_MODE_PROPERTIES;
        EditorSelectedNode = 0;
    } else if (id == 4)
    {
        CurrentEditorMode = EDITOR_MODE_LEVEL;
        CreateSlider("Fog R", 0, 255, level_fogR, 1, 16, v2(10, sy), v2(200, 24), slider_setFogR, SliderLabelInteger);
        sy += szy + sp;
        CreateSlider("Fog G", 0, 255, level_fogG, 1, 16, v2(10, sy), v2(200, 24), slider_setFogG, SliderLabelInteger);
        sy += szy + sp;
        CreateSlider("Fog B", 0, 255, level_fogB, 1, 16, v2(10, sy), v2(200, 24), slider_setFogB, SliderLabelInteger);
        sy += szy + sp;
        CreateSlider("Fog Start", -50, 200, level_fogStart, 1, 50, v2(10, sy), v2(200, 24), slider_setFogStart,
                     NULLPTR);
        sy += szy + sp;
        CreateSlider("Fog End", 0, 300, level_fogEnd, 1, 50, v2(10, sy), v2(200, 24), slider_setFogEnd, NULLPTR);
        sy += szy + sp;
        CreateSlider("Floor Tex", 0, WALL_TEXTURE_COUNT - 1, level_floor_tex, 1, 16, v2(10, sy), v2(200, 24),
                     slider_setFloorTex, SliderLabelInteger);
        sy += szy + sp;
        CreateSlider("Ceil Tex", 0, WALL_TEXTURE_COUNT, level_ceil_tex, 1, 16, v2(10, sy), v2(200, 24),
                     slider_setCeilTex, SliderLabelInteger);
        sy += szy + sp;
        CreateSlider("Sky R", 0, 255, level_skyR, 1, 16, v2(10, sy), v2(200, 24), slider_setSkyR, SliderLabelInteger);
        sy += szy + sp;
        CreateSlider("Sky G", 0, 255, level_skyG, 1, 16, v2(10, sy), v2(200, 24), slider_setSkyG, SliderLabelInteger);
        sy += szy + sp;
        CreateSlider("Sky B", 0, 255, level_skyB, 1, 16, v2(10, sy), v2(200, 24), slider_setSkyB, SliderLabelInteger);
        sy += szy + sp;
        CreateSlider("Music", 0, MUSIC_COUNT, musicId, 1, 1, v2(10, sy), v2(200, 24), slider_setMusic,
                     SliderLabelInteger);
    }
}

void BtnLoad()
{
    // clear all editor nodes
    ListFreeWithData(EditorNodes);
    EditorNodes = CreateList();

    const Level *l = GetState()->level;

    // load level properties
    level_fogR = l->FogColor >> 16;
    level_fogG = l->FogColor >> 8;
    level_fogB = l->FogColor;
    level_fogStart = l->FogStart;
    level_fogEnd = l->FogEnd;
    level_ceil_tex = l->CeilingTexture;
    level_floor_tex = l->FloorTexture;
    level_skyR = l->SkyColor >> 16;
    level_skyG = l->SkyColor >> 8;
    level_skyB = l->SkyColor;
    musicId = l->MusicID;

    // add a node for the player
    EditorNode *playerNode = malloc(sizeof(EditorNode));
    playerNode->type = NODE_PLAYER;
    playerNode->position = l->position;
    playerNode->rotation = fmod(l->rotation, 2 * PI);
    ListAdd(EditorNodes, playerNode);

    // add a node for each actor
    for (int i = 0; i < ListGetSize(l->actors); i++)
    {
        Actor *a = ListGet(l->actors, i);
        EditorNode *actorNode = malloc(sizeof(EditorNode));
        actorNode->type = NODE_ACTOR;
        actorNode->position = a->position;
        actorNode->rotation = fmod(a->rotation, 2 * PI);
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
    for (int i = 0; i < ListGetSize(l->walls); i++)
    {
        const Wall *w = ListGet(l->walls, i);
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
}

void BtnTest()
{
    Level *l = NodesToLevel();
    ChangeLevel(l);
    GMainStateSet();
}

void GEditorStateSet()
{
    if (!EditorInitComplete)
    {
        // center the view to 0,0
        EditorPanX = WindowWidth() / 2;
        EditorPanY = WindowHeight() / 2;

        editorUiStack = CreateUiStack();
        EditorNodes = CreateList();
        // will be freed immediately after this function, but we create it here to avoid nullptr free

        int sx = 10;
        const int szy = 30;
        const int szx = 120;
        const int sp = 30;
        UiStackPush(editorUiStack,
                    CreateRadioButtonControl(v2(sx, 10), v2(szx, szy), "Add", SetEditorMode, TOP_LEFT, true, 0, 0));
        sx += szx + sp;
        UiStackPush(editorUiStack,
                    CreateRadioButtonControl(v2(sx, 10), v2(szx, szy), "Move", SetEditorMode, TOP_LEFT, false, 0, 1));
        sx += szx + sp;
        UiStackPush(editorUiStack,
                    CreateRadioButtonControl(v2(sx, 10), v2(szx, szy), "Delete", SetEditorMode, TOP_LEFT, false, 0, 2));
        sx += szx + sp;
        UiStackPush(editorUiStack,
                    CreateRadioButtonControl(v2(sx, 10), v2(szx, szy), "Prop", SetEditorMode, TOP_LEFT, false, 0, 3));
        sx += szx + sp;
        UiStackPush(editorUiStack,
                    CreateRadioButtonControl(v2(sx, 10), v2(szx, szy), "Level", SetEditorMode, TOP_LEFT, false, 0, 4));

        CreateButton("Zoom In", v2(10, 50), v2(120, 24), BtnZoomIn, true);
        CreateButton("Zoom Out", v2(10, 78), v2(120, 24), BtnZoomOut, true);
        CreateButton("Zoom 100", v2(10, 106), v2(120, 24), BtnZoomReset, true);

        CreateButton("PREV Node", v2(140, 50), v2(120, 24), BtnPrevNode, true);
        CreateButton("NEXT Node", v2(140, 78), v2(120, 24), BtnNextNode, true);

        UiStackPush(editorUiStack, CreateCheckboxControl(v2(10, 140), v2(120, 30), "Snap", ToggleSnapToGrid, TOP_LEFT,
                                                         EditorSnapToGrid));

        UiStackPush(editorUiStack, CreateButtonControl(v2(-60, 10), v2(120, 24), "Load", BtnLoad, TOP_RIGHT));
        UiStackPush(editorUiStack, CreateButtonControl(v2(-60, 30), v2(120, 24), "Test", BtnTest, TOP_RIGHT));
        UiStackPush(editorUiStack, CreateButtonControl(v2(-60, 50), v2(120, 24), "Save", BtnCopyBytecode, TOP_RIGHT));

        EditorBaseControlCount = ListGetSize(editorUiStack->Controls);

        SetEditorMode(false, 0, 0);

        EditorInitComplete = true;
    }

    SetRenderCallback(GEditorStateRender);
    SetUpdateCallback(GEditorStateUpdate, NULL, EDITOR_STATE);
}
