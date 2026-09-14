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
		void SetTexture(const sf::Texture* newTexture);
		void AddQuad(const Vector2Df& center, const Vector2Df& size, const sf::Color& color);
		void AddQuad(const Vector2Df& center, const Vector2Df& size, const sf::IntRect& frame,
			const sf::Color& tint = sf::Color::White);

		std::size_t GetQuadsCount() const;
		const sf::VertexArray& GetVertices() const;

	private:
		sf::VertexArray vertices{ sf::Quads };
		const sf::Texture* texture = nullptr;
	};
}
