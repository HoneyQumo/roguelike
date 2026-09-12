#pragma once

#include <functional>
#include <string>
#include <Component.h>
#include <EventList.h>
#include <GameObject.h>
#include "FactionComponent.h"
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
        void SetShooter(XYZEngine::GameObjectId newShooterId, Faction newShooterFaction);
        XYZEngine::SubscriptionId SubscribeHit(std::function<void(const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&, bool)> onHit);
        XYZEngine::SubscriptionId SubscribeExpire(std::function<void(const XYZEngine::Vector2Df&)> onExpire);

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::ColliderComponent* collider = nullptr;

        XYZEngine::Vector2Df direction = {1.f, 0.f};
        float speed = 600.f;
        float damage = 10.f;
        XYZEngine::Cooldown lifetime = XYZEngine::Cooldown::Started(3.f);
        XYZEngine::GameObjectId shooterId = XYZEngine::NO_GAME_OBJECT;
        Faction shooterFaction = Faction::Neutral;
        XYZEngine::EventList<const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&, bool> hitEvent;
        XYZEngine::EventList<const XYZEngine::Vector2Df&> expireEvent;

        bool isHandled = false;

        void OnTrigger(const XYZEngine::Trigger& trigger);
        bool IsFriendly(XYZEngine::GameObject* target) const;
        float GetMaxStep() const;
        XYZEngine::ColliderComponent* FindHit(const XYZEngine::Vector2Df& moved) const;
        void Hit(XYZEngine::ColliderComponent* target);
        void Destroy();
    };
}
