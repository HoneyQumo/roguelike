#include "pch.h"
#include "ParticleAuraComponent.h"
#include "GameObject.h"
#include "MathUtils.h"
#include "ParticleSystem.h"
#include <algorithm>
#include <cmath>

namespace XYZEngine
{
	namespace
	{
		constexpr float AURA_ANGLE_STEP = 137.5f;
		constexpr float FULL_CIRCLE = 360.f;
		constexpr float TANGENT_DEGREES = 90.f;
	}

	ParticleAuraComponent::ParticleAuraComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetTransform();
	}

	void ParticleAuraComponent::Update(float deltaTime)
	{
		if (!isActive || spec == nullptr || spec->ratePerSecond <= 0.f)
		{
			return;
		}

		pending += spec->ratePerSecond * deltaTime;

		while (pending >= 1.f)
		{
			pending -= 1.f;
			angle = std::fmod(angle + AURA_ANGLE_STEP, FULL_CIRCLE);

			Vector2Df radial = RotateByDegrees({1.f, 0.f}, angle);
			Vector2Df position = transform->GetWorldPosition() + radial * radius;

			ParticleSystem::Instance()->EmitCount(*spec, position, RotateByDegrees(radial, TANGENT_DEGREES), 1);
		}
	}

	void ParticleAuraComponent::Render()
	{
	}

	void ParticleAuraComponent::SetSpec(const ParticleSpec* newSpec)
	{
		spec = newSpec;
	}

	void ParticleAuraComponent::SetRadius(float newRadius)
	{
		radius = std::max(0.f, newRadius);
	}

	void ParticleAuraComponent::SetActive(bool newIsActive)
	{
		if (isActive == newIsActive)
		{
			return;
		}

		isActive = newIsActive;
		pending = 0.f;
	}

	bool ParticleAuraComponent::IsActive() const
	{
		return isActive;
	}

	float ParticleAuraComponent::GetRadius() const
	{
		return radius;
	}
}
