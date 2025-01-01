//
// Created by droc101 on 6/23/2024.
//

#include "GEditorState.h"
#include <math.h>
#include <stdio.h>
#include "../config.h"
#include "../Helpers/CommonAssets.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/MathEx.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Helpers/Graphics/Font.h"
#include "../Helpers/LevelLoader.h"
#include "../Structs/Actor.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Level.h"
#include "../Structs/UI/Controls/Button.h"
#include "../Structs/UI/Controls/CheckBox.h"
#include "../Structs/UI/Controls/RadioButton.h"
#include "../Structs/UI/Controls/Slider.h"
#include "../Structs/UI/UiStack.h"
#include "../Structs/Wall.h"
#include "GMainState.h"

typedef enum
{
	EDITOR_MODE_ADD, // click to add a node
	EDITOR_MODE_MOVE, // move nodes around
	EDITOR_MODE_DELETE, // delete nodes
	EDITOR_MODE_PROPERTIES, // edit node properties
	EDITOR_MODE_LEVEL // edit level properties
} EditorMode;

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

#pragma region Editor State Variables
double editorZoom = 20.0;
double editorPanX = 0.0;
double editorPanY = 0.0;
bool editorInitComplete = false;
bool editorSnapToGrid = true;

int editorSelectedNode = -1;

int editorBaseControlCount = -1;

EditorMode currentEditorMode = EDITOR_MODE_MOVE;

UiStack *editorUiStack;

List *editorNodes;

bool isAddModeDragging = false;
#pragma endregion

#pragma region Editor Level Variables
byte level_fogR;
byte level_fogG;
byte level_fogB;

double level_fogStart;
double level_fogEnd;

uint level_floorTex;
byte level_ceilTex;

byte level_skyR;
byte level_skyG;
byte level_skyB;

uint level_musicID;
#pragma endregion

#pragma region Helpers

char *SliderActorNameLabelCallback(const Control *slider)
{
	const SliderData *data = (SliderData *)slider->ControlData;
	char *buf = malloc(64);
	chk_malloc(buf);
	sprintf(buf, "Type: %s", GetActorName(data->value));
	return buf;
}

