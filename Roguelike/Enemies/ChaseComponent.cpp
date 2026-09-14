#include "ChaseComponent.h"
#include "DamageInfo.h"
#include "FactionComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "PathService.h"
#include "HealthComponent.h"
#include <AimRotationComponent.h>
#include <DebugDraw.h>
#include <GameObject.h>
#include <GameWorld.h>
#include <MathUtils.h>

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

		GameObject* attacker = damage.source.attackerName.empty()
			? nullptr
			: GameWorld::Instance()->FindGameObject(damage.source.attackerName);

		if (attacker != nullptr)
		{
			investigatePoint = attacker->GetTransform()->GetWorldPosition();
			memory = Remember(memory, alertTime);
			return;
		}

		Vector2Df back = damage.source.direction;
		if (back.GetLengthSquared() <= 0.f)
		{
			memory = Forget(Alarm(memory, alertTime));
			return;
		}

		investigatePoint = transform->GetWorldPosition() - back.Normalized() * ENEMY_ALERT_POINT_DISTANCE;
		memory = Remember(memory, alertTime);
	}

	ChaseSense ChaseComponent::ReadSense(const Vector2Df& targetPosition, bool hasTarget) const
	{
		ChaseSense sense;
		sense.detectionRadius = detectionRadius;
		sense.stopDistance = stopDistance;
		sense.arriveDistance = ENEMY_ALERT_ARRIVE_DISTANCE;
		sense.isAlerted = IsSearching(memory);
		sense.isForced = isForced && hasTarget;
		sense.hasPoint = memory.hasPoint;

		Vector2Df position = transform->GetWorldPosition();
		Vector2Df toTarget = targetPosition - position;

		sense.distanceToTarget = hasTarget ? toTarget.GetLength() : detectionRadius + 1.f;
		sense.distanceToPoint = memory.hasPoint ? (investigatePoint - position).GetLength() : 0.f;

		if (hasTarget)
		{
			VisionRange range;
			range.maxDistance = detectionRadius;
			range.calmHalfAngle = visionHalfAngle;
			range.alertHalfAngle = alertHalfAngle;

			VisionCone cone = ConeFor(range, sense.isAlerted);

			bool isBlocked = LevelGrid::Current().HasWallBetween(position, targetPosition);
			sense.isVisible = CanSeeTarget(cone, Facing(), toTarget, isBlocked);
		}

		return sense;
	}

	Vector2Df ChaseComponent::Facing() const
	{
		return DirectionFromDegrees(transform->GetWorldRotation());
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
			aim->SetMaxDistance(0.f);
			return;
		}

		if (sense.isAlerted && memory.hasPoint)
		{
			aim->AimAtPoint(investigatePoint);
			aim->SetMaxDistance(0.f);
			return;
		}

		aim->StopAiming();
	}

	void ChaseComponent::Update(float deltaTime)
	{
		if (movement == nullptr)
		{
			return;
		}

		memory = Fade(memory, deltaTime);

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

		if (sense.isVisible)
		{
			investigatePoint = targetPosition;
			memory = Remember(memory, searchTime);
		}

		ChaseMove move = ChooseChaseMove(sense);

		isEngaged = RoguelikeGame::IsEngaged(sense);
		isChasing = RoguelikeGame::IsTargetDetected(sense);

		ApplyAim(sense);

		if (move == ChaseMove::Approach)
		{
			MoveTowards(targetPosition, deltaTime);
			return;
		}

		if (move == ChaseMove::Investigate)
		{
			MoveTowards(investigatePoint, deltaTime);
			return;
		}

		navigator.Reset();

		if (sense.isAlerted && memory.hasPoint && !RoguelikeGame::IsTargetDetected(sense))
		{
			memory = GiveUp(memory, ENEMY_LOOK_AROUND_TIME);
		}
	}

	void ChaseComponent::MoveTowards(const Vector2Df& goal, float deltaTime)
	{
		Vector2Df position = transform->GetWorldPosition();
		movement->SetDirection(navigator.Steer(position, goal, movement->GetSpeed(), deltaTime));
	}

	void ChaseComponent::DrawRoute() const
	{
		if (!DebugDraw::Instance()->IsEnabled() || !navigator.IsOnRoute())
		{
			return;
		}

		const RouteFollower& route = navigator.GetRoute();
		Vector2Df from = transform->GetWorldPosition();
		for (std::size_t i = route.GetIndex(); i < route.GetRoute().size(); i++)
		{
			DebugDraw::Instance()->DrawLine(from, route.GetRoute()[i], DEBUG_ROUTE_COLOR);
			from = route.GetRoute()[i];
		}
	}

	void ChaseComponent::DrawVision() const
	{
		if (!DebugDraw::Instance()->IsEnabled() || detectionRadius <= 0.f)
		{
			return;
		}

		VisionRange range;
		range.maxDistance = detectionRadius;
		range.calmHalfAngle = visionHalfAngle;
		range.alertHalfAngle = alertHalfAngle;

		bool isAlerted = IsSearching(memory);
		VisionCone cone = ConeFor(range, isAlerted);
		const sf::Color& color = isAlerted ? DEBUG_CHASING_COLOR : DEBUG_DETECTION_COLOR;

		Vector2Df position = transform->GetWorldPosition();
		float facing = transform->GetWorldRotation();
		float edge = facing - cone.halfAngleDegrees;
		float span = 2.f * cone.halfAngleDegrees;

		Vector2Df previous = position + DirectionFromDegrees(edge) * cone.maxDistance;
		DebugDraw::Instance()->DrawLine(position, previous, color);

		for (int step = 1; step <= DEBUG_VISION_CONE_STEPS; step++)
		{
			float angle = edge + span * static_cast<float>(step) / DEBUG_VISION_CONE_STEPS;
			Vector2Df point = position + DirectionFromDegrees(angle) * cone.maxDistance;

			DebugDraw::Instance()->DrawLine(previous, point, color);
			previous = point;
		}

		DebugDraw::Instance()->DrawLine(previous, position, color);
	}

	void ChaseComponent::Render()
	{
		DrawVision();
		DrawRoute();
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

	void ChaseComponent::SetVisionHalfAngle(float newHalfAngle)
	{
		visionHalfAngle = newHalfAngle;
	}

	void ChaseComponent::SetAlertHalfAngle(float newHalfAngle)
	{
		alertHalfAngle = newHalfAngle;
	}

	void ChaseComponent::SetSearchTime(float newSearchTime)
	{
		searchTime = newSearchTime;
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
		return IsSearching(memory);
	}

	bool ChaseComponent::IsEngaged() const
	{
		return isEngaged;
	}
}
