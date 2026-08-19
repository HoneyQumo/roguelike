#include "pch.h"
#include "MusicComponent.h"
#include <iostream>

namespace XYZEngine
{
	MusicComponent::MusicComponent(GameObject* gameObject) : Component(gameObject) {}

	MusicComponent::~MusicComponent()
	{
		Stop();
	}

	void MusicComponent::Update(float deltaTime)
	{

	}
	void MusicComponent::Render()
	{

	}

	// Track is owned by ResourceSystem: it is streamed from disk, so it can't be copied per component.
	void MusicComponent::SetMusic(sf::Music* newMusic)
	{
		if (newMusic == nullptr)
		{
			std::cout << "Can't set empty music." << std::endl;
			return;
		}

		Stop();
		music = newMusic;
	}
	void MusicComponent::SetLoop(bool isLooped)
	{
		if (music == nullptr)
		{
			return;
		}

		music->setLoop(isLooped);
	}
	void MusicComponent::SetVolume(float newVolume)
	{
		if (music == nullptr)
		{
			return;
		}

		music->setVolume(newVolume);
	}

	void MusicComponent::Play()
	{
		if (music == nullptr)
		{
			std::cout << "Can't play empty music." << std::endl;
			return;
		}

		music->play();
	}
	void MusicComponent::Pause()
	{
		if (music == nullptr)
		{
			return;
		}

		music->pause();
	}
	void MusicComponent::Resume()
	{
		if (music == nullptr || music->getStatus() != sf::SoundSource::Paused)
		{
			return;
		}

		music->play();
	}
	void MusicComponent::Stop()
	{
		if (music == nullptr)
		{
			return;
		}

		music->stop();
	}

	bool MusicComponent::IsPlaying() const
	{
		return music != nullptr && music->getStatus() == sf::SoundSource::Playing;
	}
}