Level *NodesToLevel()
{
	Level *l = CreateLevel();

	l->fogColor = 0xFF << 24 | level_fogR << 16 | level_fogG << 8 | level_fogB;
	l->fogStart = level_fogStart;
	l->fogEnd = level_fogEnd;

	l->floorTexture = level_floorTex;
	l->ceilingTexture = level_ceilTex;
	l->skyColor = 0xFF << 24 | level_skyR << 16 | level_skyG << 8 | level_skyB;

	l->musicID = level_musicID;

	// reconstruct the level from the editor nodes
	for (int i = 0; i < editorNodes->size; i++)
	{
		const EditorNode *node = ListGet(editorNodes, i);
		switch (node->type)
		{
			case NODE_PLAYER:
				l->player.pos = node->position;
				l->player.angle = node->rotation;
				break;
			case NODE_ACTOR:
			{
				const byte paramA = node->extra2 >> 24 & 0xFF;
				const byte paramB = node->extra2 >> 16 & 0xFF;
				const byte paramC = node->extra2 >> 8 & 0xFF;
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

void CreateButton(char *text, const Vector2 position, const Vector2 size,
				  const ButtonCallback callback, bool /*enabled*/)
{
	Control *button = CreateButtonControl(position, size, text, callback, TOP_LEFT);
	UiStackPush(editorUiStack, button);
}

void CreateSlider(char *label,
				  const double min,
				  const double max,
				  const double value,
				  const double step,
				  const double altStep,
				  const Vector2 position,
				  const Vector2 size,
				  const SliderCallback callback,
				  const SliderLabelFunction getLabel)
{
	Control *slider = CreateSliderControl(position,
										  size,
										  label,
										  callback,
										  TOP_LEFT,
										  min,
										  max,
										  value,
										  step,
										  altStep,
										  getLabel);
	UiStackPush(editorUiStack, slider);
}


void RenderGrid()
{
	const int gridSpacing = editorZoom;
	const int gridOffsetX = (int)editorPanX % gridSpacing;
	const int gridOffsetY = (int)editorPanY % gridSpacing;

	SetColorUint(0xFF808080);
	for (int x = gridOffsetX; x < WindowWidth(); x += gridSpacing)
	{
		DrawLine(v2(x, 0), v2(x, WindowHeight()), 1.0f);
	}
	for (int y = gridOffsetY; y < WindowHeight(); y += gridSpacing)
	{
		DrawLine(v2(0, y), v2(WindowWidth(), y), 1.0f);
	}

	SetColorUint(0xFF0000FF);
	DrawLine(v2(editorPanX, 0), v2(editorPanX, WindowHeight()), 2.0f);
	SetColorUint(0xFFFF0000);
	DrawLine(v2(0, editorPanY), v2(WindowWidth(), editorPanY), 2.0f);

	// draw world space numbers along bottom and right
	char buf[32];
	for (int x = gridOffsetX; x < WindowWidth(); x += gridSpacing)
	{
		const int worldSpaceX = (int)((x - editorPanX) / editorZoom);

		if (worldSpaceX % 5 != 0)
		{
			continue;
		}
		sprintf(buf, "%d", worldSpaceX);
		DrawTextAligned(buf,
						16,
						0xFFFFFFFF,
						v2(x - 50, WindowHeight() - 25),
						v2(100, 20),
						FONT_HALIGN_CENTER,
						FONT_VALIGN_MIDDLE,
						false);
	}
	for (int y = gridOffsetY; y < WindowHeight(); y += gridSpacing)
	{
		const int worldSpaceY = (int)((y - editorPanY) / editorZoom);

		if (worldSpaceY % 5 != 0)
		{
			continue;
		}
		sprintf(buf, "%d", worldSpaceY);
		DrawTextAligned(buf,
						16,
						0xFFFFFFFF,
						v2(WindowWidth() - 110, y - 10),
						v2(100, 20),
						FONT_HALIGN_RIGHT,
						FONT_VALIGN_MIDDLE,
						false);
	}
}

void DrawNode(const EditorNode *node, int *hoveredNode, const int i)
{
	const Vector2 screenPos = v2(node->position.x * editorZoom + editorPanX,
								 node->position.y * editorZoom + editorPanY);

	if (node->type == NODE_WALL_A)
	{
		// Draw a line to the next node, which should be the wall's other end
		const EditorNode *nodeB = ListGet(editorNodes, i + 1);
		const Vector2 screenPosB = v2(nodeB->position.x * editorZoom + editorPanX,
									  nodeB->position.y * editorZoom + editorPanY);
		SetColorUint(0xFFFFFFFF);
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
			color = 0xFF0000A0;
			break;
	}

	bool hovered = false;
	const Vector2 mousePos = GetMousePos();
	if (mousePos.x >= screenPos.x - 5 &&
		mousePos.x <= screenPos.x + 5 &&
		mousePos.y >= screenPos.y - 5 &&
		mousePos.y <= screenPos.y + 5)
	{
		hovered = true;
		*hoveredNode = i;
	}

	if (editorSelectedNode == i)
	{
		color = 0xFFFFFF00;
	}

	if (hovered || editorSelectedNode == i)
	{
		SetColorUint(0xFFFFFFFF);
		DrawRect(screenPos.x - 6, screenPos.y - 6, 12, 12);
	}

	SetColorUint(color);
	DrawRect(screenPos.x - 5, screenPos.y - 5, 10, 10);

	// for player and actor nodes, draw a line indicating rotation
	if (node->type == NODE_PLAYER || node->type == NODE_ACTOR)
	{
		const Vector2 lineEnd = v2(screenPos.x + cos(node->rotation) * 20, screenPos.y + sin(node->rotation) * 20);
		DrawLine(screenPos, lineEnd, 1);
	}
}

void DrawNodeTooltip(const EditorNode *node)
{
	const Vector2 screenPos = v2(node->position.x * editorZoom + editorPanX,
								 node->position.y * editorZoom + editorPanY);

	char nodeInfo[96];
	switch (node->type)
	{
		case NODE_PLAYER:
			sprintf(nodeInfo,
					"Player: %.2f, %.2f\nRotation: %.2f",
					node->position.x,
					node->position.y,
					radToDeg(node->rotation));
			break;
		case NODE_ACTOR:
			sprintf(nodeInfo,
					"Actor: 0x%04x\nPosition: %.2f, %.2f\nRotation: %.2f",
					node->extra,
					node->position.x,
					node->position.y,
					radToDeg(node->rotation));
			break;
		case NODE_WALL_A:
			sprintf(nodeInfo, "Wall (A): %.2f, %.2f\nTexture: 0x%04x", node->position.x, node->position.y, node->extra);
			break;
		case NODE_WALL_B:
			sprintf(nodeInfo, "Wall (B): %.2f, %.2f", node->position.x, node->position.y);
			break;
	}

	const Vector2 measuredText = MeasureText(nodeInfo, 16, false);
	const int textWidth = measuredText.x;
	const int textHeight = measuredText.y;
	SetColorUint(0x80000000);
	DrawRect(screenPos.x + 10, screenPos.y, textWidth + 20, textHeight + 20);
	FontDrawString(v2(screenPos.x + 20, screenPos.y + 10), nodeInfo, 16, 0xFFFFFFFF, false);
}

#pragma endregion

#pragma region Callbacks

void SetMusicSlider(const double value)
{
	level_musicID = (byte)value;
}

void SetNodeRotationSlider(const double value)
{
	EditorNode *node = ListGet(editorNodes, editorSelectedNode);
	node->rotation = degToRad(value);
}

void SetNodeExtraSlider(const double value)
{
	EditorNode *node = ListGet(editorNodes, editorSelectedNode);
	node->extra = (uint)value;
}

void SetFogRSlider(const double value)
{
	level_fogR = (byte)value;
}

void SetFogGSlider(const double value)
{
	level_fogG = (byte)value;
}

void SetFogBSlider(const double value)
{
	level_fogB = (byte)value;
}

void SetFogStartSlider(const double value)
{
	level_fogStart = value;
}

void SetFogEndSlider(const double value)
{
	level_fogEnd = value;
}

void SetFloorTexSlider(const double value)
{
	level_floorTex = (uint)value;
}

void SetCeilTexSlider(const double value)
{
	level_ceilTex = (byte)value;
}

void SetSkyRSlider(const double value)
{
	level_skyR = (byte)value;
}

void SetSkyGSlider(const double value)
{
	level_skyG = (byte)value;
}

void SetSkyBSlider(const double value)
{
	level_skyB = (byte)value;
}

void SetActorParamASlider(const double value)
{
	EditorNode *node = ListGet(editorNodes, editorSelectedNode);
	// ReSharper disable once CppRedundantParentheses
	node->extra2 = (node->extra2 & 0x00FFFFFF) | (byte)value << 24;
}

void SetActorParamBSlider(const double value)
{
	EditorNode *node = ListGet(editorNodes, editorSelectedNode);
	// ReSharper disable once CppRedundantParentheses
	node->extra2 = (node->extra2 & 0xFF00FFFF) | (byte)value << 16;
}

void SetActorParamCSlider(const double value)
{
	EditorNode *node = ListGet(editorNodes, editorSelectedNode);
	// ReSharper disable once CppRedundantParentheses
	node->extra2 = (node->extra2 & 0xFFFF00FF) | (byte)value << 8;
}

void SetActorParamDSlider(const double value)
{
	EditorNode *node = ListGet(editorNodes, editorSelectedNode);
	// ReSharper disable once CppRedundantParentheses
	node->extra2 = (node->extra2 & 0xFFFFFF00) | (byte)value;
}


void BtnZoomIn()
{
	editorZoom += 2.0;
	editorZoom = clampf(editorZoom, 10.0, 60.0);
}

void BtnZoomOut()
{
	editorZoom -= 2.0;
	editorZoom = clampf(editorZoom, 10.0, 60.0);
}

void BtnZoomReset()
{
	editorZoom = 20.0;
}

void SDLCALL SaveDialogCallback(void *userdata, const char * const *filelist, int filter)
{
	LevelBytecode *bc = (LevelBytecode *)userdata;
	if (!filelist || !*filelist) // error or cancel
	{
		free(bc->data);
		free(bc);
		LogError("File dialog encountered an error or was cancelled.");
		return;
	}

	const char *filename = filelist[0];

	FILE *file = fopen(filename, "wb");
	if (!file)
	{
		Error("Failed to open file for writing");
	}
	fwrite(bc->data, 1, bc->size, file);
	fclose(file);

	free(bc->data);
	free(bc);
}

void BtnCopyBytecode()
{
	LevelBytecode *bc = GenerateBytecode(GetState()->level);

	const SDL_DialogFileFilter filters[] = {
		{ "Level Data File", "bin" }
	};

	SDL_ShowSaveFileDialog(SaveDialogCallback, bc, GetGameWindow(), filters, 1, NULL);
}

void ToggleSnapToGrid(const bool enabled)
{
	editorSnapToGrid = enabled;
}

void BtnPrevNode()
{
	if (currentEditorMode == EDITOR_MODE_PROPERTIES)
	{
		editorSelectedNode--;
		if (editorSelectedNode < 0)
		{
			editorSelectedNode = editorNodes->size - 1;
		}
	}
}

void BtnNextNode()
{
	if (currentEditorMode == EDITOR_MODE_PROPERTIES)
	{
		editorSelectedNode++;
		if (editorSelectedNode >= editorNodes->size)
		{
			editorSelectedNode = 0;
		}
	}
}

void SetEditorMode(bool /*c*/, byte /*g*/, const byte id)
{
	// Remove all controls that were added for the previous mode
	while (editorUiStack->Controls->size > editorBaseControlCount)
	{
		UiStackRemove(editorUiStack, ListGet(editorUiStack->Controls, editorBaseControlCount));
	}

	int sy = 250;
	const int sp = 10;
	const int szy = 24;

	if (id == 0)
	{
		currentEditorMode = EDITOR_MODE_ADD;
		CreateSlider("Add Actor?", 0, 1, 0, 1, 1, v2(10, sy), v2(200, szy), NULL, SliderLabelInteger);
		sy += szy + sp;
		CreateSlider("Wall Tex",
					 0,
					 WALL_TEXTURE_COUNT - 1,
					 0,
					 1,
					 16,
					 v2(10, sy),
					 v2(200, szy),
					 NULL,
					 SliderLabelInteger);
	} else if (id == 1)
	{
		editorSelectedNode = -1;
		currentEditorMode = EDITOR_MODE_MOVE;
	} else if (id == 2)
	{
		currentEditorMode = EDITOR_MODE_DELETE;
	} else if (id == 3)
	{
		currentEditorMode = EDITOR_MODE_PROPERTIES;
		editorSelectedNode = 0;
	} else if (id == 4)
	{
		currentEditorMode = EDITOR_MODE_LEVEL;
		CreateSlider("Fog R", 0, 255, level_fogR, 1, 16, v2(10, sy), v2(200, 24), SetFogRSlider, SliderLabelInteger);
		sy += szy + sp;
		CreateSlider("Fog G", 0, 255, level_fogG, 1, 16, v2(10, sy), v2(200, 24), SetFogGSlider, SliderLabelInteger);
		sy += szy + sp;
		CreateSlider("Fog B", 0, 255, level_fogB, 1, 16, v2(10, sy), v2(200, 24), SetFogBSlider, SliderLabelInteger);
		sy += szy + sp;
		CreateSlider("Fog Start", -50, 200, level_fogStart, 1, 50, v2(10, sy), v2(200, 24), SetFogStartSlider, NULL);
		sy += szy + sp;
		CreateSlider("Fog End", 0, 300, level_fogEnd, 1, 50, v2(10, sy), v2(200, 24), SetFogEndSlider, NULL);
		sy += szy + sp;
		CreateSlider("Floor Tex",
					 0,
					 WALL_TEXTURE_COUNT - 1,
					 level_floorTex,
					 1,
					 16,
					 v2(10, sy),
					 v2(200, 24),
					 SetFloorTexSlider,
					 SliderLabelInteger);
		sy += szy + sp;
		CreateSlider("Ceil Tex",
					 0,
					 WALL_TEXTURE_COUNT,
					 level_ceilTex,
					 1,
					 16,
					 v2(10, sy),
					 v2(200, 24),
					 SetCeilTexSlider,
					 SliderLabelInteger);
		sy += szy + sp;
		CreateSlider("Sky R", 0, 255, level_skyR, 1, 16, v2(10, sy), v2(200, 24), SetSkyRSlider, SliderLabelInteger);
		sy += szy + sp;
		CreateSlider("Sky G", 0, 255, level_skyG, 1, 16, v2(10, sy), v2(200, 24), SetSkyGSlider, SliderLabelInteger);
		sy += szy + sp;
		CreateSlider("Sky B", 0, 255, level_skyB, 1, 16, v2(10, sy), v2(200, 24), SetSkyBSlider, SliderLabelInteger);
		sy += szy + sp;
		CreateSlider("Music",
					 0,
					 MUSIC_COUNT,
					 level_musicID,
					 1,
					 1,
					 v2(10, sy),
					 v2(200, 24),
					 SetMusicSlider,
					 SliderLabelInteger);
	}
}

void BtnLoad()
{
	// clear all editor nodes
	ListFreeWithData(editorNodes);
	editorNodes = CreateList();

	const Level *l = GetState()->level;

	// load level properties
	level_fogR = l->fogColor >> 16;
	level_fogG = l->fogColor >> 8;
	level_fogB = l->fogColor;
	level_fogStart = l->fogStart;
	level_fogEnd = l->fogEnd;
	level_ceilTex = l->ceilingTexture;
	level_floorTex = l->floorTexture;
	level_skyR = l->skyColor >> 16;
	level_skyG = l->skyColor >> 8;
	level_skyB = l->skyColor;
	level_musicID = l->musicID;

	// add a node for the player
	EditorNode *playerNode = malloc(sizeof(EditorNode));
	chk_malloc(playerNode);
	playerNode->type = NODE_PLAYER;
	playerNode->position = l->player.pos;
	playerNode->rotation = fmod(l->player.angle, 2 * PI);
	ListAdd(editorNodes, playerNode);

	// add a node for each actor
	for (int i = 0; i < ListGetSize(l->actors); i++)
	{
		const Actor *a = ListGet(l->actors, i);
		EditorNode *actorNode = malloc(sizeof(EditorNode));
		chk_malloc(actorNode);
		actorNode->type = NODE_ACTOR;
		actorNode->position = a->position;
		actorNode->rotation = fmod(a->rotation, 2 * PI);
		actorNode->index = i;
		actorNode->extra = a->actorType;
		uint extra2 = 0;
		extra2 |= (a->paramA & 0xFF) << 24;
		extra2 |= (a->paramB & 0xFF) << 16;
		extra2 |= (a->paramC & 0xFF) << 8;
		extra2 |= a->paramD & 0xFF;
		actorNode->extra2 = extra2;
		ListAdd(editorNodes, actorNode);
	}

	// add a node for each wall
	for (int i = 0; i < ListGetSize(l->walls); i++)
	{
		const Wall *w = ListGet(l->walls, i);
		EditorNode *wallNodeA = malloc(sizeof(EditorNode));
		chk_malloc(wallNodeA);
		wallNodeA->type = NODE_WALL_A;
		wallNodeA->index = i;
		wallNodeA->position = w->a;
		wallNodeA->extra = w->texId;
		wallNodeA->extra2 = w->uvScale;
		ListAdd(editorNodes, wallNodeA);

		EditorNode *wallNodeB = malloc(sizeof(EditorNode));
		chk_malloc(wallNodeB);
		wallNodeB->type = NODE_WALL_B;
		wallNodeB->index = i;
		wallNodeB->position = w->b;
		ListAdd(editorNodes, wallNodeB);
	}
}

void BtnTest()
{
	Level *l = NodesToLevel();
	ChangeLevel(l);
	GMainStateSet();
}

#pragma endregion

#pragma region Mode Update Functions

void UpdateMoveMode()
{
	// check if we are hovering over a node
	for (int i = 0; i < editorNodes->size; i++)
	{
		const EditorNode *node = ListGet(editorNodes, i);
		const Vector2 screenPos = v2(node->position.x * editorZoom + editorPanX,
									 node->position.y * editorZoom + editorPanY);

		bool hovered = false;
		const Vector2 mousePos = GetMousePos();
		if (mousePos.x >= screenPos.x - 5 &&
			mousePos.x <= screenPos.x + 5 &&
			mousePos.y >= screenPos.y - 5 &&
			mousePos.y <= screenPos.y + 5)
		{
			hovered = true;
		}

		if (hovered && IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
		{
			editorSelectedNode = i;
		}
	}

	if (IsMouseButtonJustReleased(SDL_BUTTON_LEFT))
	{
		editorSelectedNode = -1;
	}

	// move the selected node to the mouse position
	if (editorSelectedNode != -1)
	{
		EditorNode *node = ListGet(editorNodes, editorSelectedNode);
		const Vector2 mousePos = GetMousePos();
		node->position = v2((mousePos.x - editorPanX) / editorZoom, (mousePos.y - editorPanY) / editorZoom);

		if (editorSnapToGrid)
		{
			node->position.x = round(node->position.x);
			node->position.y = round(node->position.y);
		}
	}
}

void UpdateDeleteMode()
{
	for (int i = 0; i < editorNodes->size; i++)
	{
		const EditorNode *node = ListGet(editorNodes, i);
		const Vector2 screenPos = v2(node->position.x * editorZoom + editorPanX,
									 node->position.y * editorZoom + editorPanY);

		bool hovered = false;
		const Vector2 mousePos = GetMousePos();
		if (mousePos.x >= screenPos.x - 5 &&
			mousePos.x <= screenPos.x + 5 &&
			mousePos.y >= screenPos.y - 5 &&
			mousePos.y <= screenPos.y + 5)
		{
			hovered = true;
		}

		if (hovered && IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
		{
			if (node->type != NODE_PLAYER)
			{
				ListRemoveAt(editorNodes, i);
				// if we are deleting a wall, we need to delete both nodes
				if (node->type == NODE_WALL_A)
				{
					ListRemoveAt(editorNodes, i);
				} else if (node->type == NODE_WALL_B)
				{
					ListRemoveAt(editorNodes, i - 1);
				}
			}
			break; // don't delete more than one node per click
		}
	}
}

void AddModeWall()
{
	if (IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
	{
		const Vector2 mousePos = GetMousePos();
		Vector2 worldPos = v2((mousePos.x - editorPanX) / editorZoom, (mousePos.y - editorPanY) / editorZoom);

		if (editorSnapToGrid)
		{
			worldPos.x = round(worldPos.x);
			worldPos.y = round(worldPos.y);
		}

		const Control *texSld = ListGet(editorUiStack->Controls, 1 + editorBaseControlCount);
		const SliderData *texData = (SliderData *)texSld->ControlData;
		const int tex = texData->value;


		// Create 2 nodes for a wall
		EditorNode *nodeA = malloc(sizeof(EditorNode));
		chk_malloc(nodeA);
		nodeA->type = NODE_WALL_A;
		nodeA->position = worldPos;
		nodeA->extra = tex;
		const float uv = 1.0;
		nodeA->extra2 = uv;
		ListAdd(editorNodes, nodeA);

		EditorNode *nodeB = malloc(sizeof(EditorNode));
		chk_malloc(nodeB);
		nodeB->type = NODE_WALL_B;
		nodeB->position = worldPos;
		ListAdd(editorNodes, nodeB);

		isAddModeDragging = true;
	} else if (IsMouseButtonJustReleased(SDL_BUTTON_LEFT))
	{
		isAddModeDragging = false;
	} else if (isAddModeDragging)
	{
		// delete the 2 nodes we just created if escape is pressed
		if (IsKeyJustPressed(SDL_SCANCODE_ESCAPE))
		{
			ListRemoveAt(editorNodes, editorNodes->size - 1);
			ListRemoveAt(editorNodes, editorNodes->size - 1);
			isAddModeDragging = false;
			return;
		}

		const Vector2 mousePos = GetMousePos();
		Vector2 worldPos = v2((mousePos.x - editorPanX) / editorZoom, (mousePos.y - editorPanY) / editorZoom);

		if (editorSnapToGrid)
		{
			worldPos.x = round(worldPos.x);
			worldPos.y = round(worldPos.y);
		}

		EditorNode *nodeB = ListGet(editorNodes, editorNodes->size - 1);
		nodeB->position = worldPos;
	}
}

void AddModeActor()
{
	if (IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
	{
		EditorNode *node = malloc(sizeof(EditorNode));
		chk_malloc(node);
		node->type = NODE_ACTOR;
		const Vector2 mousePos = GetMousePos();
		Vector2 worldPos = v2((mousePos.x - editorPanX) / editorZoom, (mousePos.y - editorPanY) / editorZoom);

		if (editorSnapToGrid)
		{
			worldPos.x = round(worldPos.x);
			worldPos.y = round(worldPos.y);
		}

		node->position = worldPos;
		node->rotation = 0;
		node->extra = 1;
		node->extra2 = 0;
		ListAdd(editorNodes, node);
	}
}

void UpdateAddMode()
{
	const Control *modeSld = ListGet(editorUiStack->Controls, 0 + editorBaseControlCount);
	const SliderData *sldData = (SliderData *)modeSld->ControlData;
	const int mode = sldData->value;
	switch (mode)
	{
		case 0:
		{
			AddModeWall();
			break;
		}
		case 1:
		{
			AddModeActor();
			break;
		}
		default:
			break;
	}
}

void UpdatePropertiesMode()
{
	for (int i = 0; i < editorNodes->size; i++)
	{
		const EditorNode *node = ListGet(editorNodes, i);
		const Vector2 screenPos = v2(node->position.x * editorZoom + editorPanX,
									 node->position.y * editorZoom + editorPanY);

		bool hovered = false;
		const Vector2 mousePos = GetMousePos();
		if (mousePos.x >= screenPos.x - 5 &&
			mousePos.x <= screenPos.x + 5 &&
			mousePos.y >= screenPos.y - 5 &&
			mousePos.y <= screenPos.y + 5)
		{
			hovered = true;
		}

		if (hovered && IsMouseButtonJustPressed(SDL_BUTTON_LEFT))
		{
			editorSelectedNode = i;
		}
	}

	while (editorUiStack->Controls->size > editorBaseControlCount)
	{
		UiStackRemove(editorUiStack, ListGet(editorUiStack->Controls, editorBaseControlCount));
	}

	const EditorNode *node = ListGet(editorNodes, editorSelectedNode);
	switch (node->type)
	{
		case NODE_PLAYER:
			CreateSlider("ang",
						 0,
						 359,
						 radToDeg(node->rotation),
						 1,
						 45,
						 v2(10, 250),
						 v2(200, 24),
						 SetNodeRotationSlider,
						 NULL);
			break;
		case NODE_ACTOR:
			CreateSlider("ang",
						 0,
						 359,
						 radToDeg(node->rotation),
						 1,
						 45,
						 v2(10, 250),
						 v2(200, 24),
						 SetNodeRotationSlider,
						 NULL);
			CreateSlider("Type",
						 0,
						 GetActorTypeCount() - 1,
						 node->extra,
						 1,
						 16,
						 v2(10, 280),
						 v2(200, 24),
						 SetNodeExtraSlider,
						 SliderActorNameLabelCallback);
			CreateSlider(GetActorParamName(node->extra, 0),
						 0,
						 255,
						 node->extra2 >> 24 & 0xFF,
						 1,
						 16,
						 v2(10, 310),
						 v2(200, 24),
						 SetActorParamASlider,
						 SliderLabelInteger);
			CreateSlider(GetActorParamName(node->extra, 1),
						 0,
						 255,
						 node->extra2 >> 16 & 0xFF,
						 1,
						 16,
						 v2(10, 340),
						 v2(200, 24),
						 SetActorParamBSlider,
						 SliderLabelInteger);
			CreateSlider(GetActorParamName(node->extra, 2),
						 0,
						 255,
						 node->extra2 >> 8 & 0xFF,
						 1,
						 16,
						 v2(10, 370),
						 v2(200, 24),
						 SetActorParamCSlider,
						 SliderLabelInteger);
			CreateSlider(GetActorParamName(node->extra, 3),
						 0,
						 255,
						 node->extra2 & 0xFF,
						 1,
						 16,
						 v2(10, 400),
						 v2(200, 24),
						 SetActorParamDSlider,
						 SliderLabelInteger);
			break;
		case NODE_WALL_A:
			CreateSlider("Tex",
						 0,
						 WALL_TEXTURE_COUNT - 1,
						 node->extra,
						 1,
						 16,
						 v2(10, 250),
						 v2(200, 24),
						 SetNodeExtraSlider,
						 SliderLabelInteger);
			break;
		default:
			break;
	}
}

#pragma endregion

void GEditorStateUpdate(GlobalState * /*State*/)
{
#ifdef ENABLE_LEVEL_EDITOR
	if (IsKeyJustPressed(SDL_SCANCODE_F6))
	{
		GMainStateSet();
	}

	if (IsMouseButtonPressed(SDL_BUTTON_RIGHT))
	{
		const Vector2 mouseDelta = GetMouseRel();
		editorPanX += mouseDelta.x;
		editorPanY += mouseDelta.y;
	}

	if (editorUiStack->ActiveControl != -1)
	{
		return; // eat input if we are interacting with a control
	}

	switch (currentEditorMode)
	{
		case EDITOR_MODE_MOVE:
		{
			UpdateMoveMode();
			break;
		}
		case EDITOR_MODE_DELETE:
		{
			UpdateDeleteMode();
			break;
		}
		case EDITOR_MODE_ADD:
		{
			UpdateAddMode();
			break;
		}
		case EDITOR_MODE_PROPERTIES:
		{
			UpdatePropertiesMode();
			break;
		}
		case EDITOR_MODE_LEVEL:
		{
			break;
		}
		default:
		{
			Error("Invalid editor mode");
		}
	}
#endif
}

void GEditorStateRender(GlobalState * /*State*/)
{
#ifdef ENABLE_LEVEL_EDITOR
	SetColorUint(0xFF123456);
	ClearColor(0xFF123456);

	RenderGrid();

	int hoveredNode = -1;
	for (int i = 0; i < editorNodes->size; i++)
	{
		DrawNode(ListGet(editorNodes, i), &hoveredNode, i);
	}

	if (hoveredNode != -1)
	{
		DrawNodeTooltip(ListGet(editorNodes, hoveredNode));
	}

	ProcessUiStack(editorUiStack);
	DrawUiStack(editorUiStack);

	// if we are editing the properties of a wall a node, draw its texture
	if (currentEditorMode == EDITOR_MODE_PROPERTIES && editorSelectedNode != -1)
	{
		const EditorNode *node = ListGet(editorNodes, editorSelectedNode);
		if (node->type == NODE_WALL_A)
		{
			const byte *tex = wallTextures[node->extra];
			if (tex != NULL)
			{
				const SDL_Rect dst = {10, 310, 64, 64};
				DrawTexture(v2(dst.x, dst.y), v2(dst.w, dst.h), tex);
			}
		}
	} else if (currentEditorMode == EDITOR_MODE_ADD)
	{
		const Control *texSld = ListGet(editorUiStack->Controls, 1 + editorBaseControlCount);
		const SliderData *sliderData = (SliderData *)texSld->ControlData;

		const byte *tex = wallTextures[(int)sliderData->value];
		if (tex != NULL)
		{
			const SDL_Rect dst = {10, 360, 64, 64};
			DrawTexture(v2(dst.x, dst.y), v2(dst.w, dst.h), tex);
		}
	}

#endif
}

void GEditorStateSet()
{
	if (!editorInitComplete)
	{
		// center the view to 0,0
		editorPanX = WindowWidth() / 2.0;
		editorPanY = WindowHeight() / 2.0;

		editorUiStack = CreateUiStack();
		editorNodes = CreateList();
		// will be freed immediately after this function, but we create it here to avoid nullptr free

		int sx = 10;
		const int szy = 30;
		const int szx = 120;
		const int sp = 0;
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

		UiStackPush(editorUiStack,
					CreateCheckboxControl(v2(10, 140),
										  v2(120, 30),
										  "Snap",
										  ToggleSnapToGrid,
										  TOP_LEFT,
										  editorSnapToGrid));

		UiStackPush(editorUiStack, CreateButtonControl(v2(-60, 10), v2(120, 24), "Load", BtnLoad, TOP_RIGHT));
		UiStackPush(editorUiStack, CreateButtonControl(v2(-60, 40), v2(120, 24), "Test", BtnTest, TOP_RIGHT));
		UiStackPush(editorUiStack, CreateButtonControl(v2(-60, 70), v2(120, 24), "Save", BtnCopyBytecode, TOP_RIGHT));

		editorBaseControlCount = ListGetSize(editorUiStack->Controls);

		SetEditorMode(false, 0, 0);

		editorInitComplete = true;
	}

	SetStateCallbacks(GEditorStateUpdate, NULL, EDITOR_STATE, GEditorStateRender);
}
