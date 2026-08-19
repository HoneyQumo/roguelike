#pragma once

#include <SFML/Graphics.hpp>

namespace XYZEngine
{
	class ColliderComponent;
	struct Collision
	{
	public:
		Collision(ColliderComponent* newFirst, ColliderComponent* newSecond, sf::FloatRect newCollisionRect) :
			first(newFirst), second(newSecond), collisionRect(newCollisionRect) {
		};

		ColliderComponent* GetFirst() const { return first; }
		ColliderComponent* GetSecond() const { return second; }
	private:
		ColliderComponent* first;
		ColliderComponent* second;
		sf::FloatRect collisionRect;
	};
}