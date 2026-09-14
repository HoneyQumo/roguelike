#pragma once

#include <string>
#include <Component.h>
#include <TransformComponent.h>
#include <MovementComponent.h>
#include "ChaseRules.h"
#include "Navigator.h"
#include "SightMemory.h"
#include "Vision.h"
#include <Cooldown.h>

namespace XYZEngine
{
	class AimRotationComponent;
}

namespace RoguelikeGame
{
	struct DamageInfo;
	class HealthComponent;

	class ChaseComponent : public XYZEngine::Component
	{
	public:
		ChaseComponent(XYZEngine::GameObject* gameObject);

		void Start() override;
		void Update(float deltaTime) override;
		void Render() override;

		void SetTargetName(const std::string& newTargetName);
		const std::string& GetTargetName() const;

		void SetDetectionRadius(float newDetectionRadius);
		float GetDetectionRadius() const;

		void SetStopDistance(float newStopDistance);
		float GetStopDistance() const;

		void SetAlertTime(float newAlertTime);
		void SetVisionHalfAngle(float newHalfAngle);
		void SetAlertHalfAngle(float newHalfAngle);
		void SetSearchTime(float newSearchTime);
		void SetForcedChase(bool newIsForced);

		bool IsChasing() const;
		bool IsAlerted() const;
		bool IsEngaged() const;

	private:
		XYZEngine::TransformComponent* transform = nullptr;
		XYZEngine::MovementComponent* movement = nullptr;
		XYZEngine::AimRotationComponent* aim = nullptr;
		HealthComponent* health = nullptr;

		std::string targetName;
		float detectionRadius = 0.f;
		float stopDistance = 0.f;
		float alertTime = 0.f;
		float visionHalfAngle = 180.f;
		float alertHalfAngle = 180.f;
		float searchTime = 0.f;
		bool isForced = false;
		bool isChasing = false;
		bool isEngaged = false;

		XYZEngine::Vector2Df investigatePoint = {0.f, 0.f};
		SightMemory memory;

		Navigator navigator;

		void OnDamage(const DamageInfo& damage);
		ChaseSense ReadSense(const XYZEngine::Vector2Df& targetPosition, bool hasTarget) const;
		void ApplyAim(const ChaseSense& sense);
		XYZEngine::Vector2Df Facing() const;
		void MoveTowards(const XYZEngine::Vector2Df& goal, float deltaTime);
		void DrawRoute() const;
		void DrawVision() const;
	};
}
