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
#include <randomizer.h>

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
		sense.arriveDistance = SEARCH_ARRIVE_DISTANCE;
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
			range.maxDistance = detectionRadius * (sense.isAlerted ? alertRadiusScale : 1.f);
			range.calmHalfAngle = visionHalfAngle;
			range.alertHalfAngle = alertHalfAngle;

			VisionCone cone = ConeFor(range, sense.isAlerted);

			bool isBlocked = LevelGrid::Current().HasWallBetween(position, targetPosition);
			sense.isVisible = RoguelikeGame::CanSeeTarget(cone, Facing(), toTarget, isBlocked);
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
		isTargetVisible = false;
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
			hasSearched = false;
			searchSpots.clear();
			searchSpot = 0u;
			look.Stop();
		}

		ChaseMove move = ChooseChaseMove(sense);

		isEngaged = RoguelikeGame::IsEngaged(sense);
		isChasing = RoguelikeGame::IsTargetDetected(sense);
		isTargetVisible = sense.isVisible;

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

		if (!sense.isAlerted || RoguelikeGame::IsTargetDetected(sense))
		{
			return;
		}

		if (look.IsRunning())
		{
			SearchAtTheSpot(deltaTime);
			return;
		}

		if (memory.hasPoint)
		{
			if (!hasSearched)
			{
				PlanSearch();
			}

			if (searchLookTime > 0.f || lookTime > 0.f)
			{
				StartSearchLook();
				return;
			}

			if (!TakeNextSpot())
			{
				memory = GiveUp(memory, 0.f);
			}
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

	void ChaseComponent::SetLook(float newLookTime, float newHalfSweep)
	{
		lookTime = newLookTime;
		lookHalfSweep = newHalfSweep;
	}

	void ChaseComponent::SetSearchSpots(int newRadius, int newMin, int newMax)
	{
		searchRadius = newRadius;
		searchSpotsMin = newMin;
		searchSpotsMax = newMax;
	}

	void ChaseComponent::SetSearchLook(float newLookTime)
	{
		searchLookTime = newLookTime;
	}

	void ChaseComponent::SetSearchGap(int newGap)
	{
		searchGap = newGap;
	}

	void ChaseComponent::SetAlertRadiusScale(float newScale)
	{
		alertRadiusScale = newScale;
	}

	void ChaseComponent::StartSearchLook()
	{
		float base = searchLookTime > 0.f ? searchLookTime : lookTime;
		float duration = random<float>(base, base * SEARCH_LOOK_SPREAD);

		look.Start(transform->GetWorldRotation(), lookHalfSweep, duration);
		memory = Alarm(memory, searchTime + duration);
	}

	void ChaseComponent::AimAtAngle(float degrees)
	{
		if (aim == nullptr)
		{
			return;
		}

		aim->AimAtPoint(transform->GetWorldPosition() + DirectionFromDegrees(degrees) * LOOK_AIM_DISTANCE);
	}

	void ChaseComponent::PlanSearch()
	{
		hasSearched = true;
		searchSpots.clear();
		searchSpot = 0u;

		if (searchRadius <= 0 || searchSpotsMax <= 0)
		{
			return;
		}

		const PathField* field = PathService::Current().FieldTo(investigatePoint);
		if (field == nullptr)
		{
			return;
		}

		int wanted = random<int>(std::max(1, searchSpotsMin), std::max(1, searchSpotsMax));
		searchSpots = FindHidingSpots(LevelGrid::Current(), *field, investigatePoint,
			searchRadius, static_cast<std::size_t>(wanted), searchGap);
	}

	bool ChaseComponent::TakeNextSpot()
	{
		if (searchSpot >= searchSpots.size())
		{
			return false;
		}

		investigatePoint = searchSpots[searchSpot];
		searchSpot++;
		memory = Alarm(memory, searchTime);

		return true;
	}

	void ChaseComponent::SearchAtTheSpot(float deltaTime)
	{
		look.Tick(deltaTime);
		AimAtAngle(look.GetAngle());

		if (!look.IsDone())
		{
			return;
		}

		look.Stop();

		if (!TakeNextSpot())
		{
			memory = GiveUp(memory, 0.f);
			searchSpots.clear();
		}
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

	bool ChaseComponent::CanSeeTarget() const
	{
		return isTargetVisible;
	}

	std::size_t ChaseComponent::GetSearchStep() const
	{
		return searchSpot;
	}
}
