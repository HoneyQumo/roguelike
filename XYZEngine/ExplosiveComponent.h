#pragma once

#include <functional>
#include <string>
#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include "Component.h"
#include "Vector.h"

namespace XYZEngine
{
    class ExplosiveComponent : public Component
    {
    public:
        ExplosiveComponent(GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetRadius(float newRadius);
        void SetCoreRadius(float newCoreRadius);
        void SetCenterDamage(float newCenterDamage);
        void SetEdgeDamagePart(float newEdgeDamagePart);
        void SetSelfDamagePart(float newSelfDamagePart);
        void SetOwnerName(const std::string& newOwnerName);
        void SetExplodeAction(std::function<void(const Vector2Df&)> newExplodeAction);
        void SetHitAction(std::function<void(const Vector2Df&, const Vector2Df&)> newHitAction);

        float GetRadius() const;
        bool HasExploded() const;

        int Explode(const Vector2Df& position);

    private:
        float radius = 64.f;
        float coreRadius = 0.f;
        float centerDamage = 50.f;
        float edgeDamagePart = 0.25f;
        float selfDamagePart = 1.f;
        std::string ownerName;
        bool hasExploded = false;

        std::function<void(const Vector2Df&)> explodeAction;
        std::function<void(const Vector2Df&, const Vector2Df&)> hitAction;

        float DamageAt(float distance) const;
        static bool IsBlocked(const Vector2Df& origin, const Vector2Df& target, const std::vector<sf::FloatRect>& obstacles);
        static bool CrossesRect(const Vector2Df& from, const Vector2Df& to, const sf::FloatRect& rect);
    };
}
