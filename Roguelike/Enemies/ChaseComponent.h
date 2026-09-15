#pragma once

#include <string>
#include <Component.h>
#include <TransformComponent.h>
#include <MovementComponent.h>
#include "Awareness.h"
#include "ChaseRules.h"
#include "HidingSpots.h"
#include "LookTurn.h"
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
		void SetLook(float newLookTime, float newHalfSweep);
		void SetSearchSpots(int newRadius, int newMin, int newMax);
		void SetSearchLook(float newLookTime);
		void SetSearchGap(int newGap);
		void SetAlertRadiusScale(float newScale);
		void SetAwareness(float newGain, float newDecay);
		void SetPeripheryHalfAngle(float newHalfAngle);
		void SetShoutRadius(float newRadius);
		void SetForcedChase(bool newIsForced);
		void Hear(const XYZEngine::Vector2Df& place);
		void Provoke(const XYZEngine::Vector2Df& place);

		bool IsChasing() const;
		bool IsAlerted() const;
		bool IsSuspicious() const;
		float GetAwareness() const;
		float GetShoutRadius() const;
		VisionBand GetVisionBand() const;
		AwarenessState GetAwarenessState() const;
		bool IsEngaged() const;
		bool CanSeeTarget() const;
		std::size_t GetSearchStep() const;
		const XYZEngine::Vector2Df& GetEscapeDirection() const;

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
		float peripheryHalfAngle = 0.f;
		float shoutRadius = 0.f;
		AwarenessState spottedBefore = AwarenessState::Calm;
		float searchTime = 0.f;
		bool isForced = false;
		bool isChasing = false;
		bool isEngaged = false;
		bool isTargetVisible = false;
		bool isInSight = false;
		VisionBand band = VisionBand::None;
		bool hasLastTarget = false;
		float awareness = 0.f;
		AwarenessRates rates;
		XYZEngine::Vector2Df lastTargetPosition = {0.f, 0.f};

		XYZEngine::Vector2Df investigatePoint = {0.f, 0.f};
		XYZEngine::Vector2Df escapeFrom = {0.f, 0.f};
		XYZEngine::Vector2Df escapeDirection = {0.f, 0.f};
		SightMemory memory;

		float lookTime = 0.f;
		float lookHalfSweep = 0.f;
		int searchRadius = 0;
		int searchSpotsMin = 0;
		int searchSpotsMax = 0;
		float searchLookTime = 0.f;
		int searchGap = 3;
		float alertRadiusScale = 1.f;

		LookTurn look;
		std::vector<XYZEngine::Vector2Df> searchSpots;
		std::size_t searchSpot = 0u;
		bool hasSearched = false;

		Navigator navigator;

		void OnDamage(const DamageInfo& damage);
		ChaseSense ReadSense(const XYZEngine::Vector2Df& targetPosition, bool hasTarget) const;
		VisionField ReadField(bool isAlerted) const;
		AwarenessSense ReadAwareness(const ChaseSense& sense, const XYZEngine::Vector2Df& targetPosition, float deltaTime) const;
		void ApplyAim(const ChaseSense& sense);
		void Shout();
		XYZEngine::Vector2Df Facing() const;
		void MoveTowards(const XYZEngine::Vector2Df& goal, float deltaTime);
		void AimAtAngle(float degrees);
		void PlanSearch();
		bool TakeNextSpot();
		void SearchAtTheSpot(float deltaTime);
		void StartSearchLook();
		void DrawRoute() const;
		void DrawVision() const;
	};
}
