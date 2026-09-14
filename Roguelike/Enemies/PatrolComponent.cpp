#include "PatrolComponent.h"
#include "ChaseComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "PathService.h"
#include <AimRotationComponent.h>
#include <DebugDraw.h>
#include <MathUtils.h>
#include <GameObject.h>

using namespace XYZEngine;

namespace RoguelikeGame
{
    PatrolComponent::PatrolComponent(GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
    }

    void PatrolComponent::Start()
    {
        movement = gameObject->GetComponent<MovementComponent>();
        aim = gameObject->GetComponent<AimRotationComponent>();
        chase = gameObject->GetComponent<ChaseComponent>();
    }

    void PatrolComponent::SetPoints(std::vector<PatrolStop> newPoints)
    {
        points = std::move(newPoints);
        index = 0u;
        isLooking = false;
        navigator.Reset();
    }

    const std::vector<PatrolStop>& PatrolComponent::GetPoints() const
    {
        return points;
    }

    bool PatrolComponent::IsWalking() const
    {
        return points.size() > 1u && !isLooking && (chase == nullptr || !chase->IsEngaged());
    }

    bool PatrolComponent::IsLooking() const
    {
        return isLooking;
    }

    void PatrolComponent::SetLook(float newLookTime, float newHalfSweep)
    {
        lookTime = newLookTime;
        lookHalfSweep = newHalfSweep;
    }

    void PatrolComponent::StartLook()
    {
        isLooking = true;
        lookElapsed = 0.f;
        lookPlan.baseAngle = transform->GetWorldRotation();
        lookPlan.halfSweep = lookHalfSweep;
        lookPlan.duration = lookTime;

        movement->SetDirection({0.f, 0.f});
        navigator.Reset();
    }

    void PatrolComponent::AimAtAngle(float degrees)
    {
        if (aim == nullptr)
        {
            return;
        }

        aim->AimAtPoint(transform->GetWorldPosition() + DirectionFromDegrees(degrees) * LOOK_AIM_DISTANCE);
    }

    std::size_t PatrolComponent::GetPointIndex() const
    {
        return index;
    }

    void PatrolComponent::Update(float deltaTime)
    {
        if (points.empty() || movement == nullptr)
        {
            return;
        }

        if (chase != nullptr && chase->IsEngaged())
        {
            wasEngaged = true;
            isLooking = false;
            navigator.Reset();
            return;
        }

        Vector2Df position = transform->GetWorldPosition();

        if (wasEngaged)
        {
            wasEngaged = false;
            index = NearestPatrolIndex(points, position);
            navigator.Reset();
        }

        if (points.size() == 1u)
        {
            if (aim != nullptr)
            {
                aim->AimAtPoint(points[0].position);
            }

            return;
        }

        if (isLooking)
        {
            lookElapsed += deltaTime;
            AimAtAngle(LookAngleAt(lookPlan, lookElapsed));
            movement->SetDirection({0.f, 0.f});

            if (IsLookDone(lookPlan, lookElapsed))
            {
                isLooking = false;
                index = NextPatrolIndex(index, points.size());
                navigator.Reset();
            }

            return;
        }

        if (HasReachedPatrolPoint(points[index].position, position, ENEMY_ROUTE_ARRIVE_DISTANCE))
        {
            if (points[index].isWatch && lookTime > 0.f)
            {
                StartLook();
                return;
            }

            index = NextPatrolIndex(index, points.size());
            navigator.Reset();
        }

        if (aim != nullptr)
        {
            aim->AimAtPoint(points[index].position);
        }

        WalkTo(points[index].position, deltaTime);
    }

    void PatrolComponent::WalkTo(const Vector2Df& goal, float deltaTime)
    {
        Vector2Df position = transform->GetWorldPosition();
        movement->SetDirection(navigator.Steer(position, goal, movement->GetSpeed(), deltaTime));
    }

    void PatrolComponent::DrawRoute() const
    {
        if (!DebugDraw::Instance()->IsEnabled() || points.size() < 2u)
        {
            return;
        }

        for (std::size_t step = 0u; step < points.size(); step++)
        {
            const Vector2Df& from = points[step].position;
            const Vector2Df& to = points[NextPatrolIndex(step, points.size())].position;

            DebugDraw::Instance()->DrawLine(from, to, DEBUG_PATROL_COLOR);
        }
    }

    void PatrolComponent::Render()
    {
        DrawRoute();
    }
}
