#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <cstddef>
#include <vector>
#include "Vector.h"

namespace XYZEngine
{
	constexpr std::size_t DEFAULT_SOUND_SLOTS = 24;
	constexpr int ANY_SOUND_CATEGORY = 0;

	// Место занято, пока звук не доиграл. Время считаем сами: спрашивать OpenAL
	// нельзя - без звуковой карты он отвечает «молчу» на всё.
	struct SoundSlot
	{
		int category = ANY_SOUND_CATEGORY;
		float startedAt = 0.f;
		float endsAt = 0.f;
	};

	// -1, когда все места заняты чужими категориями: чужой звук не обрываем.
	int ChooseSoundSlot(const std::vector<SoundSlot>& slots, float now, int category);

	/**
	*	Общий пул для звуков, которые начинаются и заканчиваются сами: выстрелы,
	*	попадания, крики, реплики. Их не останавливают и не зацикливают, поэтому
	*	отдельный источник каждому не нужен.
	*
	*	Остановить отданный пулу звук нельзя. Всё, что надо уметь обрывать или
	*	зацикливать, остаётся на своём AudioComponent.
	*/
	class SoundPool
	{
	public:
		static SoundPool* Instance();

		void SetSlots(std::size_t count);
		std::size_t GetSlots() const;

		void PlayAt(const sf::SoundBuffer* buffer, const Vector2Df& at, float volume, int category,
			float minDistance, float attenuation);
		void PlayAtListener(const sf::SoundBuffer* buffer, float volume, int category);

		void Clear();

		const std::vector<SoundSlot>& GetBusySlots() const;

	private:
		SoundPool() { SetSlots(DEFAULT_SOUND_SLOTS); }
		~SoundPool() {}

		SoundPool(const SoundPool&) = delete;
		SoundPool& operator=(const SoundPool&) = delete;

		int Take(const sf::SoundBuffer* buffer, float volume, int category);

		std::vector<sf::Sound> sounds;
		std::vector<SoundSlot> slots;
	};
}
