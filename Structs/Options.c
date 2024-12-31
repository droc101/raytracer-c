//
// Created by droc101 on 10/27/24.
//

#include "Options.h"
#include <stdio.h>

#include "../Helpers/Core/Error.h"
#include "../Helpers/Core/Logging.h"

void DefaultOptions(Options *options)
{
	options->renderer = RENDERER_OPENGL;
	options->musicVolume = 1.0;
	options->sfxVolume = 1.0;
	options->masterVolume = 1.0;
	options->fullscreen = false;
	options->vsync = false;
	options->mouseSpeed = 1;
	options->controllerMode = false;
	options->msaa = MSAA_4X;
	options->mipmaps = true;
	options->rumbleStrength = 1.0f;
	options->cameraInvertX = true;
	options->controllerSwapOkCancel = false;
}

bool ValidateOptions(const Options *options)
{
	if (options->renderer >= RENDERER_MAX)
	{
		return false;
	}
	if (options->musicVolume < 0 || options->musicVolume > 1)
	{
		return false;
	}
	if (options->sfxVolume < 0 || options->sfxVolume > 1)
	{
		return false;
	}
	if (options->masterVolume < 0 || options->masterVolume > 1)
	{
		return false;
	}
	if (options->mouseSpeed < 0.01 || options->mouseSpeed > 2.00)
	{
		return false;
	}
	return true;
}

ushort GetOptionsChecksum(Options *options)
{
	const byte *data = (byte *)options;
	ushort checksum = 0;
	for (int i = sizeof(ushort); i < sizeof(Options) - sizeof(ushort); i++)
	{
		checksum += data[i];
	}
	return checksum;
}

char *GetOptionsPath()
{
	char *folderPath = SDL_GetPrefPath(APPDATA_ORG_NAME, APPDATA_APP_NAME);
	const char *fileName = "options.bin";
	char *filePath = malloc(strlen(folderPath) + strlen(fileName) + 1);
	chk_malloc(filePath);
	strcpy(filePath, folderPath);
	strcat(filePath, fileName);

	SDL_free(folderPath);
	return filePath;
}

void LoadOptions(Options *options)
{
	char *filePath = GetOptionsPath();

	FILE *file = fopen(filePath, "rb");
	if (file == NULL)
	{
		LogWarning("Options file not found, using default options\n");
		DefaultOptions(options);
	} else
	{
		fseek(file, 0, SEEK_END);
		const int fileLen = ftell(file);

		// if the file is the wrong size, just use the default options
		if (fileLen != sizeof(Options))
		{
			LogWarning("Options file is invalid, using defaults\n");
			DefaultOptions(options);
			fclose(file);
			free(filePath);
			return;
		}

		LogInfo("Valid options file found, loading options\n");

		fseek(file, 0, SEEK_SET);
		fread(options, sizeof(Options), 1, file);

		if (options->checksum != GetOptionsChecksum(options))
		{
			LogWarning("Options file checksum invalid, using defaults\n");
			DefaultOptions(options);
		}

		if (!ValidateOptions(options))
		{
			LogWarning("Options file is invalid, using defaults\n");
			DefaultOptions(options);
		}

		fclose(file);
	}

	free(filePath);
}

void SaveOptions(Options *options)
{
	options->checksum = GetOptionsChecksum(options);

	char *filePath = GetOptionsPath();

	FILE *file = fopen(filePath, "wb");
	fwrite(options, sizeof(Options), 1, file);
	fclose(file);

	free(filePath);
}
