//
// Created by droc101 on 4/22/2024.
//

#include "GlobalState.h"
#include <stdio.h>
#include "../Assets/AssetReader.h"
#include "../Assets/Assets.h"
#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"
#include "../Helpers/Core/PhysicsThread.h"
#include "../Helpers/LevelEntries.h"
#include "../Helpers/LevelLoader.h"
#include "../Structs/Level.h"
#include "../Structs/Wall.h"
#include "Camera.h"
#include "Options.h"

GlobalState state;

const byte *music[MUSIC_COUNT] = {
	gzmpg_audio_field,
};

void ChannelFinished(const int channel)
{
	// callback for when a channel finishes playing (so we can free it)
	state.channels[channel] = NULL;
}

void InitState()
{
	state.hp = 100;
	state.maxHp = 100;
	state.ammo = 100;
	state.maxAmmo = 100;
	state.coins = 0;
	state.blueCoins = 0;
	state.physicsFrame = 0;
	state.level = CreateLevel(); // empty level so we don't segfault
	state.requestExit = false;
	state.music = NULL;
	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		state.channels[i] = NULL;
	}
	state.CameraY = 0;
	state.textBoxActive = false;
	state.cam = CreateCamera();
	LoadOptions(&state.options);

	UpdateVolume();

	StopMusic();
	Mix_ChannelFinished(ChannelFinished);
}

void UpdateVolume()
{
	const double sfxVol = state.options.sfxVolume;
	const double musicVol = state.options.musicVolume;
	const double masterVol = state.options.masterVolume;
	Mix_MasterVolume((int)(masterVol * MIX_MAX_VOLUME));
	Mix_Volume(-1, (int)(sfxVol * MIX_MAX_VOLUME));
	Mix_VolumeMusic((int)(musicVol * MIX_MAX_VOLUME));
}

void ShowTextBox(const TextBox tb)
{
	state.textBox = tb;
	state.textBoxPage = 0;
	state.textBoxActive = true;
}

GlobalState *GetState()
{
	return &state;
}

void TakeDamage(const int damage)
{
	state.hp -= damage;
	if (state.hp < 0)
	{
		state.hp = 0;
	}
}

void Heal(const int amount)
{
	state.hp += amount;
	if (state.hp > state.maxHp)
	{
		state.hp = state.maxHp;
	}
}

void AddAmmo(const int amount)
{
	state.ammo += amount;
	if (state.ammo > state.maxAmmo)
	{
		state.ammo = state.maxAmmo;
	}
}

void UseAmmo(const int amount)
{
	state.ammo -= amount;
	if (state.ammo < 0)
	{
		state.ammo = 0;
	}
}

void SetUpdateCallback(const FrameUpdateFunction UpdateGame,
					   const FixedUpdateFunction FixedUpdateGame,
					   const CurrentState currentState)
{
	state.physicsFrame = 0;
	state.UpdateGame = UpdateGame;
	state.currentState = currentState;
	PhysicsThreadSetFunction(FixedUpdateGame);
}

void SetRenderCallback(const FrameRenderFunction RenderGame)
{
	state.RenderGame = RenderGame;
}

void ChangeLevel(Level *l)
{
	DestroyLevel(state.level);
	state.level = l;
	state.textBoxActive = false;
	if (l->MusicID != 0)
	{
		ChangeMusic(music[l->MusicID - 1]);
	} else
	{
		StopMusic();
	}

	for (int i = 0; i < l->walls->size; i++)
	{
		Wall *w = ListGet(l->walls, i);
		WallBake(w);
	}
}

void ChangeMusic(const byte *asset)
{
	if (AssetGetType(asset) != ASSET_TYPE_MP3)
	{
		LogWarning("ChangeMusic Error: Asset is not a music file.\n");
		return;
	}

	StopMusic(); // stop the current music and free its data
	const byte *mp3 = DecompressAsset(asset);
	const uint mp3Size = AssetGetSize(asset);
	Mix_Music *mus = Mix_LoadMUS_RW(SDL_RWFromConstMem(mp3, mp3Size), 1);
	if (mus == NULL)
	{
		printf("Mix_LoadMUS_RW Error: %s\n", Mix_GetError());
		return;
	}
	state.music = mus;
	Mix_FadeInMusic(mus, -1, 500);
}

void StopMusic()
{
	if (state.music != NULL)
	{
		// stop and free the current music
		Mix_HaltMusic();
		Mix_FreeMusic(state.music);
		state.music = NULL; // set to NULL, so we don't free it again if this function fails
	}
}

void PlaySoundEffect(const byte *asset)
{
	if (AssetGetType(asset) != ASSET_TYPE_WAV)
	{
		LogError("PlaySoundEffect Error: Asset is not a sound effect file.\n");
		return;
	}

	const byte *wav = DecompressAsset(asset);
	const uint wavSize = AssetGetSize(asset);
	Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(wav, wavSize), 1);
	if (chunk == NULL)
	{
		LogError("Mix_LoadWAV_RW Error: %s\n", Mix_GetError());
		return;
	}
	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		if (state.channels[i] == NULL)
		{
			state.channels[i] = chunk;
			Mix_PlayChannel(i, chunk, 0);
			return;
		}
	}
	LogError("PlaySoundEffect Error: No available channels.\n");
	Mix_FreeChunk(chunk);
}

void DestroyGlobalState()
{
	SaveOptions(&state.options);
	DestroyLevel(state.level);
	free(state.cam);
	if (state.music != NULL)
	{
		Mix_HaltMusic();
		Mix_FreeMusic(state.music);
	}
	// free sound effects
	for (int i = 0; i < SFX_CHANNEL_COUNT; i++)
	{
		if (state.channels[i] != NULL)
		{
			Mix_FreeChunk(state.channels[i]);
		}
	}
}

void ChangeLevelByID(const int id)
{
	GetState()->levelID = id;
	GetState()->blueCoins = 0;
	const void *levelData = DecompressAsset(gLevelEntries[id].levelData);
	Level *l = LoadLevel(levelData);
	ChangeLevel(l);
}
