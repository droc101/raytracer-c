//
// Created by droc101 on 7/7/2024.
//

#include "CommonAssets.h"
#include "Core/AssetReader.h"
#include "Graphics/Drawing.h"

const char *wallTextures[WALL_TEXTURE_COUNT] = {
	TEXTURE("level_bricks"),
	TEXTURE("level_cross"),
	TEXTURE("level_wall2"),
	TEXTURE("level_uvtest"),
	TEXTURE("level_floor"),
};

const char *actorTextures[ACTOR_TEXTURE_COUNT] = {
	TEXTURE("actor_iq"),
	TEXTURE("actor_BLOB2"),
	TEXTURE("actor_demon"),
	TEXTURE("actor_monster1"),
	TEXTURE("actor_monster2"),
	TEXTURE("actor_monster3"),
	TEXTURE("actor_key"),
	TEXTURE("actor_coin"),
	TEXTURE("actor_bluecoin"),
	TEXTURE("actor_goal0"),
	TEXTURE("actor_goal1"),
	TEXTURE("actor_door"),
};

Model *skyModel;

void InitCommonAssets()
{
	SetTexParams(TEXTURE("interface_menu_bg_tile"), true, true);
	SetTexParams(TEXTURE("interface_menu_bg_tile_red"), true, true);
	SetTexParams(TEXTURE("vfx_shadow"), false, false);
	SetTexParams(TEXTURE("interface_button"), true, false);
	SetTexParams(TEXTURE("interface_button_hover"), true, false);
	SetTexParams(TEXTURE("interface_button_press"), true, false);
	SetTexParams(TEXTURE("interface_slider"), true, false);
	SetTexParams(TEXTURE("interface_checkbox_checked"), true, false);
	SetTexParams(TEXTURE("interface_checkbox_unchecked"), true, false);
	SetTexParams(TEXTURE("interface_radio_checked"), true, false);
	SetTexParams(TEXTURE("interface_radio_unchecked"), true, false);
	SetTexParams(TEXTURE("interface_slider_thumb"), true, false);
	SetTexParams(TEXTURE("interface_focus_rect"), true, false);
	SetTexParams(TEXTURE("level_sky"), true, true);

	skyModel = LoadModel(MODEL("model_sky"));
}

int FindWallTextureIndex(const char *texture)
{
	for (int i = 0; i < WALL_TEXTURE_COUNT; i++)
	{
		if (strcmp(wallTextures[i], texture) == 0)
		{
			return i;
		}
	}
	return -1;
}

void DestroyCommonAssets()
{
	FreeModel(skyModel);
}
