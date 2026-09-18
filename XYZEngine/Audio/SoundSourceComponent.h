#pragma once

#include <SFML/Audio.hpp>
#include "SoundSource.h"
#include "SoundSpace.h"

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

		// Относительный источник всегда звучит на слушателе: расстояние и
		// панорама к нему не применяются.
		void SetRelativeToListener(bool isRelative)
		{
			if (TSource* source = GetSource())
			{
				source->setRelativeToListener(isRelative);
			}
		}

		bool IsRelativeToListener() const
		{
			const TSource* source = GetSource();
			return source != nullptr && source->isRelativeToListener();
		}

		void SetWorldPosition(const Vector2Df& world)
		{
			if (TSource* source = GetSource())
			{
				SoundPoint point = ToSoundPoint(world);
				source->setPosition(point.x, point.y, point.z);
			}
		}

		Vector2Df GetWorldPosition() const
		{
			const TSource* source = GetSource();
			if (source == nullptr)
			{
				return {0.f, 0.f};
			}

			sf::Vector3f position = source->getPosition();
			return {position.x, position.y};
		}

		void SetMinDistance(float distance)
		{
			if (TSource* source = GetSource())
			{
				source->setMinDistance(distance);
			}
		}

		float GetMinDistance() const
		{
			const TSource* source = GetSource();
			return source != nullptr ? source->getMinDistance() : 0.f;
		}

		void SetAttenuation(float attenuation)
		{
			if (TSource* source = GetSource())
			{
				source->setAttenuation(attenuation);
			}
		}

		float GetAttenuation() const
		{
			const TSource* source = GetSource();
			return source != nullptr ? source->getAttenuation() : 0.f;
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
