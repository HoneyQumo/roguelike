#include "pch.h"
#include "AimRotationComponent.h"
#include "MathUtils.h"
#include "GameObject.h"
#include "GameWorld.h"
#include <cmath>

namespace XYZEngine
{
    constexpr float MIN_AIM_DISTANCE = 0.01f;

    AimRotationComponent::AimRotationComponent(GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
    }

    void AimRotationComponent::Start()
    {
        input = gameObject->GetComponent<InputComponent>();
    }

    void AimRotationComponent::Update(float deltaTime)
    {
        Vector2Df aimPosition;
        if (!TryGetAimPosition(aimPosition))
        {
            return;
        }

        Vector2Df toAim = aimPosition - transform->GetWorldPosition();
        float distance = toAim.GetLength();

        if (distance < MIN_AIM_DISTANCE || (maxDistance > 0.f && distance > maxDistance))
        {
            return;
        }

        aimDirection = (1.f / distance) * toAim;
        transform->SetWorldRotation(DegreesFromDirection(aimDirection));
    }

    void AimRotationComponent::Render()
    {
    }

    void AimRotationComponent::AimAtCursor()
    {
        isCursorAim = true;
        isPointAim = false;
        targetName.clear();
    }

    void AimRotationComponent::AimAtGameObject(const std::string& newTargetName)
    {
        isCursorAim = false;
        isPointAim = false;
        targetName = newTargetName;
    }

    void AimRotationComponent::AimAtPoint(const Vector2Df& point)
    {
        isCursorAim = false;
        isPointAim = true;
        targetName.clear();
        aimPoint = point;
    }

    void AimRotationComponent::StopAiming()
    {
        isCursorAim = false;
        isPointAim = false;
        targetName.clear();
    }

    void AimRotationComponent::SetMaxDistance(float newMaxDistance)
    {
        maxDistance = newMaxDistance;
    }

    const Vector2Df& AimRotationComponent::GetAimDirection() const
    {
        return aimDirection;
    }

    bool AimRotationComponent::TryGetAimPosition(Vector2Df& aimPosition)
    {
        if (isCursorAim)
        {
            if (input == nullptr)
            {
                return false;
            }

            aimPosition = input->GetMouseWorldPosition();
            return true;
        }

        if (isPointAim)
        {
            aimPosition = aimPoint;
            return true;
        }

        if (targetName.empty())
        {
            return false;
        }

        GameObject* target = GameWorld::Instance()->FindGameObject(targetName);
        if (target == nullptr)
        {
            return false;
        }

        aimPosition = target->GetTransform()->GetWorldPosition();
        return true;
    }
}
