#pragma once

#include <string>
#include "Component.h"
#include "TransformComponent.h"
#include "MovementComponent.h"

namespace XYZEngine
{
	class ChaseComponent : public Component
	{
	public:
		ChaseComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void SetTargetName(const std::string& newTargetName);
		const std::string& GetTargetName() const;

		void SetDetectionRadius(float newDetectionRadius);
		float GetDetectionRadius() const;

		void SetStopDistance(float newStopDistance);
		float GetStopDistance() const;

		bool IsChasing() const;
	private:
		TransformComponent* transform;
		MovementComponent* movement = nullptr;

		std::string targetName;
		float detectionRadius = 0.f;
		float stopDistance = 0.f;
		bool isChasing = false;
	};
}
