#include "pch.h"
#include "MusicComponent.h"
#include "LoggerRegistry.h"

namespace XYZEngine
{
	MusicComponent::MusicComponent(GameObject* gameObject) : SoundSourceComponent(gameObject)
	{
	}

	MusicComponent::~MusicComponent()
	{
		Stop();
	}

	// Track is owned by ResourceSystem: it is streamed from disk, so it can't be copied per component.
	void MusicComponent::SetMusic(sf::Music* newMusic)
	{
		if (newMusic == nullptr)
		{
			LOG_WARN("Can't set empty music.");
			return;
		}

		Stop();
		music = newMusic;
		SetRelativeToListener(true);
	}

	void MusicComponent::Play()
	{
		if (music == nullptr)
		{
			LOG_WARN("Can't play empty music.");
			return;
		}

		SoundSourceComponent::Play();
	}

	sf::Music* MusicComponent::GetSource()
	{
		return music;
	}
	const sf::Music* MusicComponent::GetSource() const
	{
		return music;
	}
}
