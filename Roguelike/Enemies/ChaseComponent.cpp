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

		Vector2Df position = transform->GetWorldPosition();
		Vector2Df toTarget = targetPosition - position;

		sense.distanceToTarget = hasTarget ? toTarget.GetLength() : detectionRadius + 1.f;
		sense.distanceToPoint = hasPoint ? (investigatePoint - position).GetLength() : 0.f;

		if (hasTarget)
		{
			VisionCone cone;
			cone.maxDistance = detectionRadius;
			cone.halfAngleDegrees = sense.isAlerted ? 180.f : visionHalfAngle;

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

		if (sense.isAlerted && hasPoint)
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
			MoveTowards(targetPosition, deltaTime);
			return;
		}

		if (move == ChaseMove::Investigate)
		{
			MoveTowards(investigatePoint, deltaTime);
			return;
		}

		route.Clear();

		if (sense.isAlerted && hasPoint && !RoguelikeGame::IsTargetDetected(sense))
		{
			hasPoint = false;
		}
	}

	void ChaseComponent::MoveTowards(const Vector2Df& goal, float deltaTime)
	{
		Vector2Df position = transform->GetWorldPosition();

		if (!LevelGrid::Current().HasWallBetween(position, goal))
		{
			route.Clear();
			repath.Stop();
			movement->SetDirection(goal - position);
			return;
		}

		repath.Tick(deltaTime);
		RefreshRoute(position, goal);
		route.Advance(position, ENEMY_ROUTE_ARRIVE_DISTANCE);

		movement->SetDirection(route.HasPoint() ? route.GetPoint() - position : goal - position);
	}

	void ChaseComponent::RefreshRoute(const Vector2Df& position, const Vector2Df& goal)
	{
		int goalColumn = 0;
		int goalRow = 0;
		LevelGrid::Current().ToCell(goal, goalColumn, goalRow);

		bool isGoalMoved = goalColumn != routeGoalColumn || goalRow != routeGoalRow;
		if (!isGoalMoved && route.HasPoint() && repath.IsRunning())
		{
			return;
		}

		std::vector<Vector2Df> points;
		PathService::Current().RouteTo(position, goal, points);
		route.SetRoute(std::move(points));

		routeGoalColumn = goalColumn;
		routeGoalRow = goalRow;
		repath.Start(ENEMY_REPATH_INTERVAL);
	}

	void ChaseComponent::DrawRoute() const
	{
		if (!DebugDraw::Instance()->IsEnabled() || !route.HasPoint())
		{
			return;
		}

		Vector2Df from = transform->GetWorldPosition();
		for (std::size_t i = route.GetIndex(); i < route.GetRoute().size(); i++)
		{
			DebugDraw::Instance()->DrawLine(from, route.GetRoute()[i], sf::Color::Cyan);
			from = route.GetRoute()[i];
		}
	}

	void ChaseComponent::Render()
	{
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
