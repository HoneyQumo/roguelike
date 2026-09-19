#pragma once

#include <SFML/Audio.hpp>
#include "SoundSourceComponent.h"

namespace XYZEngine
{
	class AudioComponent : public SoundSourceComponent<sf::Sound>
	{
	public:
		AudioComponent(GameObject* gameObject);

		void SetSound(const sf::SoundBuffer* newSound);
		void Update(float deltaTime) override;
		void Play() override;

	protected:
		sf::Sound* GetSource() override;
		const sf::Sound* GetSource() const override;

	private:
		sf::Sound sound;
	};
}
