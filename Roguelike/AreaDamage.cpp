#include "AreaDamage.h"
#include <PhysicsSystem.h>
#include <algorithm>

using namespace XYZEngine;

namespace RoguelikeGame
{
    AreaQuery QueryDamageArea(const Vector2Df& center, float radius, GameObject* ignore)
    {
        AreaQuery query;

        sf::FloatRect area(center.x - radius, center.y - radius, 2.f * radius, 2.f * radius);

        for (auto collider : PhysicsSystem::Instance()->Overlap(area))
        {
            GameObject* candidate = collider->GetGameObject();
            if (candidate == nullptr || candidate == ignore)
            {
                continue;
            }

            auto health = candidate->GetComponent<HealthComponent>();
            if (health == nullptr)
            {
                if (!collider->IsTrigger())
                {
                    query.obstacles.push_back(collider->GetBounds());
                }

                continue;
            }

            if (!health->IsAlive() || health->IsInvulnerable())
            {
                continue;
            }

            auto transform = candidate->GetTransform();
            if (transform == nullptr)
            {
                continue;
            }

            bool isKnown = std::any_of(query.targets.begin(), query.targets.end(),
                                       [candidate](const AreaTarget& target) { return target.gameObject == candidate; });
            if (isKnown)
            {
                continue;
            }

            Vector2Df position = transform->GetWorldPosition();
            Vector2Df toTarget = position - center;
            float distance = toTarget.GetLength();
            if (distance > radius)
            {
                continue;
            }

            query.targets.push_back({candidate, health, position, toTarget.Normalized(), distance});
        }

        return query;
    }
}
