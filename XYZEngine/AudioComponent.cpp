#include "pch.h"
#include "AudioComponent.h"
#include <iostream>

namespace XYZEngine
{
	AudioComponent::AudioComponent(GameObject* gameObject) : Component(gameObject)
	{
		sound = new sf::Sound();
	}
	AudioComponent::~AudioComponent()
	{
		sound->stop();
		delete sound;
	}

	void AudioComponent::Update(float deltaTime)
	{

	}
	void AudioComponent::Render()
	{

	}

	void AudioComponent::SetSound(const sf::SoundBuffer* newSound)
	{
		if (newSound == nullptr)
		{
			std::cout << "Can't set empty sound." << std::endl;
			return;
		}

		sound->setBuffer(*newSound);
	}
	void AudioComponent::SetLoop(bool isLooped)
	{
		sound->setLoop(isLooped);
	}
	void AudioComponent::SetVolume(float newVolume)
	{
		sound->setVolume(newVolume);
	}

	void AudioComponent::Play()
	{
		if (sound->getBuffer() == nullptr)
		{
			std::cout << "Can't play sound without buffer." << std::endl;
			return;
		}

		sound->play();
	}
	void AudioComponent::Pause()
	{
		sound->pause();
	}
	void AudioComponent::Resume()
	{
		if (sound->getStatus() == sf::SoundSource::Paused)
		{
			sound->play();
		}
	}
	void AudioComponent::Stop()
	{
		sound->stop();
	}

	bool AudioComponent::IsPlaying() const
	{
		return sound->getStatus() == sf::SoundSource::Playing;
	}
}
