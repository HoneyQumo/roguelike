#pragma once

#include <SFML/Graphics.hpp>
#include "Component.h"
#include "TransformComponent.h"
#include "HealthComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class HealthBarComponent : public Component
	{
	public:
		HealthBarComponent(GameObject* gameObject);
		~HealthBarComponent();

		void Update(float deltaTime) override;
		void Render() override;

		void SetSize(float newWidth, float newHeight);
		void SetOffset(float offsetX, float offsetY);
		void SetColors(const sf::Color& newFillColor, const sf::Color& newBackgroundColor);
	private:
		TransformComponent* transform;
		HealthComponent* health = nullptr;

		sf::RectangleShape* background;
		sf::RectangleShape* fill;

		Vector2Df size = { 48.f, 6.f };
		Vector2Df offset = { 0.f, 40.f };
	};
}
