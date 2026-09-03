#pragma once

#include "Component.h"
#include "Vector.h"

namespace XYZEngine
{
	class TransformComponent;
	class MovementComponent;
	class ColliderComponent;
	class HealthComponent;
	class SpriteMovementAnimationComponent;

	class DodgeRollComponent : public Component
	{
	public:
		DodgeRollComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void SetSpeeds(const float* newFrameSpeeds, int newFramesCount, float newPeakSpeed);
		void SetMaxStep(float newMaxStep);
		void SetInvulnerableFrames(int firstFrame, int lastFrame);
		void SetIgnoredLayers(unsigned int newIgnoredLayers);
		void SetCooldown(float newCooldown);

		bool IsReady() const;
		bool IsRolling() const;
		int GetDirection() const;

		bool TryRoll(const Vector2Df& direction);
		void CancelRoll();

	private:
		TransformComponent* transform;
		MovementComponent* movement = nullptr;
		ColliderComponent* collider = nullptr;
		HealthComponent* health = nullptr;
		SpriteMovementAnimationComponent* animation = nullptr;
		bool areComponentsSearched = false;

		const float* frameSpeeds = nullptr;
		int framesCount = 0;
		float peakSpeed = 0.f;
		float maxStep = 0.f;

		int invulnerableFirstFrame = 0;
		int invulnerableLastFrame = -1;
		unsigned int ignoredLayers = 0u;
		float cooldown = 0.f;

		bool isRolling = false;
		int rollDirection = 0;
		float cooldownTimer = 0.f;
		Vector2Df rollVector = { 1.f, 0.f };

		void FindComponents();
		Vector2Df GetForward() const;
		int GetDirectionIndex(const Vector2Df& direction, int directionsCount) const;
		void Finish();
	};
}
