#include "pch.h"
#include "ParticleEmitterComponent.h"
#include "GameObject.h"
#include "ParticleSystem.h"

namespace XYZEngine
{
	ParticleEmitterComponent::ParticleEmitterComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetTransform();
	}

	void ParticleEmitterComponent::Start()
	{
		if (spec == nullptr)
		{
			return;
		}

		if (spec->emission == ParticleEmission::Burst)
		{
			ParticleSystem::Instance()->Emit(*spec, transform->GetWorldPosition(), direction);
			return;
		}

		isEmitting = true;
		timeLeft = spec->duration;
	}

	void ParticleEmitterComponent::Update(float deltaTime)
	{
		if (spec == nullptr || !isEmitting)
		{
			return;
		}

		if (spec->duration > 0.f)
		{
			timeLeft -= deltaTime;
			if (timeLeft <= 0.f)
			{
				isEmitting = false;
			}
		}

		pending += spec->ratePerSecond * deltaTime;
		EmitPending();
	}

	void ParticleEmitterComponent::EmitPending()
	{
		int count = static_cast<int>(pending);
		if (count <= 0)
		{
			return;
		}

		pending -= static_cast<float>(count);
		ParticleSystem::Instance()->EmitCount(*spec, transform->GetWorldPosition(), direction, count);
	}

	void ParticleEmitterComponent::Render()
	{
	}

	void ParticleEmitterComponent::SetSpec(const ParticleSpec* newSpec)
	{
		spec = newSpec;
	}

	void ParticleEmitterComponent::SetDirection(const Vector2Df& newDirection)
	{
		direction = newDirection;
	}

	bool ParticleEmitterComponent::IsEmitting() const
	{
		return isEmitting;
	}
}
