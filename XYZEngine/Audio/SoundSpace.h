#pragma once

#include <algorithm>
#include "Vector.h"

namespace XYZEngine
{
	struct SoundPoint
	{
		float x = 0.f;
		float y = 0.f;
		float z = 0.f;
	};

	// Переворот мира по Y уже сделан камерой, а у слушателя SFML ось Y тоже
	// вверх - знак менять не надо, хотя для плоского SFML совет обратный.
	inline SoundPoint ToSoundPoint(const Vector2Df& world)
	{
		return {world.x, world.y, 0.f};
	}

	// Затухание OpenAL: ближе минимальной дистанции громкость полная, дальше падает.
	inline float HearingFactor(float distance, float minDistance, float attenuation)
	{
		if (minDistance <= 0.f || attenuation <= 0.f)
		{
			return 1.f;
		}

		float outside = std::max(distance, minDistance);

		return minDistance / (minDistance + attenuation * (outside - minDistance));
	}

	// Слушатель один на всю программу, поэтому ставит его тот же, кто ставит кадр.
	void SetListenerPosition(const Vector2Df& world);
	Vector2Df GetListenerPosition();
}
