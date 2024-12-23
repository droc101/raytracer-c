//
// Created by droc101 on 11/10/2024.
//

#include "PlatformHelpers.h"

#include "../defines.h"

#ifdef WIN32
#include <dwmapi.h>
#include <SDL_syswm.h>
#include "Core/Logging.h"

#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

void DwmDarkMode(SDL_Window *window)
{
#ifdef WIN32
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	const HWND hWnd = info.info.win.window; // NOLINT(*-misplaced-const)
	const BOOL enable = true;
	const HRESULT res = DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &enable, sizeof(BOOL));
	if (res != S_OK)
	{
		LogWarning("Failed to enable dark mode: %lx\n", res);
	}
#endif
}

#ifdef WIN32
/**
 * Reallocates memory for an array of nmemb elements of size bytes each.
 * @param ptr Pointer to the memory block to be reallocated.
 * @param nmemb Number of elements.
 * @param size Size of each element.
 * @return Pointer to the reallocated memory block.
 * @note Dear Microsoft, Please explode.
 */
void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	if (nmemb == 0 || size == 0)
	{
		return NULL;
	}
	if (nmemb > SIZE_MAX / size)
	{
		return NULL;
	}
	return realloc(ptr, nmemb * size);
}
#endif
