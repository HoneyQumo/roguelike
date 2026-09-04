#include "ExplosiveComponent.h"
#include "AreaDamage.h"
#include "GameSettings.h"
#include <DebugDraw.h>
#include <GameObject.h>
#include "HealthComponent.h"
#include <TransformComponent.h>
#include <PhysicsSystem.h>
#include <LoggerRegistry.h>
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace XYZEngine;

namespace RoguelikeGame
{
    constexpr float MIN_SEGMENT_LENGTH = 0.0001f;

    ExplosiveComponent::ExplosiveComponent(GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
    }

    void ExplosiveComponent::Update(float deltaTime)
    {
    }

    void ExplosiveComponent::Render()
    {
        if (DebugDraw::Instance()->IsEnabled() && !hasExploded)
        {
            DebugDraw::Instance()->DrawCircle(transform->GetWorldPosition(), radius, DEBUG_BLAST_COLOR);
        }
    }

    void ExplosiveComponent::SetRadius(float newRadius)
    {
        assert(newRadius > 0.f);
        radius = std::max(newRadius, 1.f);
    }

    void ExplosiveComponent::SetCoreRadius(float newCoreRadius)
    {
        assert(newCoreRadius >= 0.f);
        coreRadius = std::max(newCoreRadius, 0.f);
    }

    void ExplosiveComponent::SetCenterDamage(float newCenterDamage)
    {
        assert(newCenterDamage >= 0.f);
        centerDamage = std::max(newCenterDamage, 0.f);
    }

    void ExplosiveComponent::SetEdgeDamagePart(float newEdgeDamagePart)
    {
        assert(newEdgeDamagePart >= 0.f && newEdgeDamagePart <= 1.f);
        edgeDamagePart = std::clamp(newEdgeDamagePart, 0.f, 1.f);
    }

    void ExplosiveComponent::SetSelfDamagePart(float newSelfDamagePart)
    {
        assert(newSelfDamagePart >= 0.f);
        selfDamagePart = std::max(newSelfDamagePart, 0.f);
    }

    void ExplosiveComponent::SetOwnerId(GameObjectId newOwnerId)
    {
        ownerId = newOwnerId;
    }

    SubscriptionId ExplosiveComponent::SubscribeExplode(std::function<void(const Vector2Df&)> onExplode)
    {
        return explodeEvent.Subscribe(std::move(onExplode));
    }

    SubscriptionId ExplosiveComponent::SubscribeHit(std::function<void(const Vector2Df&, const Vector2Df&)> onHit)
    {
        return hitEvent.Subscribe(std::move(onHit));
    }

    float ExplosiveComponent::GetRadius() const
    {
        return radius;
    }

    bool ExplosiveComponent::HasExploded() const
    {
        return hasExploded;
    }

    int ExplosiveComponent::Explode(const Vector2Df& position)
    {
        if (hasExploded)
        {
            return 0;
        }

        hasExploded = true;

        AreaQuery query = QueryDamageArea(position, radius, gameObject);

        int hits = 0;

        for (const AreaTarget& target : query.targets)
        {
            if (target.distance > coreRadius && IsBlocked(position, target.position, query.obstacles))
            {
                continue;
            }

            float damage = DamageAt(target.distance);
            if (ownerId != NO_GAME_OBJECT && target.gameObject->GetId() == ownerId)
            {
                damage *= selfDamagePart;
            }

            if (damage <= 0.f)
            {
                continue;
            }

            target.health->TakeDamage(damage);
            hits++;

            hitEvent.Invoke(target.position, target.direction);
        }

        explodeEvent.Invoke(position);

        LOG_INFO(gameObject->GetName() + " explodes for " + std::to_string(static_cast<int>(centerDamage))
                 + " damage in radius " + std::to_string(static_cast<int>(radius)) + ", targets hit: " + std::to_string(hits));

        return hits;
    }

    float ExplosiveComponent::DamageAt(float distance) const
    {
        float part = std::clamp(distance / radius, 0.f, 1.f);
        return centerDamage * (1.f - part * (1.f - edgeDamagePart));
    }

    bool ExplosiveComponent::IsBlocked(const Vector2Df& origin, const Vector2Df& target, const std::vector<sf::FloatRect>& obstacles)
    {
        for (const sf::FloatRect& obstacle : obstacles)
        {
            if (obstacle.contains(origin.x, origin.y) || obstacle.contains(target.x, target.y))
            {
                continue;
            }

            if (CrossesRect(origin, target, obstacle))
            {
                return true;
            }
        }

        return false;
    }

    bool ExplosiveComponent::CrossesRect(const Vector2Df& from, const Vector2Df& to, const sf::FloatRect& rect)
    {
        float deltaX = to.x - from.x;
        float deltaY = to.y - from.y;

        float enter = 0.f;
        float exit = 1.f;

        if (std::fabs(deltaX) < MIN_SEGMENT_LENGTH)
        {
            if (from.x < rect.left || from.x > rect.left + rect.width)
            {
                return false;
            }
        }
        else
        {
            float first = (rect.left - from.x) / deltaX;
            float second = (rect.left + rect.width - from.x) / deltaX;
            if (first > second)
            {
                std::swap(first, second);
            }

            enter = std::max(enter, first);
            exit = std::min(exit, second);
            if (enter > exit)
            {
                return false;
            }
        }

        if (std::fabs(deltaY) < MIN_SEGMENT_LENGTH)
        {
            if (from.y < rect.top || from.y > rect.top + rect.height)
            {
                return false;
            }
        }
        else
        {
            float first = (rect.top - from.y) / deltaY;
            float second = (rect.top + rect.height - from.y) / deltaY;
            if (first > second)
            {
                std::swap(first, second);
            }

            enter = std::max(enter, first);
            exit = std::min(exit, second);
            if (enter > exit)
            {
                return false;
            }
        }

        return true;
    }
}
