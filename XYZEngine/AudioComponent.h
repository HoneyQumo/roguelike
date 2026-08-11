#pragma once

#include <SFML/Audio.hpp>
#include "Component.h"

namespace XYZEngine
{
	class AudioComponent : public Component
	{
	public:
		AudioComponent(GameObject* gameObject);
		~AudioComponent();

		void Update(float deltaTime) override;
		void Render() override;

		void SetSound(const sf::SoundBuffer* newSound);
		void SetLoop(bool isLooped);
		void SetVolume(float newVolume);

		void Play();
		void Pause();
		void Resume();
		void Stop();

		bool IsPlaying() const;
	private:
		sf::Sound* sound;
	};
}
