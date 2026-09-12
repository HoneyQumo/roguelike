#pragma once

#include <SFML/Audio.hpp>
#include "SoundSourceComponent.h"

namespace XYZEngine
{
	class MusicComponent : public SoundSourceComponent<sf::Music>
	{
	public:
		MusicComponent(GameObject* gameObject);
		~MusicComponent();

		void SetMusic(sf::Music* newMusic);
		void Play() override;

	protected:
		sf::Music* GetSource() override;
		const sf::Music* GetSource() const override;

	private:
		sf::Music* music = nullptr;
	};
}
