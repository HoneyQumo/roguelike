#pragma once

#include "Component.h"

namespace XYZEngine
{
	// Общий тип для звука и музыки: без него выключить звук миром целиком нечем.
	class SoundSource : public Component
	{
	public:
		SoundSource(GameObject* gameObject) : Component(gameObject) {}

		virtual void Pause() = 0;
		virtual void Resume() = 0;
		virtual void Stop() = 0;
		virtual bool IsPlaying() const = 0;
	};
}
