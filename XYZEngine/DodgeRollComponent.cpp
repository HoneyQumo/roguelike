#include "pch.h"
#include "DodgeRollComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "MovementComponent.h"
#include "ColliderComponent.h"
#include "HealthComponent.h"
#include "SpriteMovementAnimationComponent.h"
#include "LoggerRegistry.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace XYZEngine
{
	constexpr float FULL_TURN_RADIANS = 6.2831853f;
	constexpr float RADIANS_IN_DEGREE = 0.01745329f;

	DodgeRollComponent::DodgeRollComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
	}

	void DodgeRollComponent::Update(float deltaTime)
	{
		FindComponents();

		if (cooldownTimer > 0.f)
		{
			cooldownTimer = std::max(0.f, cooldownTimer - deltaTime);
		}

		if (!isRolling)
		{
			return;
		}

		if (animation == nullptr || animation->GetCurrentAnimation() != MovementAnimation::Roll || animation->IsFinished())
		{
			Finish();
			return;
		}

		int frame = animation->GetCurrentFrame();
		if (frame < 0 || frame >= framesCount)
		{
			return;
		}

		if (health != nullptr)
		{
			health->SetInvulnerable(frame >= invulnerableFirstFrame && frame <= invulnerableLastFrame);
		}

		float step = peakSpeed * frameSpeeds[frame] * deltaTime;
		if (maxStep > 0.f)
		{
			step = std::min(step, maxStep);
		}

		if (step > 0.f)
		{
			transform->MoveBy(step * rollVector);
		}
	}

	void DodgeRollComponent::Render()
	{
	}

	void DodgeRollComponent::SetSpeeds(const float* newFrameSpeeds, int newFramesCount, float newPeakSpeed)
	{
		assert(newFrameSpeeds != nullptr);
		assert(newFramesCount > 0);
		assert(newPeakSpeed > 0.f);

		frameSpeeds = newFrameSpeeds;
		framesCount = newFramesCount;
		peakSpeed = newPeakSpeed;
	}

	void DodgeRollComponent::SetMaxStep(float newMaxStep)
	{
		maxStep = newMaxStep;
	}

	void DodgeRollComponent::SetInvulnerableFrames(int firstFrame, int lastFrame)
	{
		invulnerableFirstFrame = firstFrame;
		invulnerableLastFrame = lastFrame;
	}

	void DodgeRollComponent::SetIgnoredLayers(unsigned int newIgnoredLayers)
	{
		ignoredLayers = newIgnoredLayers;
	}

	void DodgeRollComponent::SetCooldown(float newCooldown)
	{
		assert(newCooldown >= 0.f);
		cooldown = newCooldown;
	}

	bool DodgeRollComponent::IsReady() const
	{
		return !isRolling && cooldownTimer <= 0.f && frameSpeeds != nullptr && animation != nullptr
			&& animation->GetRollDirectionsCount() > 0 && (health == nullptr || health->IsAlive());
	}

	bool DodgeRollComponent::IsRolling() const
	{
		return isRolling;
	}

	int DodgeRollComponent::GetDirection() const
	{
		return rollDirection;
	}

	bool DodgeRollComponent::TryRoll(const Vector2Df& direction)
	{
		FindComponents();

		if (!IsReady())
		{
			return false;
		}

		float length = direction.GetLength();
		rollVector = length > 0.f ? (1.f / length) * direction : GetForward();
		rollDirection = GetDirectionIndex(rollVector, animation->GetRollDirectionsCount());

		animation->PlayRoll(rollDirection);
		if (animation->GetCurrentAnimation() != MovementAnimation::Roll)
		{
			return false;
		}

		isRolling = true;

		if (movement != nullptr)
		{
			movement->SetEnabled(false);
		}

		if (collider != nullptr)
		{
			collider->SetIgnoredLayers(ignoredLayers);
		}

		LOG_INFO(gameObject->GetName() + " rolls in direction " + std::to_string(rollDirection));
		return true;
	}

	void DodgeRollComponent::CancelRoll()
	{
		if (isRolling)
		{
			Finish();
		}
	}

	void DodgeRollComponent::FindComponents()
	{
		if (areComponentsSearched)
		{
			return;
		}

		movement = gameObject->GetComponent<MovementComponent>();
		collider = gameObject->GetComponent<ColliderComponent>();
		health = gameObject->GetComponent<HealthComponent>();
		animation = gameObject->GetComponent<SpriteMovementAnimationComponent>();
		areComponentsSearched = true;
	}

	Vector2Df DodgeRollComponent::GetForward() const
	{
		float rotation = transform->GetWorldRotation() * RADIANS_IN_DEGREE;
		return { std::cos(rotation), std::sin(rotation) };
	}

	int DodgeRollComponent::GetDirectionIndex(const Vector2Df& direction, int directionsCount) const
	{
		Vector2Df forward = GetForward();
		float relative = std::atan2(direction.y, direction.x) - std::atan2(forward.y, forward.x);
		float step = FULL_TURN_RADIANS / static_cast<float>(directionsCount);

		int index = static_cast<int>(std::lround(relative / step));
		return ((index % directionsCount) + directionsCount) % directionsCount;
	}

	void DodgeRollComponent::Finish()
	{
		isRolling = false;
		cooldownTimer = cooldown;

		if (health != nullptr)
		{
			health->SetInvulnerable(false);
		}

		if (collider != nullptr)
		{
			collider->SetIgnoredLayers(0u);
		}

		if (movement != nullptr && (health == nullptr || health->IsAlive()))
		{
			movement->SetEnabled(true);
		}
	}
}
