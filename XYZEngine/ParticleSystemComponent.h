#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "Component.h"
#include "ParticleSpec.h"
#include "Vector.h"

namespace XYZEngine
{
	struct Particle
	{
		const ParticleSpec* spec = nullptr;
		Vector2Df position = {0.f, 0.f};
		Vector2Df direction = {1.f, 0.f};
		Vector2Df gravityVelocity = {0.f, 0.f};
		float age = 0.f;
		float lifeTime = 0.f;

		bool IsAlive() const;
	};

	class ParticleSystemComponent : public Component
	{
	public:
		ParticleSystemComponent(GameObject* gameObject);
		~ParticleSystemComponent() override;

		void Update(float deltaTime) override;
		void Render() override;

		void SetCapacity(std::size_t newCapacity);
		void SetTexture(const sf::Texture* newTexture);

		void Emit(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction);
		void EmitCount(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction, int count);

		std::size_t GetActiveCount() const;
		std::size_t GetCapacity() const;

		const sf::VertexArray& GetAlphaVertices() const;
		const sf::VertexArray& GetAdditiveVertices() const;

	private:
		std::vector<Particle> particles;
		std::size_t writeIndex = 0;
		std::size_t usedCount = 0;

		const sf::Texture* texture = nullptr;
		sf::VertexArray alphaVertices{sf::Quads};
		sf::VertexArray additiveVertices{sf::Quads};

		void EmitOne(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction);
		void BuildVertices();
		void AppendParticle(const Particle& particle);
	};
}
