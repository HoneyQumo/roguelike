#include "pch.h"
#include "AimRotationComponent.h"
#include "GameObject.h"
#include "GameWorld.h"
#include <cmath>

namespace XYZEngine
{
    constexpr float MIN_AIM_DISTANCE = 0.01f;
    constexpr float DEGREES_IN_RADIAN = 57.29578f;

    AimRotationComponent::AimRotationComponent(GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetComponent<TransformComponent>();
    }

    void AimRotationComponent::Update(float deltaTime)
    {
        if (!isEnabled)
        {
            return;
        }

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
        transform->SetWorldRotation(std::atan2(aimDirection.y, aimDirection.x) * DEGREES_IN_RADIAN);
    }

    void AimRotationComponent::Render()
    {
    }

    void AimRotationComponent::AimAtCursor()
    {
        isCursorAim = true;
        targetName.clear();
    }

    void AimRotationComponent::AimAtGameObject(const std::string& newTargetName)
    {
        isCursorAim = false;
        targetName = newTargetName;
    }

    void AimRotationComponent::SetMaxDistance(float newMaxDistance)
    {
        maxDistance = newMaxDistance;
    }

    void AimRotationComponent::SetEnabled(bool newIsEnabled)
    {
        isEnabled = newIsEnabled;
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
                input = gameObject->GetComponent<InputComponent>();
            }

            if (input == nullptr)
            {
                return false;
            }

            aimPosition = input->GetMouseWorldPosition();
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

        aimPosition = target->GetComponent<TransformComponent>()->GetWorldPosition();
        return true;
    }
}
