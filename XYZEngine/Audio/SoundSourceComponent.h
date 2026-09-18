#pragma once

#include <SFML/Audio.hpp>
#include "SoundSource.h"

namespace XYZEngine
{
	template <typename TSource>
	class SoundSourceComponent : public SoundSource
	{
	public:
		SoundSourceComponent(GameObject* gameObject) : SoundSource(gameObject) {}

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

		void Pause() override
		{
			if (TSource* source = GetSource())
			{
				source->pause();
			}
		}

		void Resume() override
		{
			TSource* source = GetSource();
			if (source != nullptr && source->getStatus() == sf::SoundSource::Paused)
			{
				source->play();
			}
		}

		void Stop() override
		{
			if (TSource* source = GetSource())
			{
				source->stop();
			}
		}

		bool IsPlaying() const override
		{
			const TSource* source = GetSource();
			return source != nullptr && source->getStatus() == sf::SoundSource::Playing;
		}

	protected:
		virtual TSource* GetSource() = 0;
		virtual const TSource* GetSource() const = 0;
	};
}
