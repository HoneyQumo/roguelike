#pragma once

#include <UiLabel.h>
#include <UiScreen.h>
#include <vector>
#include "SubtitleQueue.h"

namespace RoguelikeGame
{
	/**
	*	Рисует то, что сказано. Правила показа живут в очереди - экран только
	*	берёт её строки, переносит по ширине и раскладывает снизу вверх.
	*
	*	Свой экран, а не плашка HUD: та одна на всех, и реплика затирала бы
	*	название локации.
	*/
	class SubtitleScreen : public XYZEngine::UiScreen
	{
	public:
		SubtitleScreen();

		void SetQueue(const SubtitleQueue* newQueue);
		void Update(float deltaTime) override;

		int GetShownLineCount() const;

	private:
		const SubtitleQueue* queue = nullptr;
		std::vector<XYZEngine::UiLabel*> labels;
		int shownLineCount = 0;

		void Fill();
	};
}
