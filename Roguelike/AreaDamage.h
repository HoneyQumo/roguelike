#pragma once

#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include <GameObject.h>
#include <Vector.h>
#include "HealthComponent.h"

namespace RoguelikeGame
{
    struct AreaTarget
    {
        XYZEngine::GameObject* gameObject;
        HealthComponent* health;
        XYZEngine::Vector2Df position;
        XYZEngine::Vector2Df direction;
        float distance;
    };

    struct AreaQuery
    {
        std::vector<AreaTarget> targets;
        std::vector<sf::FloatRect> obstacles;
    };

    AreaQuery QueryDamageArea(const XYZEngine::Vector2Df& center, float radius, XYZEngine::GameObject* ignore);
}
