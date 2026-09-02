#include "pch.h"
#include "ExplosiveComponent.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include "TransformComponent.h"
#include "PhysicsSystem.h"
#include "LoggerRegistry.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace XYZEngine
{
    constexpr float MIN_SEGMENT_LENGTH = 0.0001f;

    ExplosiveComponent::ExplosiveComponent(GameObject* gameObject) : Component(gameObject)
    {
    }

    void ExplosiveComponent::Update(float deltaTime)
    {
    }

    void ExplosiveComponent::Render()
    {
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
        edgeDamagePart = std::min(std::max(newEdgeDamagePart, 0.f), 1.f);
    }

    void ExplosiveComponent::SetSelfDamagePart(float newSelfDamagePart)
    {
        assert(newSelfDamagePart >= 0.f);
        selfDamagePart = std::max(newSelfDamagePart, 0.f);
    }

    void ExplosiveComponent::SetOwnerName(const std::string& newOwnerName)
    {
        ownerName = newOwnerName;
    }

    void ExplosiveComponent::SetExplodeAction(std::function<void(const Vector2Df&)> newExplodeAction)
    {
        explodeAction = newExplodeAction;
    }

    void ExplosiveComponent::SetHitAction(std::function<void(const Vector2Df&, const Vector2Df&)> newHitAction)
    {
        hitAction = newHitAction;
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

        sf::FloatRect area(position.x - radius, position.y - radius, 2.f * radius, 2.f * radius);

        std::vector<ColliderComponent*> found = PhysicsSystem::Instance()->Overlap(area);

        std::vector<sf::FloatRect> obstacles;
        std::vector<GameObject*> targets;

        for (auto collider : found)
        {
            GameObject* candidate = collider->GetGameObject();
            if (candidate == nullptr || candidate == gameObject)
            {
                continue;
            }

            auto health = candidate->GetComponent<HealthComponent>();
            if (health == nullptr)
            {
                if (!collider->IsTrigger())
                {
                    obstacles.push_back(collider->GetBounds());
                }
                continue;
            }

            if (!health->IsAlive() || health->IsInvulnerable())
            {
                continue;
            }

            if (std::find(targets.begin(), targets.end(), candidate) == targets.end())
            {
                targets.push_back(candidate);
            }
        }

        int hits = 0;

        for (auto target : targets)
        {
            auto targetTransform = target->GetComponent<TransformComponent>();
            if (targetTransform == nullptr)
            {
                continue;
            }

            Vector2Df targetPosition = targetTransform->GetWorldPosition();
            float distance = (targetPosition - position).GetLength();
            if (distance > radius)
            {
                continue;
            }

            if (distance > coreRadius && IsBlocked(position, targetPosition, obstacles))
            {
                continue;
            }

            auto health = target->GetComponent<HealthComponent>();
            if (health == nullptr || !health->IsAlive() || health->IsInvulnerable())
            {
                continue;
            }

            float damage = DamageAt(distance);
            if (!ownerName.empty() && target->GetName() == ownerName)
            {
                damage *= selfDamagePart;
            }

            if (damage <= 0.f)
            {
                continue;
            }

            health->TakeDamage(damage);
            hits++;

            if (hitAction != nullptr)
            {
                Vector2Df hitDirection = distance > 0.f ? (1.f / distance) * (targetPosition - position) : Vector2Df(1.f, 0.f);
                hitAction(targetPosition, hitDirection);
            }
        }

        if (explodeAction != nullptr)
        {
            explodeAction(position);
        }

        LOG_INFO(gameObject->GetName() + " explodes for " + std::to_string(static_cast<int>(centerDamage))
                 + " damage in radius " + std::to_string(static_cast<int>(radius)) + ", targets hit: " + std::to_string(hits));

        return hits;
    }

    float ExplosiveComponent::DamageAt(float distance) const
    {
        float part = std::min(std::max(distance / radius, 0.f), 1.f);
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
