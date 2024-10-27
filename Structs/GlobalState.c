//
// Created by droc101 on 4/22/2024.
//

#include <stdio.h>
#include "GlobalState.h"
#include "../Structs/Level.h"
#include "../Assets/AssetReader.h"
#include "../Helpers/LevelEntries.h"
#include "../Helpers/LevelLoader.h"
#include "Camera.h"
#include "Options.h"

GlobalState state;

void ChannelFinished(int channel) { // callback for when a channel finishes playing (so we can free it)
    state.channels[channel] = NULLPTR;
}

void InitState() {
    state.hp = 100;
    state.maxHp = 100;
    state.ammo = 100;
    state.maxAmmo = 100;
    state.coins = 0;
    state.blueCoins = 0;
    state.physicsFrame = 0;
    state.level = CreateLevel(); // empty level so we don't segfault
    state.requestExit = false;
    state.music = NULLPTR;
    for (int i = 0; i < SFX_CHANNEL_COUNT; i++) {
        state.channels[i] = NULLPTR;
    }
    state.CameraY = 0;
    state.textBoxActive = false;
    state.cam = CreateCamera();
    LoadOptions(&state.options);

    UpdateVolume();

    StopMusic();
    Mix_ChannelFinished(ChannelFinished);
}

void UpdateVolume() {
    double sfxVol = state.options.sfxVolume;
    double musicVol = state.options.musicVolume;
    sfxVol *= state.options.masterVolume;
    musicVol *= state.options.masterVolume;
    Mix_Volume(-1, (int) (sfxVol * MIX_MAX_VOLUME));
    Mix_VolumeMusic((int) (musicVol * MIX_MAX_VOLUME));
}

void ShowTextBox(TextBox tb) {
    state.textBox = tb;
    state.textBoxPage = 0;
    state.textBoxActive = true;
}

GlobalState *GetState() {
    return &state;
}

void TakeDamage(int damage) {
    state.hp -= damage;
    if (state.hp < 0) {
        state.hp = 0;
    }
}

void Heal(int amount) {
    state.hp += amount;
    if (state.hp > state.maxHp) {
        state.hp = state.maxHp;
    }
}

void AddAmmo(int amount) {
    state.ammo += amount;
    if (state.ammo > state.maxAmmo) {
        state.ammo = state.maxAmmo;
    }
}

void UseAmmo(int amount) {
    state.ammo -= amount;
    if (state.ammo < 0) {
        state.ammo = 0;
    }
}

uint DefaultFixedUpdate(const uint interval, GlobalState* param)
{
    param->physicsFrame++;
    return interval;
}

void SetUpdateCallback(void (* const UpdateGame)(GlobalState* State), uint (* const FixedUpdateGame)(uint interval, GlobalState* State), const CurrentState currentState) {
    state.physicsFrame = 0;
    state.UpdateGame = UpdateGame;
    state.currentState = currentState;
    SDL_RemoveTimer(state.FixedFramerateUpdate);
    if (FixedUpdateGame) // yummy null
    {
        state.FixedFramerateUpdate = SDL_AddTimer(PHYSICS_TARGET_MS, (SDL_TimerCallback)FixedUpdateGame, GetState());
    }
    else
    {
        // ReSharper disable once CppRedundantCastExpression
        state.FixedFramerateUpdate = SDL_AddTimer(PHYSICS_TARGET_MS, (SDL_TimerCallback)DefaultFixedUpdate, GetState());
    }
}

void SetRenderCallback(void (*RenderGame)(GlobalState* State)) {
    state.RenderGame = (void (*)(void *)) RenderGame;
}

const byte *music[MUSIC_COUNT] = {
        gzmpg_audio_field
};

void ChangeLevel(Level *l) {
    DestroyLevel(state.level);
    state.level = l;
    if (l->MusicID != 0) {
        ChangeMusic(music[l->MusicID - 1]);
    } else {
        StopMusic();
    }

    for (int i = 0; i < l->walls->size; i++) {
        Wall *w = (Wall *) ListGet(l->walls, i);
        WallBake(w);
    }

    if (l->staticWalls != NULLPTR) {
        DestroySizedArray(l->staticWalls);
    }
    BakeWallArray(l);
    BakeActorArray(l);
}

void ChangeMusic(const byte *asset) {

    if (AssetGetType(asset) != ASSET_TYPE_MP3) {
        printf("ChangeMusic Error: Asset is not a music file.\n");
        return;
    }

    StopMusic(); // stop the current music and free its data
    byte *mp3 = DecompressAsset(asset);
    uint mp3Size = AssetGetSize(asset);
    Mix_Music *mus = Mix_LoadMUS_RW(SDL_RWFromConstMem(mp3, mp3Size), 1);
    if (mus == NULLPTR) {
        printf("Mix_LoadMUS_RW Error: %s\n", Mix_GetError());
        return;
    }
    state.music = mus;
    Mix_FadeInMusic(mus, -1, 500);
}

void StopMusic() {
    if (state.music != NULLPTR) { // stop and free the current music
        Mix_HaltMusic();
        Mix_FreeMusic(state.music);
        state.music = NULLPTR; // set to NULL, so we don't free it again if this function fails
    }
}

void PlaySoundEffect(byte *asset) {
    if (AssetGetType(asset) != ASSET_TYPE_WAV) {
        printf("PlaySoundEffect Error: Asset is not a sound effect file.\n");
        return;
    }

    byte *wav = DecompressAsset(asset);
    uint wavSize = AssetGetSize(asset);
    Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(wav, wavSize), 1);
    if (chunk == NULLPTR) {
        printf("Mix_LoadWAV_RW Error: %s\n", Mix_GetError());
        return;
    }
    for (int i = 0; i < SFX_CHANNEL_COUNT; i++) {
        if (state.channels[i] == NULLPTR) {
            state.channels[i] = chunk;
            Mix_PlayChannel(i, chunk, 0);
            return;
        }
    }
    printf("PlaySoundEffect Error: No available channels.\n");
    Mix_FreeChunk(chunk);

}

void DestroyGlobalState() {
    SaveOptions(&state.options);
    SDL_RemoveTimer(state.FixedFramerateUpdate);
    DestroyLevel(state.level);
    free(state.cam);
    if (state.music != NULLPTR) {
        Mix_HaltMusic();
        Mix_FreeMusic(state.music);
    }
    // free sound effects
    for (int i = 0; i < SFX_CHANNEL_COUNT; i++) {
        if (state.channels[i] != NULLPTR) {
            Mix_FreeChunk(state.channels[i]);
        }
    }
}

void ChangeLevelByID(int id) {
    GetState()->levelID = id;
    GetState()->blueCoins = 0;
    void *levelData = DecompressAsset(gLevelEntries[id].levelData);
    Level *l = LoadLevel(levelData);
    ChangeLevel(l);
}
