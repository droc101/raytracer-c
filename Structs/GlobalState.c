//
// Created by droc101 on 4/22/2024.
//

#include <stdio.h>
#include "GlobalState.h"
#include "../Structs/Level.h"
#include "../Assets/AssetReader.h"
#include "../Helpers/LevelEntries.h"
#include "../Helpers/LevelLoader.h"

GlobalState state;

void ChannelFinished(int channel) { // callback for when a channel finishes playing (so we can free it)
    state.channels[channel] = NULLPTR;
}

void InitState() {
    state.hp = 100;
    state.maxHp = 100;
    state.ammo = 100;
    state.maxAmmo = 100;
    state.frame = 0;
    state.level = CreateLevel(); // empty level so we don't segfault
    state.requestExit = false;
    state.music = NULLPTR;
    for (int i = 0; i < SFX_CHANNEL_COUNT; i++) {
        state.channels[i] = NULLPTR;
    }
    state.FakeHeight = 0;
    StopMusic();
    Mix_ChannelFinished(ChannelFinished);
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

void SetUpdateCallback(void (*UpdateGame)()) {
    state.frame = 0;
    state.UpdateGame = UpdateGame;
}

void SetRenderCallback(void (*RenderGame)()) {
    state.RenderGame = RenderGame;
}

const byte *music[] = {
        gzmpg_audio_field
};

void ChangeLevel(Level *l) {
    DestroyLevel(state.level);
    state.level = l;
    if (l->MusicID != -1) {
        ChangeMusic(music[l->MusicID]);
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

    StopMusic(); // stop the current music and free it's data
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
        state.music = NULLPTR; // set to NULL so we don't free it again if this function fails
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
    DestroyLevel(state.level);
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
    void *levelData = DecompressAsset(gLevelEntries[id].levelData);
    Level *l = LoadLevel(levelData);
    GetState()->levelID = id;
    ChangeLevel(l);
}
