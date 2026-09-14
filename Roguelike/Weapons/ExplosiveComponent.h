#pragma once

#include <functional>
#include <string>
#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include <Component.h>
#include <EventList.h>
#include <GameObject.h>
#include <TransformComponent.h>
#include <Vector.h>
#include "Faction.h"

namespace RoguelikeGame
{
    class ExplosiveComponent : public XYZEngine::Component
    {
    public:
        ExplosiveComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetRadius(float newRadius);
        void SetCoreRadius(float newCoreRadius);
        void SetCenterDamage(float newCenterDamage);
        void SetEdgeDamagePart(float newEdgeDamagePart);
        void SetSelfDamagePart(float newSelfDamagePart);
        void SetOwner(XYZEngine::GameObjectId newOwnerId, const std::string& newOwnerName = std::string(), Faction newOwnerFaction = Faction::Neutral);
        XYZEngine::SubscriptionId SubscribeExplode(std::function<void(const XYZEngine::Vector2Df&)> onExplode);
        XYZEngine::SubscriptionId SubscribeHit(std::function<void(const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&)> onHit);

        float GetRadius() const;
        bool HasExploded() const;

        int Explode(const XYZEngine::Vector2Df& position);

    private:
        XYZEngine::TransformComponent* transform = nullptr;

        float radius = 64.f;
        float coreRadius = 0.f;
        float centerDamage = 50.f;
        float edgeDamagePart = 0.25f;
        float selfDamagePart = 1.f;
        XYZEngine::GameObjectId ownerId = XYZEngine::NO_GAME_OBJECT;
        std::string ownerName;
        Faction ownerFaction = Faction::Neutral;
        bool hasExploded = false;

        XYZEngine::EventList<const XYZEngine::Vector2Df&> explodeEvent;
        XYZEngine::EventList<const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&> hitEvent;

        float DamageAt(float distance) const;
        static bool IsBlocked(const XYZEngine::Vector2Df& origin, const XYZEngine::Vector2Df& target, const std::vector<sf::FloatRect>& obstacles);
        static bool CrossesRect(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to, const sf::FloatRect& rect);
    };
}
