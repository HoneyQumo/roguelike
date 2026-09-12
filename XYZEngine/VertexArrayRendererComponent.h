#pragma once

#include <SFML/Graphics.hpp>
#include "Component.h"
#include "Vector.h"

namespace XYZEngine
{
	// Рисует статичную геометрию одним draw.
	// Координаты вершин задаются в мировом пространстве
	class VertexArrayRendererComponent : public Component
	{
	public:
		VertexArrayRendererComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void Clear();
		void AddQuad(const Vector2Df& center, const Vector2Df& size, const sf::Color& color);

		std::size_t GetQuadsCount() const;

	private:
		sf::VertexArray vertices{ sf::Quads };
	};
}
