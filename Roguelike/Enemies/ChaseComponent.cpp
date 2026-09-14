#include "ChaseComponent.h"
#include "DamageInfo.h"
#include "FactionComponent.h"
#include "GameSettings.h"
#include "HealthComponent.h"
#include <AimRotationComponent.h>
#include <DebugDraw.h>
#include <GameObject.h>
#include <GameWorld.h>

using namespace XYZEngine;

namespace RoguelikeGame
{
	ChaseComponent::ChaseComponent(GameObject* gameObject) : Component(gameObject)
	{
		transform = gameObject->GetTransform();
	}

	void ChaseComponent::Start()
	{
		movement = gameObject->GetComponent<MovementComponent>();
		aim = gameObject->GetComponent<AimRotationComponent>();
		health = gameObject->GetComponent<HealthComponent>();

		if (health != nullptr)
		{
			health->SubscribeDamage([this](const DamageInfo& damage) { OnDamage(damage); });
		}
	}

	void ChaseComponent::OnDamage(const DamageInfo& damage)
	{
		if (detectionRadius <= 0.f || alertTime <= 0.f)
		{
			return;
		}

		if (!CanDamage(damage.source.attackerFaction, GetFactionOf(gameObject)))
		{
			return;
		}

		alertLeft = alertTime;

		GameObject* attacker = damage.source.attackerName.empty()
			? nullptr
			: GameWorld::Instance()->FindGameObject(damage.source.attackerName);

		if (attacker != nullptr)
		{
			investigatePoint = attacker->GetTransform()->GetWorldPosition();
			hasPoint = true;
			return;
		}

		Vector2Df back = damage.source.direction;
		if (back.GetLengthSquared() <= 0.f)
		{
			hasPoint = false;
			return;
		}

		investigatePoint = transform->GetWorldPosition() - back.Normalized() * ENEMY_ALERT_POINT_DISTANCE;
		hasPoint = true;
	}

	ChaseSense ChaseComponent::ReadSense(const Vector2Df& targetPosition, bool hasTarget) const
	{
		ChaseSense sense;
		sense.detectionRadius = detectionRadius;
		sense.stopDistance = stopDistance;
		sense.arriveDistance = ENEMY_ALERT_ARRIVE_DISTANCE;
		sense.isAlerted = alertLeft > 0.f;
		sense.isForced = isForced && hasTarget;
		sense.hasPoint = hasPoint;
		sense.distanceToTarget = hasTarget
			? (targetPosition - transform->GetWorldPosition()).GetLength()
			: detectionRadius + 1.f;
		sense.distanceToPoint = hasPoint ? (investigatePoint - transform->GetWorldPosition()).GetLength() : 0.f;

		if (!hasTarget)
		{
			sense.detectionRadius = 0.f;
		}

		return sense;
	}

	void ChaseComponent::ApplyAim(const ChaseSense& sense)
	{
		if (aim == nullptr)
		{
			return;
		}

		if (RoguelikeGame::IsTargetDetected(sense))
		{
			aim->AimAtGameObject(targetName);
			aim->SetMaxDistance(sense.isForced || sense.isAlerted ? 0.f : detectionRadius);
			return;
		}

		if (sense.isAlerted && hasPoint)
		{
			aim->AimAtPoint(investigatePoint);
			aim->SetMaxDistance(0.f);
		}
	}

	void ChaseComponent::Update(float deltaTime)
	{
		if (movement == nullptr)
		{
			return;
		}

		if (alertLeft > 0.f)
		{
			alertLeft -= deltaTime;
		}

		isChasing = false;
		isEngaged = false;
		movement->SetDirection({ 0.f, 0.f });

		if (targetName.empty() || detectionRadius <= 0.f)
		{
			return;
		}

		GameObject* target = GameWorld::Instance()->FindGameObject(targetName);
		Vector2Df targetPosition = target != nullptr ? target->GetTransform()->GetWorldPosition() : Vector2Df{ 0.f, 0.f };

		ChaseSense sense = ReadSense(targetPosition, target != nullptr);
		ChaseMove move = ChooseChaseMove(sense);

		isEngaged = RoguelikeGame::IsEngaged(sense);
		isChasing = RoguelikeGame::IsTargetDetected(sense);

		ApplyAim(sense);

		if (move == ChaseMove::Approach)
		{
			movement->SetDirection(targetPosition - transform->GetWorldPosition());
			return;
		}

		if (move == ChaseMove::Investigate)
		{
			movement->SetDirection(investigatePoint - transform->GetWorldPosition());
			return;
		}

		if (sense.isAlerted && hasPoint && !RoguelikeGame::IsTargetDetected(sense))
		{
			hasPoint = false;
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

	void ChaseComponent::SetAlertTime(float newAlertTime)
	{
		alertTime = newAlertTime;
	}

	void ChaseComponent::SetForcedChase(bool newIsForced)
	{
		isForced = newIsForced;
	}

	bool ChaseComponent::IsChasing() const
	{
		return isChasing;
	}

	bool ChaseComponent::IsAlerted() const
	{
		return alertLeft > 0.f;
	}

	bool ChaseComponent::IsEngaged() const
	{
		return isEngaged;
	}
}
