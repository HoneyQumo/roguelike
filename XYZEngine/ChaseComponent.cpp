#include "pch.h"
#include "ChaseComponent.h"
#include "GameWorld.h"

namespace XYZEngine
{
	ChaseComponent::ChaseComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetComponent<TransformComponent>();
	}

	void ChaseComponent::Update(float deltaTime)
	{
		if (movement == nullptr)
		{
			movement = gameObject->GetComponent<MovementComponent>();
			if (movement == nullptr)
			{
				return;
			}
		}

		isChasing = false;
		movement->SetDirection({ 0.f, 0.f });

		if (targetName.empty() || detectionRadius <= 0.f)
		{
			return;
		}

		// Searching every frame instead of caching the pointer: the target may be destroyed at any moment.
		GameObject* target = GameWorld::Instance()->FindGameObject(targetName);
		if (target == nullptr)
		{
			return;
		}

		Vector2Df toTarget = target->GetComponent<TransformComponent>()->GetWorldPosition() - transform->GetWorldPosition();
		float distance = toTarget.GetLength();
		if (distance > detectionRadius)
		{
			return;
		}

		isChasing = true;

		if (distance > stopDistance)
		{
			movement->SetDirection(toTarget);
		}
	}
	void ChaseComponent::Render()
	{

	}

	void ChaseComponent::SetTargetName(const std::string& newTargetName)
	{
		targetName = newTargetName;
	}
	const std::string& ChaseComponent::GetTargetName() const
	{
		return targetName;
	}

	void ChaseComponent::SetDetectionRadius(float newDetectionRadius)
	{
		detectionRadius = newDetectionRadius;
	}
	float ChaseComponent::GetDetectionRadius() const
	{
		return detectionRadius;
	}

	void ChaseComponent::SetStopDistance(float newStopDistance)
	{
		stopDistance = newStopDistance;
	}
	float ChaseComponent::GetStopDistance() const
	{
		return stopDistance;
	}

	bool ChaseComponent::IsChasing() const
	{
		return isChasing;
	}
}
