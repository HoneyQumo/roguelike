#include "pch.h"
#include "SoundSpace.h"
#include <SFML/Audio/Listener.hpp>

namespace XYZEngine
{
	void SetListenerPosition(const Vector2Df& world)
	{
		SoundPoint point = ToSoundPoint(world);

		sf::Listener::setPosition(point.x, point.y, point.z);
	}

	Vector2Df GetListenerPosition()
	{
		sf::Vector3f position = sf::Listener::getPosition();

		return {position.x, position.y};
	}
}
