//
// Created by droc101 on 10/27/24.
//

#include "Options.h"

void DefaultOptions(Options *options) {
    options->renderer = 0;
    options->musicVolume = 1.0;
    options->sfxVolume = 1.0;
    options->masterVolume = 1.0;
    options->uiScale = 1;
    options->fullscreen = false;
}

char *GetOptionsPath() {
    char *folderPath = SDL_GetPrefPath(APPDATA_ORG_NAME, APPDATA_APP_NAME);
    char *fileName = "options.bin";
    char *filePath = malloc(strlen(folderPath) + strlen(fileName) + 1);
    strcpy(filePath, folderPath);
    strcat(filePath, fileName);

    free(folderPath);
    return filePath;
}

void LoadOptions(Options *options) {
    char *filePath = GetOptionsPath();

    FILE *file = fopen(filePath, "rb");
    if (file == NULL) {
        printf("Options file not found, using default options\n");
        DefaultOptions(options);
    } else {
        int fileLen = 0;
        fseek(file, 0, SEEK_END);
        fileLen = ftell(file);

        // if the file is the wrong size, just use the default options
        if (fileLen != sizeof(Options)) {
            printf("Options file is invalid, using defaults\n");
            DefaultOptions(options);
            fclose(file);
            free(filePath);
            return;
        }

        printf("Valid options file found, loading options\n");

        fseek(file, 0, SEEK_SET);
        fread(options, sizeof(Options), 1, file);
        fclose(file);
    }

    free(filePath);
}

void SaveOptions(Options *options) {
    char *filePath = GetOptionsPath();

    FILE *file = fopen(filePath, "wb");
    fwrite(options, sizeof(Options), 1, file);
    fclose(file);

    free(filePath);
}
