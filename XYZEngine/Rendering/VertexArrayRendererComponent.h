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

		// Четырёхугольник по углам: лента и любая другая геометрия не по осям.
		void AddQuad(const Vector2Df& first, const Vector2Df& second, const Vector2Df& third, const Vector2Df& fourth,
			const sf::Color& color);
		void AddQuad(const Vector2Df& center, const Vector2Df& size, const sf::IntRect& frame,
			const sf::Color& tint = sf::Color::White);

		// Меняет кадр уже добавленного квада, не трогая его положение.
		void SetQuadFrame(std::size_t quad, const sf::IntRect& frame);

		std::size_t GetQuadsCount() const;
		const sf::VertexArray& GetVertices() const;

	private:
		sf::VertexArray vertices{ sf::Quads };
		const sf::Texture* texture = nullptr;
	};
}
