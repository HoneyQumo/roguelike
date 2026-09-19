#pragma once

#include <string>
#include "SpeechCatalog.h"
#include "SubtitleQueue.h"
#include "WorldSound.h"

namespace XYZEngine
{
	class GameObject;
}

namespace RoguelikeGame
{
	/**
	*	Одна точка «сказать» на всю игру.
	*
	*	Граница проходит не между репликой врага и репликой катсцены - это один
	*	и тот же разговор с разным поводом. Граница между «кто попросил сказать»
	*	и «как это показывается и звучит»: первое у каждого своё, второе здесь.
	*/
	class SpeechDirector
	{
	public:
		static SpeechDirector& Current();

		void SetCatalog(const SpeechCatalog* newCatalog);
		void SetQueue(SubtitleQueue* newQueue);

		// Конкретная фраза - для катсцен: там говорят по сценарию.
		bool Say(const std::string& lineId, XYZEngine::GameObject* source, SoundPlace place);

		// Любая из подходящих - для врагов: чтобы не повторяться.
		bool SayFromSet(const std::string& setId, XYZEngine::GameObject* source, SoundPlace place);

		// Субтитр не должен пережить то, что его вызвало.
		void Silence();

		int GetSaidCount() const;

	private:
		const SpeechCatalog* catalog = nullptr;
		SubtitleQueue* queue = nullptr;
		int saidCount = 0;
	};
}
