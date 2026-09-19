#pragma once

#include <AudioComponent.h>
#include <GameObject.h>
#include <SoundPool.h>
#include "GameSettings.h"

namespace RoguelikeGame
{
	// Свой звук вытесняет из пула только свой: выстрел не съедает чужой крик.
	enum class SoundKind : int
	{
		Shot = 1,
		Hit = 2,
		Voice = 3,
		Hurt = 4,
		Step = 5
	};

	// Звуки самого игрока слышны на слушателе, а он стоит на камере: та отстаёт
	// от героя и упирается в край карты, так что в мире они бы гуляли.
	enum class SoundPlace
	{
		AtListener,
		InWorld
	};

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

	struct SoundReach
	{
		float fullDistance = SOUND_FULL_DISTANCE;
		float attenuation = SOUND_ATTENUATION;
	};

	// Каждый вид разносится по-своему: выстрел через полкарты, шаг - на пару
	// шагов. Новый звук объявляет дальность сам, а не наследует чужую молча.
	inline SoundReach ReachOf(SoundKind kind)
	{
		if (kind == SoundKind::Step)
		{
			return {STEP_FULL_DISTANCE, STEP_ATTENUATION};
		}

		return {};
	}

	// Одноразовый звук: его не останавливают и не зацикливают, поэтому ему
	// хватает места в общем пуле, а не своего источника.
	inline void PlayOneShot(const sf::SoundBuffer* buffer, float volume, SoundKind kind, SoundPlace place,
		XYZEngine::GameObject* source)
	{
		if (place == SoundPlace::AtListener || source == nullptr)
		{
			XYZEngine::SoundPool::Instance()->PlayAtListener(buffer, volume, static_cast<int>(kind));
			return;
		}

		SoundReach reach = ReachOf(kind);

		XYZEngine::SoundPool::Instance()->PlayAt(buffer, source->GetTransform()->GetWorldPosition(), volume,
			static_cast<int>(kind), reach.fullDistance, reach.attenuation);
	}
}
