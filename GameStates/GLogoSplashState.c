//
// Created by droc101 on 7/29/2024.
//

#include "GLogoSplashState.h"
#include "../Helpers/Core/AssetReader.h"
#include "../Helpers/Core/Input.h"
#include "../Helpers/Graphics/Drawing.h"
#include "../Structs/GlobalState.h"
#include "../Structs/Vector2.h"
#include "GMenuState.h"

void GLogoSplashStateFixedUpdate(GlobalState *State, double /*delta*/)
{
#ifdef DEBUG_NOSPLASH
	if (State->physicsFrame == 1)
	{
		GMenuStateSet();
	}
	if (State->physicsFrame > 0)
	{
		State->physicsFrame++;
		return;
	}
#endif

	if (State->physicsFrame == 20)
	{
		PlaySoundEffect(SOUND("sfx_coincling"));
	}

	if (State->physicsFrame == 120)
	{
		GMenuStateSet();
	}

	State->physicsFrame++;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void GLogoSplashStateRender(GlobalState *State)
{
	SetColorUint(0x0);
	ClearColor(0xFF000000);
	if (State->physicsFrame < 20 || State->physicsFrame > 100)
	{
		return;
	}

	const SDL_Rect destRect = {WindowWidth() / 2 - 150, WindowHeight() / 2 - 150, 300, 300};
	DrawTexture(v2(destRect.x, destRect.y), v2(destRect.w, destRect.h), TEXTURE("interface_studio"));
}

void GLogoSplashStateSet()
{
	SetStateCallbacks(NULL,
					  GLogoSplashStateFixedUpdate,
					  LOGO_SPLASH_STATE,
					  GLogoSplashStateRender); // Non-fixed is not needed for this state
}
