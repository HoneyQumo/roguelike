#pragma once

#include <string>

namespace RoguelikeGame
{
	constexpr auto STEP_KEY_PREFIX = "step_";
	constexpr auto STEP_AUDIO_PATH = "Resources/Audio/Steps/";
	constexpr auto STEP_AUDIO_SUFFIX = ".wav";

	// Несколько вариантов подряд, чтобы шаг три раза в секунду не читался петлёй.
	constexpr int STEP_VARIANTS = 4;

	inline std::string StepKey(int variant)
	{
		return variant < 1 || variant > STEP_VARIANTS
			? std::string()
			: STEP_KEY_PREFIX + std::to_string(variant);
	}

	inline std::string StepFilePath(int variant)
	{
		std::string key = StepKey(variant);

		return key.empty() ? key : STEP_AUDIO_PATH + key + STEP_AUDIO_SUFFIX;
	}
}
