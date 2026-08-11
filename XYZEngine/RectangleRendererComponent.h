#pragma once

#include <SFML/Graphics.hpp>
#include "Component.h"
#include "TransformComponent.h"
#include "Vector.h"

namespace XYZEngine
{
	class RectangleRendererComponent : public Component
	{
	public:
		RectangleRendererComponent(GameObject* gameObject);
		~RectangleRendererComponent();

		void Update(float deltaTime) override;
		void Render() override;

		void SetSize(float newWidth, float newHeight);
		Vector2Df GetSize() const;

		void SetColor(const sf::Color& newColor);
	private:
		sf::RectangleShape* rectangle;
		TransformComponent* transform;

		Vector2Df size = { 0.f, 0.f };
	};
}
