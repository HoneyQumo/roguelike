#include "PatrolComponent.h"
#include "ChaseComponent.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "PathService.h"
#include <AimRotationComponent.h>
#include <DebugDraw.h>
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

    void PatrolComponent::SetPoints(std::vector<Vector2Df> newPoints)
    {
        points = std::move(newPoints);
        index = 0u;
        navigator.Reset();
    }

    const std::vector<Vector2Df>& PatrolComponent::GetPoints() const
    {
        return points;
    }

    bool PatrolComponent::IsWalking() const
    {
        return points.size() > 1u && (chase == nullptr || !chase->IsEngaged());
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
                aim->AimAtPoint(points[0]);
            }

            return;
        }

        if (HasReachedPatrolPoint(points[index], position, ENEMY_ROUTE_ARRIVE_DISTANCE))
        {
            index = NextPatrolIndex(index, points.size());
            navigator.Reset();
        }

        if (aim != nullptr)
        {
            aim->AimAtPoint(points[index]);
        }

        WalkTo(points[index], deltaTime);
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
            const Vector2Df& from = points[step];
            const Vector2Df& to = points[NextPatrolIndex(step, points.size())];

            DebugDraw::Instance()->DrawLine(from, to, DEBUG_PATROL_COLOR);
        }
    }

    void PatrolComponent::Render()
    {
        DrawRoute();
    }
}
