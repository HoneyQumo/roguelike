#pragma once

#include <functional>
#include <string>
#include <Component.h>
#include <TransformComponent.h>
#include <ColliderComponent.h>
#include <Vector.h>
#include <Cooldown.h>

namespace RoguelikeGame
{
    class ProjectileComponent : public XYZEngine::Component
    {
    public:
        ProjectileComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetDirection(const XYZEngine::Vector2Df& newDirection);
        void SetSpeed(float newSpeed);
        void SetDamage(float newDamage);
        void SetLifetime(float newLifetime);
        void SetShooterName(const std::string& newShooterName);
        void SetHitAction(std::function<void(const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&, bool)> newHitAction);
        void SetExpireAction(std::function<void(const XYZEngine::Vector2Df&)> newExpireAction);

    private:
        XYZEngine::TransformComponent* transform;
        XYZEngine::ColliderComponent* collider = nullptr;

        XYZEngine::Vector2Df direction = {1.f, 0.f};
        float speed = 600.f;
        float damage = 10.f;
        XYZEngine::Cooldown lifetime = XYZEngine::Cooldown::Started(3.f);
        std::string shooterName;
        std::function<void(const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&, bool)> hitAction;
        std::function<void(const XYZEngine::Vector2Df&)> expireAction;

        bool isHandled = false;

        void OnTrigger(const XYZEngine::Trigger& trigger);
        void Destroy();
    };
}
