#pragma once

#include <AudioComponent.h>
#include "GameSettings.h"

namespace RoguelikeGame
{
	// Звук на карте: затухает с расстоянием и едет за своим объектом.
	inline void PlaceInWorld(XYZEngine::AudioComponent* audio)
	{
		if (audio == nullptr)
		{
			return;
		}

		audio->SetRelativeToListener(false);
		audio->SetMinDistance(SOUND_FULL_DISTANCE);
		audio->SetAttenuation(SOUND_ATTENUATION);
	}
}
