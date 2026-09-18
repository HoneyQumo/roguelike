#include "CameraDirectorComponent.h"
#include "CameraFollow.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include <CameraComponent.h>
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

        XYZEngine::Vector2Df wanted = KeepInsideLevel(Destination());

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
            aim = ApproachPoint(aim, wanted, CAMERA_FOLLOW_TIME, deltaTime);
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
            Place(follow->GetTransform()->GetWorldPosition());
        }
    }

    // Без сетки или камеры зажимать нечем - так бывает на экранах вне уровня.
    XYZEngine::Vector2Df CameraDirectorComponent::KeepInsideLevel(const XYZEngine::Vector2Df& point) const
    {
        const LevelGrid& grid = LevelGrid::Current();
        if (grid.IsEmpty() || gameObject == nullptr)
        {
            return point;
        }

        auto camera = gameObject->GetComponent<XYZEngine::CameraComponent>();
        if (camera == nullptr)
        {
            return point;
        }

        XYZEngine::Vector2Df size = camera->GetViewSize();
        XYZEngine::Vector2Df half = {0.5f * size.x, 0.5f * size.y};

        // ToWorld даёт центр клетки, отсюда половина тайла по краям.
        XYZEngine::Vector2Df corner = grid.ToWorld(grid.GetWidth() - 1, 0);
        XYZEngine::Vector2Df min = {-0.5f * TILE_SIZE, -0.5f * TILE_SIZE};
        XYZEngine::Vector2Df max = {corner.x + 0.5f * TILE_SIZE, corner.y + 0.5f * TILE_SIZE};

        return ClampToBounds(point, half, min, max);
    }

    void CameraDirectorComponent::Place(const XYZEngine::Vector2Df& point)
    {
        aim = KeepInsideLevel(point);

        if (transform != nullptr)
        {
            transform->SetWorldPosition(aim);
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
            Place(follow->GetTransform()->GetWorldPosition());
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
