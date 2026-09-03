#pragma once

#include <Component.h>
#include <Vector.h>
#include <Cooldown.h>

namespace XYZEngine
{
	class TransformComponent;
	class MovementComponent;
	class ColliderComponent;
	class SpriteMovementAnimationComponent;
}

namespace RoguelikeGame
{
	class HealthComponent;

	class DodgeRollComponent : public XYZEngine::Component
	{
	public:
		DodgeRollComponent(XYZEngine::GameObject* gameObject);

		void Start() override;
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

		bool TryRoll(const XYZEngine::Vector2Df& direction);
		void CancelRoll();

	private:
		XYZEngine::TransformComponent* transform;
		XYZEngine::MovementComponent* movement = nullptr;
		XYZEngine::ColliderComponent* collider = nullptr;
		HealthComponent* health = nullptr;
		XYZEngine::SpriteMovementAnimationComponent* animation = nullptr;
		bool areComponentsSearched = false;

		const float* frameSpeeds = nullptr;
		int framesCount = 0;
		float peakSpeed = 0.f;
		float maxStep = 0.f;

		int invulnerableFirstFrame = 0;
		int invulnerableLastFrame = -1;
		unsigned int ignoredLayers = 0u;

		bool isRolling = false;
		int rollDirection = 0;
		XYZEngine::Cooldown cooldown;
		XYZEngine::Vector2Df rollVector = { 1.f, 0.f };

		void FindComponents();
		XYZEngine::Vector2Df GetForward() const;
		int GetDirectionIndex(const XYZEngine::Vector2Df& direction, int directionsCount) const;
		void Finish();
	};
}
