#include "ChaseComponent.h"
#include "DamageInfo.h"
#include "FactionComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "Noise.h"
#include "BackOffSpot.h"
#include "CoverSpot.h"
#include "WeaponComponent.h"
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
		weapon = gameObject->GetComponent<WeaponComponent>();

		if (movement != nullptr)
		{
			walkSpeed = movement->GetSpeed();
		}

		if (health != nullptr)
		{
			health->SubscribeDamage([this](const DamageInfo& damage) { OnDamage(damage); });
		}
	}

	void ChaseComponent::Hear(const Vector2Df& place)
	{
		if (detectionRadius <= 0.f || alertTime <= 0.f || (health != nullptr && !health->IsAlive()))
		{
			return;
		}

		TakePoint(place, alertTime);
		awareness = std::max(awareness, AWARENESS_ALERT_AT);
	}

	void ChaseComponent::Provoke(const Vector2Df& place)
	{
		if (detectionRadius <= 0.f || (health != nullptr && !health->IsAlive()))
		{
			return;
		}

		TakePoint(place, alertTime);
		awareness = AWARENESS_PROVOKE_AT;
		spottedBefore = AwarenessState::Provoked;
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
			TakePoint(attacker->GetTransform()->GetWorldPosition(), alertTime);
			return;
		}

		Vector2Df back = damage.source.direction;
		if (back.GetLengthSquared() <= 0.f)
		{
			memory = Forget(Alarm(memory, alertTime));
			return;
		}

		TakePoint(transform->GetWorldPosition() - back.Normalized() * ENEMY_ALERT_POINT_DISTANCE, alertTime);
	}

	ChaseSense ChaseComponent::ReadSense(const Vector2Df& targetPosition, bool hasTarget) const
	{
		ChaseSense sense;
		sense.detectionRadius = detectionRadius;
		sense.stopDistance = stopDistance;
		sense.backOffDistance = std::max(0.f, stopDistance - ENEMY_COMFORT_DEAD_ZONE);
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
			bool isBlocked = LevelGrid::Current().HasWallBetween(position, targetPosition);
			sense.isVisible = BandFor(ReadField(sense.isAlerted), Facing(), toTarget, isBlocked) != VisionBand::None;
		}

		return sense;
	}

	VisionField ChaseComponent::ReadField(bool isAlerted) const
	{
		VisionField field;
		field.maxDistance = detectionRadius * (isAlerted ? alertRadiusScale : 1.f);
		field.focusHalfAngle = isAlerted ? std::max(visionHalfAngle, alertHalfAngle) : visionHalfAngle;
		field.peripheryHalfAngle = std::max(peripheryHalfAngle, field.focusHalfAngle);

		return field;
	}

	Vector2Df ChaseComponent::Facing() const
	{
		return DirectionFromDegrees(transform->GetWorldRotation());
	}

	AwarenessSense ChaseComponent::ReadAwareness(const ChaseSense& sense, const Vector2Df& targetPosition, float deltaTime) const
	{
		VisionField field = ReadField(sense.isAlerted);

		AwarenessSense look;
		look.band = band;
		look.keepsMemory = IsSearching(memory);
		look.distance = sense.distanceToTarget;
		look.maxDistance = BandDistance(field, band);
		look.isTargetMoving = hasLastTarget && deltaTime > 0.f
			&& (targetPosition - lastTargetPosition).GetLength() / deltaTime >= AWARENESS_MOVING_SPEED;

		return look;
	}

	void ChaseComponent::Shout()
	{
		if (shoutRadius <= 0.f)
		{
			return;
		}

		Noise call;
		call.position = transform->GetWorldPosition();
		call.radius = shoutRadius;
		call.from = GetFactionOf(gameObject);
		call.kind = NoiseKind::Call;

		RaiseNoise(call, gameObject);
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

		isChasing = false;
		isEngaged = false;
		isBackingOff = false;
		isTargetVisible = false;
		isInSight = false;
		band = VisionBand::None;
		movement->SetDirection({ 0.f, 0.f });
		ApplyPace();

		if (targetName.empty() || detectionRadius <= 0.f)
		{
			memory = Tick(memory, deltaTime, false);
			return;
		}

		GameObject* target = GameWorld::Instance()->FindGameObject(targetName);
		Vector2Df targetPosition = target != nullptr ? target->GetTransform()->GetWorldPosition() : Vector2Df{ 0.f, 0.f };

		ChaseSense sense = ReadSense(targetPosition, target != nullptr);
		isInSight = sense.isVisible;
		band = target != nullptr
			? BandFor(ReadField(sense.isAlerted), Facing(), targetPosition - transform->GetWorldPosition(),
				LevelGrid::Current().HasWallBetween(transform->GetWorldPosition(), targetPosition))
			: VisionBand::None;

		awareness = NextAwareness(awareness, rates, ReadAwareness(sense, targetPosition, deltaTime), deltaTime);
		lastTargetPosition = targetPosition;
		hasLastTarget = target != nullptr;

		AwarenessState state = StateOf(awareness);
		if (IsSpottedNow(spottedBefore, state))
		{
			Shout();
		}

		spottedBefore = state;
		sense.isVisible = isInSight && state == AwarenessState::Provoked;
		sense.isAlerted = sense.isAlerted || state == AwarenessState::Alerted;
		sense.isReloading = target != nullptr && weapon != nullptr && weapon->IsReloading();
		sense.hasCover = sense.isReloading && TakeCoverFrom(targetPosition);

		if (sense.isVisible)
		{
			if (!memory.hasPoint)
			{
				escapeFrom = targetPosition;
				escapeDirection = {0.f, 0.f};
			}

			Vector2Df moved = targetPosition - escapeFrom;
			if (moved.GetLength() > SEARCH_ESCAPE_STEP)
			{
				escapeDirection = moved;
				escapeFrom = targetPosition;
			}

			investigatePoint = targetPosition;
			memory = Remember(memory, searchTime, SEARCH_TRAVEL_TIME);
			hasSearched = false;
			searchSpots.clear();
			searchSpot = 0u;
			look.Stop();
		}

		ChaseMove move = ChooseChaseMove(sense);
		memory = Tick(memory, deltaTime, move == ChaseMove::Investigate);

		isEngaged = RoguelikeGame::IsEngaged(sense);
		isChasing = RoguelikeGame::IsTargetDetected(sense);
		isBackingOff = move == ChaseMove::Withdraw;
		isTargetVisible = sense.isVisible;
		ApplyPace();

		ApplyAim(sense);

		if (move == ChaseMove::Approach)
		{
			MoveTowards(targetPosition, deltaTime);
			return;
		}

		if (move == ChaseMove::TakeCover)
		{
			MoveTowards(coverSpot, deltaTime);
			return;
		}

		if (move == ChaseMove::Withdraw)
		{
			BackAwayFrom(targetPosition, sense.stopDistance, deltaTime);
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

	/**
	*	Берёт точку, если до неё есть дорога.
	*
	*	До точки за рекой или в запертой комнате враг будет толкаться в стену
	*	весь бюджет. Пусть лучше останется настороже на месте: слышал он всё равно.
	*/
	void ChaseComponent::TakePoint(const Vector2Df& place, float duration)
	{
		investigatePoint = place;
		memory = CanReach(place)
			? Remember(memory, duration, SEARCH_TRAVEL_TIME)
			: Forget(Alarm(memory, duration));
	}

	bool ChaseComponent::CanReach(const Vector2Df& place) const
	{
		std::vector<Vector2Df> steps;

		return PathService::Current().RouteTo(transform->GetWorldPosition(), place, steps);
	}

	/**
	*	Находит укрытие на время перезарядки.
	*
	*	Точка живёт, пока из неё действительно не видно: игрок двигается, и стена
	*	перестаёт закрывать. Искать каждый кадр нельзя - поле строится от самого врага.
	*/
	bool ChaseComponent::TakeCoverFrom(const Vector2Df& threat)
	{
		Vector2Df position = transform->GetWorldPosition();

		if (hasCoverSpot && !LevelGrid::Current().HasWallBetween(threat, coverSpot))
		{
			hasCoverSpot = false;
		}

		if (!hasCoverSpot)
		{
			const PathField* field = PathService::Current().FieldTo(position);
			hasCoverSpot = field != nullptr
				&& FindCoverSpot(LevelGrid::Current(), *field, position, threat, ENEMY_COVER_RADIUS, coverSpot);
		}

		return hasCoverSpot;
	}

	void ChaseComponent::MoveTowards(const Vector2Df& goal, float deltaTime)
	{
		Vector2Df position = transform->GetWorldPosition();
		movement->SetDirection(navigator.Steer(position, goal, movement->GetSpeed(), deltaTime));
	}

	/**
	*	Отходит по маршруту, а не спиной в стену.
	*
	*	Точка живёт, пока до неё не дошли и пока она дальше от угрозы, чем мы сейчас.
	*	Искать каждый кадр нельзя: поле строится от самого врага, а слотов кэша четыре.
	*
	*	Зажатый в угол остаётся стоять и стрелять - это лучше, чем скрестись в стену.
	*/
	void ChaseComponent::BackAwayFrom(const Vector2Df& threat, float wanted, float deltaTime)
	{
		Vector2Df position = transform->GetWorldPosition();

		if (hasBackOffSpot)
		{
			bool isReached = (backOffSpot - position).GetLength() <= ENEMY_ROUTE_ARRIVE_DISTANCE;
			bool isPointless = (backOffSpot - threat).GetLength() <= (position - threat).GetLength();

			hasBackOffSpot = !isReached && !isPointless;
		}

		if (!hasBackOffSpot)
		{
			const PathField* field = PathService::Current().FieldTo(position);
			hasBackOffSpot = field != nullptr
				&& FindBackOffSpot(LevelGrid::Current(), *field, position, threat, wanted, ENEMY_BACK_OFF_RADIUS, backOffSpot);
		}

		if (!hasBackOffSpot)
		{
			navigator.Reset();
			return;
		}

		MoveTowards(backOffSpot, deltaTime);
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

	void ChaseComponent::SetPeripheryHalfAngle(float newHalfAngle)
	{
		peripheryHalfAngle = std::max(newHalfAngle, 0.f);
	}

	void ChaseComponent::SetShoutRadius(float newRadius)
	{
		shoutRadius = std::max(newRadius, 0.f);
	}

	float ChaseComponent::GetShoutRadius() const
	{
		return shoutRadius;
	}

	VisionBand ChaseComponent::GetVisionBand() const
	{
		return band;
	}

	void ChaseComponent::SetAwareness(float newGain, float newDecay)
	{
		rates.gain = std::max(newGain, 0.f);
		rates.decay = std::max(newDecay, 0.f);
	}

	bool ChaseComponent::IsSuspicious() const
	{
		return StateOf(awareness) == AwarenessState::Alerted;
	}

	float ChaseComponent::GetAwareness() const
	{
		return awareness;
	}

	AwarenessState ChaseComponent::GetAwarenessState() const
	{
		return StateOf(awareness);
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
		searchSpots = FindHidingSpots(LevelGrid::Current(), *field, investigatePoint, escapeDirection,
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
		memory = Remember(memory, searchTime, SEARCH_TRAVEL_TIME);

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

	void ChaseComponent::SetChaseSpeed(float newChaseSpeed)
	{
		chaseSpeed = newChaseSpeed < 0.f ? 0.f : newChaseSpeed;
	}

	float ChaseComponent::GetChaseSpeed() const
	{
		return chaseSpeed;
	}

	float ChaseComponent::GetWalkSpeed() const
	{
		return walkSpeed;
	}

	// Патруль остаётся шагом, а вот погоня идёт бегом - и тяжёлый ствол укорачивает именно бег.
	void ChaseComponent::ApplyPace()
	{
		if (movement == nullptr || chaseSpeed <= 0.f)
		{
			return;
		}

		// Мёртвого разгонять нечем: смерть обнуляет скорость, и вернуть её - значит пустить труп по полу.
		if (health != nullptr && !health->IsAlive())
		{
			return;
		}

		if (isBackingOff)
		{
			movement->SetSpeed(chaseSpeed * ENEMY_BACK_OFF_PACE);
			return;
		}

		movement->SetSpeed(isChasing ? chaseSpeed : walkSpeed);
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

	const Vector2Df& ChaseComponent::GetEscapeDirection() const
	{
		return escapeDirection;
	}
}
