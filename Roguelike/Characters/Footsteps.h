#pragma once

#include <string>
#include "GameSettings.h"

namespace RoguelikeGame
{
	constexpr auto STEP_KEY_PREFIX = "step_";

	// Набор выбирается по имени: дальше у тяжёлого и у щитовика шаги будут
	// свои, и добавиться они должны строкой в каталоге, а не правкой кода.
	constexpr auto HERO_STEPS = "hero";
	constexpr auto FOE_STEPS = "boot";
	constexpr auto STEP_AUDIO_PATH = "Resources/Audio/Steps/";
	constexpr auto STEP_AUDIO_SUFFIX = ".wav";

	// Несколько вариантов подряд, чтобы шаг три раза в секунду не читался петлёй.
	constexpr int STEP_VARIANTS = 4;

	// След оставляет только бег: ходьба - это тишина, и на ней держится
	// заход со спины, которому учит вся первая локация.
	constexpr float RUN_NOISE_RADIUS = 5.f * TILE_SIZE;
	constexpr float RUN_NOISE_LOUDNESS = 0.5f;

	inline std::string StepKey(const char* set, int variant)
	{
		return set == nullptr || variant < 1 || variant > STEP_VARIANTS
			? std::string()
			: STEP_KEY_PREFIX + std::string(set) + "_" + std::to_string(variant);
	}

	// Касание земли приходится на первый кадр каждой половины цикла: восемь
	// кадров - это два шага, а не восемь.
	inline bool IsStepFrame(int frame, int framesInClip)
	{
		if (framesInClip < 2 || frame < 0 || frame >= framesInClip)
		{
			return false;
		}

		return frame % (framesInClip / 2) == 0;
	}

	// Кадр держится несколько тиков подряд, поэтому одного «это кадр касания»
	// мало - иначе один шаг звучал бы очередью.
	struct StepBeat
	{
		int lastFrame = -1;
	};

	inline bool TakeStep(StepBeat& beat, int frame, int framesInClip)
	{
		if (frame == beat.lastFrame)
		{
			return false;
		}

		beat.lastFrame = frame;

		return IsStepFrame(frame, framesInClip);
	}

	inline void LoseStep(StepBeat& beat)
	{
		beat.lastFrame = -1;
	}

	inline std::string StepFilePath(const char* set, int variant)
	{
		std::string key = StepKey(set, variant);

		return key.empty() ? key : STEP_AUDIO_PATH + key + STEP_AUDIO_SUFFIX;
	}
}
