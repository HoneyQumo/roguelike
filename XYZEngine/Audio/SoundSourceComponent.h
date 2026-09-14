#pragma once

#include <SFML/Audio.hpp>
#include "Component.h"

namespace XYZEngine
{
	template <typename TSource>
	class SoundSourceComponent : public Component
	{
	public:
		SoundSourceComponent(GameObject* gameObject) : Component(gameObject) {}

		void Update(float deltaTime) override {}
		void Render() override {}

		void SetLoop(bool isLooped)
		{
			if (TSource* source = GetSource())
			{
				source->setLoop(isLooped);
			}
		}

		void SetVolume(float newVolume)
		{
			if (TSource* source = GetSource())
			{
				source->setVolume(newVolume);
			}
		}

		virtual void Play()
		{
			if (TSource* source = GetSource())
			{
				source->play();
			}
		}

		void Pause()
		{
			if (TSource* source = GetSource())
			{
				source->pause();
			}
		}

		void Resume()
		{
			TSource* source = GetSource();
			if (source != nullptr && source->getStatus() == sf::SoundSource::Paused)
			{
				source->play();
			}
		}

		void Stop()
		{
			if (TSource* source = GetSource())
			{
				source->stop();
			}
		}

		bool IsPlaying() const
		{
			const TSource* source = GetSource();
			return source != nullptr && source->getStatus() == sf::SoundSource::Playing;
		}

	protected:
		virtual TSource* GetSource() = 0;
		virtual const TSource* GetSource() const = 0;
	};
}
