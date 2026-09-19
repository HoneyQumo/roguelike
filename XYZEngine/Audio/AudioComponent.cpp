#include "pch.h"
#include "AudioComponent.h"
#include "LoggerRegistry.h"

namespace XYZEngine
{
	// Пока источник не поставили на карту, он звучит на слушателе - как и звучал,
	// когда пространства не было вовсе.
	AudioComponent::AudioComponent(GameObject* gameObject) : SoundSourceComponent(gameObject)
	{
		SetRelativeToListener(true);
	}

	void AudioComponent::SetSound(const sf::SoundBuffer* newSound)
	{
		if (newSound == nullptr)
		{
			LOG_WARN("Can't set empty sound.");
			return;
		}

		sound.setBuffer(*newSound);
	}

	void AudioComponent::Play()
	{
		if (sound.getBuffer() == nullptr)
		{
			LOG_WARN("Can't play sound without buffer.");
			return;
		}

		SoundSourceComponent::Play();
	}

	sf::Sound* AudioComponent::GetSource()
	{
		return &sound;
	}
	const sf::Sound* AudioComponent::GetSource() const
	{
		return &sound;
	}
}
