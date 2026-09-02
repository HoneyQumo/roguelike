#pragma once

#include <functional>
#include <string>
#include "Component.h"
#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "Vector.h"

namespace XYZEngine
{
    class ProjectileComponent : public Component
    {
    public:
        ProjectileComponent(GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetDirection(const Vector2Df& newDirection);
        void SetSpeed(float newSpeed);
        void SetDamage(float newDamage);
        void SetLifetime(float newLifetime);
        void SetShooterName(const std::string& newShooterName);
        void SetHitAction(std::function<void(const Vector2Df&, const Vector2Df&, bool)> newHitAction);
        void SetExpireAction(std::function<void(const Vector2Df&)> newExpireAction);

    private:
        TransformComponent* transform;
        ColliderComponent* collider = nullptr;

        Vector2Df direction = {1.f, 0.f};
        float speed = 600.f;
        float damage = 10.f;
        float lifetime = 3.f;
        std::string shooterName;
        std::function<void(const Vector2Df&, const Vector2Df&, bool)> hitAction;
        std::function<void(const Vector2Df&)> expireAction;

        bool isHandled = false;

        void OnTrigger(const Trigger& trigger);
        void Destroy();
    };
}
