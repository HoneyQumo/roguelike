#pragma once

#include <SFML/Audio.hpp>
#include "Component.h"

namespace XYZEngine
{
	class MusicComponent : public Component
	{
	public:
		MusicComponent(GameObject* gameObject);
		~MusicComponent();

		void Update(float deltaTime) override;
		void Render() override;

		void SetMusic(sf::Music* newMusic);
		void SetLoop(bool isLooped);
		void SetVolume(float newVolume);

		void Play();
		void Pause();
		void Resume();
		void Stop();

		bool IsPlaying() const;
	private:
		sf::Music* music = nullptr;
	};
}
