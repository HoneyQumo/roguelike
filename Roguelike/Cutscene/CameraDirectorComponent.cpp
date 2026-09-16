#include "CameraDirectorComponent.h"
#include <GameObject.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    CameraDirectorComponent::CameraDirectorComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
    }

    void CameraDirectorComponent::Update(float deltaTime)
    {
        if (transform == nullptr)
        {
            return;
        }

        XYZEngine::Vector2Df wanted = Destination();

        if (travelled < travel)
        {
            travelled += deltaTime;

            float part = travel > 0.f ? travelled / travel : 1.f;
            part = part > 1.f ? 1.f : part;

            // Мягкий разгон и торможение: линейный проезд читается как рывок.
            float eased = part * part * (3.f - 2.f * part);
            aim = from + (wanted - from) * eased;
        }
        else
        {
            aim = wanted;
        }

        transform->SetWorldPosition(aim);
    }

    void CameraDirectorComponent::Render()
    {
    }

    void CameraDirectorComponent::SetFollow(XYZEngine::GameObject* newFollow)
    {
        follow = newFollow;

        if (!isAway && follow != nullptr)
        {
            aim = follow->GetTransform()->GetWorldPosition();

            if (transform != nullptr)
            {
                transform->SetWorldPosition(aim);
            }
        }
    }

    void CameraDirectorComponent::LookAt(const XYZEngine::Vector2Df& newPoint, float seconds)
    {
        point = newPoint;
        target = nullptr;
        isAway = true;

        StartMove(seconds);
    }

    void CameraDirectorComponent::LookAt(XYZEngine::GameObject* newTarget, float seconds)
    {
        if (newTarget == nullptr)
        {
            return;
        }

        target = newTarget;
        isAway = true;

        StartMove(seconds);
    }

    void CameraDirectorComponent::LookAtFollow(float seconds)
    {
        target = nullptr;
        isAway = false;

        StartMove(seconds);
    }

    // Камера возвращается к герою мгновенно: сцену оборвали, и красоту разводить уже некогда.
    void CameraDirectorComponent::Release()
    {
        target = nullptr;
        isAway = false;
        travel = 0.f;
        travelled = 0.f;

        if (follow != nullptr)
        {
            aim = follow->GetTransform()->GetWorldPosition();

            if (transform != nullptr)
            {
                transform->SetWorldPosition(aim);
            }
        }
    }

    bool CameraDirectorComponent::IsAway() const
    {
        return isAway;
    }

    bool CameraDirectorComponent::IsMoving() const
    {
        return travelled < travel;
    }

    const XYZEngine::Vector2Df& CameraDirectorComponent::GetAim() const
    {
        return aim;
    }

    XYZEngine::Vector2Df CameraDirectorComponent::Destination() const
    {
        if (isAway)
        {
            return target != nullptr ? target->GetTransform()->GetWorldPosition() : point;
        }

        return follow != nullptr ? follow->GetTransform()->GetWorldPosition() : aim;
    }

    void CameraDirectorComponent::StartMove(float seconds)
    {
        from = aim;
        travel = seconds > 0.f ? seconds : 0.f;
        travelled = 0.f;
    }
}
