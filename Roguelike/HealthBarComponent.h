#pragma once

#include <SFML/Graphics.hpp>
#include <Component.h>
#include <TransformComponent.h>
#include "HealthComponent.h"
#include <Vector.h>

namespace RoguelikeGame
{
	class HealthBarComponent : public XYZEngine::Component
	{
	public:
		HealthBarComponent(XYZEngine::GameObject* gameObject);

		void Start() override;
		void Update(float deltaTime) override;
		void Render() override;

		void SetSize(float newWidth, float newHeight);
		void SetOffset(float offsetX, float offsetY);
		void SetColors(const sf::Color& newFillColor, const sf::Color& newBackgroundColor);
		void SetAlwaysVisible(bool newIsAlwaysVisible);
	private:
		XYZEngine::TransformComponent* transform = nullptr;
		HealthComponent* health = nullptr;

		sf::RectangleShape background;
		sf::RectangleShape fill;

		XYZEngine::Vector2Df size = { 48.f, 6.f };
		XYZEngine::Vector2Df offset = { 0.f, 40.f };
		bool isAlwaysVisible = false;
	};
}
