#include "pch.h"
#include "ParticleSystemComponent.h"
#include "GameObject.h"
#include "MathUtils.h"
#include "ParticleSystem.h"
#include "RenderSystem.h"
#include "randomizer.h"
#include <algorithm>

namespace XYZEngine
{
	namespace
	{
		float Lerp(float from, float to, float progress)
		{
			return from + (to - from) * progress;
		}

		sf::Uint8 LerpChannel(unsigned char from, unsigned char to, float progress)
		{
			float value = Lerp(static_cast<float>(from), static_cast<float>(to), progress);
			return static_cast<sf::Uint8>(std::clamp(value, 0.f, 255.f));
		}

		sf::Color LerpColor(const ParticleColor& from, const ParticleColor& to, float progress)
		{
			return sf::Color(LerpChannel(from.r, to.r, progress), LerpChannel(from.g, to.g, progress),
				LerpChannel(from.b, to.b, progress), LerpChannel(from.a, to.a, progress));
		}
	}

	bool Particle::IsAlive() const
	{
		return spec != nullptr && age < lifeTime;
	}

	ParticleSystemComponent::ParticleSystemComponent(GameObject* gameObject) : Component(gameObject)
	{
		ParticleSystem::Instance()->Register(this);
	}

	ParticleSystemComponent::~ParticleSystemComponent()
	{
		ParticleSystem::Instance()->Unregister(this);
	}

	void ParticleSystemComponent::SetCapacity(std::size_t newCapacity)
	{
		particles.assign(newCapacity, Particle());
		writeIndex = 0;
		usedCount = 0;
	}

	void ParticleSystemComponent::SetTexture(const sf::Texture* newTexture)
	{
		texture = newTexture;
	}

	void ParticleSystemComponent::Emit(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction)
	{
		EmitCount(spec, position, direction, spec.count);
	}

	void ParticleSystemComponent::EmitCount(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction, int count)
	{
		if (particles.empty() || spec.lifeTime <= 0.f || count <= 0)
		{
			return;
		}

		for (int index = 0; index < count; index++)
		{
			EmitOne(spec, position, direction);
		}
	}

	void ParticleSystemComponent::EmitOne(const ParticleSpec& spec, const Vector2Df& position, const Vector2Df& direction)
	{
		Particle& particle = particles[writeIndex];

		particle.spec = &spec;
		particle.position = position;
		particle.direction = RotateByDegrees(direction.Normalized(), random(-spec.spreadDegrees, spec.spreadDegrees));
		particle.gravityVelocity = {0.f, 0.f};
		particle.age = 0.f;
		particle.lifeTime = spec.lifeTime * (1.f - random(0.f, spec.lifeTimeSpread));

		writeIndex = (writeIndex + 1) % particles.size();
		usedCount = std::min(usedCount + 1, particles.size());
	}

	void ParticleSystemComponent::Update(float deltaTime)
	{
		std::size_t aliveCount = 0;

		for (std::size_t index = 0; index < usedCount; index++)
		{
			Particle& particle = particles[index];
			if (!particle.IsAlive())
			{
				continue;
			}

			particle.age += deltaTime;
			if (!particle.IsAlive())
			{
				particle.spec = nullptr;
				continue;
			}

			const ParticleSpec& spec = *particle.spec;
			float progress = particle.age / particle.lifeTime;
			float speed = Lerp(spec.startSpeed, spec.endSpeed, progress);

			particle.gravityVelocity = particle.gravityVelocity + deltaTime * spec.gravity;
			particle.position = particle.position + deltaTime * (speed * particle.direction + particle.gravityVelocity);

			aliveCount++;
		}

		if (aliveCount == 0)
		{
			writeIndex = 0;
			usedCount = 0;
		}

		BuildVertices();
	}

	void ParticleSystemComponent::BuildVertices()
	{
		alphaVertices.clear();
		additiveVertices.clear();

		for (std::size_t index = 0; index < usedCount; index++)
		{
			if (particles[index].IsAlive())
			{
				AppendParticle(particles[index]);
			}
		}
	}

	void ParticleSystemComponent::AppendParticle(const Particle& particle)
	{
		const ParticleSpec& spec = *particle.spec;
		float progress = particle.age / particle.lifeTime;

		float height = Lerp(spec.startSize, spec.endSize, progress);
		float aspect = spec.frame.height > 0 ? static_cast<float>(spec.frame.width) / static_cast<float>(spec.frame.height) : 1.f;
		float width = height * aspect;

		float left = particle.position.x - 0.5f * width;
		float right = particle.position.x + 0.5f * width;
		float bottom = particle.position.y - 0.5f * height;
		float top = particle.position.y + 0.5f * height;

		float u0 = static_cast<float>(spec.frame.x);
		float u1 = static_cast<float>(spec.frame.x + spec.frame.width);
		float v0 = static_cast<float>(spec.frame.y);
		float v1 = static_cast<float>(spec.frame.y + spec.frame.height);

		sf::Color color = LerpColor(spec.startColor, spec.endColor, progress);
		sf::VertexArray& target = spec.isAdditive ? additiveVertices : alphaVertices;

		target.append({{left, bottom}, color, {u0, v1}});
		target.append({{right, bottom}, color, {u1, v1}});
		target.append({{right, top}, color, {u1, v0}});
		target.append({{left, top}, color, {u0, v0}});
	}

	void ParticleSystemComponent::Render()
	{
		if (texture == nullptr)
		{
			return;
		}

		if (alphaVertices.getVertexCount() > 0)
		{
			sf::RenderStates states(sf::BlendAlpha);
			states.texture = texture;
			RenderSystem::Instance()->Render(alphaVertices, states);
		}

		if (additiveVertices.getVertexCount() > 0)
		{
			sf::RenderStates states(sf::BlendAdd);
			states.texture = texture;
			RenderSystem::Instance()->Render(additiveVertices, states);
		}
	}

	std::size_t ParticleSystemComponent::GetActiveCount() const
	{
		std::size_t aliveCount = 0;
		for (std::size_t index = 0; index < usedCount; index++)
		{
			if (particles[index].IsAlive())
			{
				aliveCount++;
			}
		}

		return aliveCount;
	}

	std::size_t ParticleSystemComponent::GetCapacity() const
	{
		return particles.size();
	}

	const sf::VertexArray& ParticleSystemComponent::GetAlphaVertices() const
	{
		return alphaVertices;
	}
	const sf::VertexArray& ParticleSystemComponent::GetAdditiveVertices() const
	{
		return additiveVertices;
	}
}
