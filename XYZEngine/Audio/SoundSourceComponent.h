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
		void SetRelativeToListener(bool newIsRelativeToListener)
		{
			isRelativeToListener = newIsRelativeToListener;
			ApplySpace();
		}

		bool IsRelativeToListener() const { return isRelativeToListener; }

		void SetWorldPosition(const Vector2Df& newWorldPosition)
		{
			worldPosition = newWorldPosition;
			ApplySpace();
		}

		const Vector2Df& GetWorldPosition() const { return worldPosition; }

		void SetMinDistance(float newMinDistance)
		{
			minDistance = newMinDistance;
			ApplySpace();
		}

		float GetMinDistance() const { return minDistance; }

		void SetAttenuation(float newAttenuation)
		{
			attenuation = newAttenuation;
			ApplySpace();
		}

		float GetAttenuation() const { return attenuation; }

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

		// Источник может смениться под компонентом - музыка живёт в ресурсах и
		// приходит указателем. Настройки при этом остаются компонентовы.
		void ApplySpace()
		{
			TSource* source = GetSource();
			if (source == nullptr)
			{
				return;
			}

			SoundPoint point = ToSoundPoint(worldPosition);

			source->setRelativeToListener(isRelativeToListener);
			source->setPosition(point.x, point.y, point.z);
			source->setMinDistance(minDistance);
			source->setAttenuation(attenuation);
		}

	private:
		Vector2Df worldPosition = {0.f, 0.f};
		bool isRelativeToListener = false;
		float minDistance = 1.f;
		float attenuation = 1.f;
	};
}
