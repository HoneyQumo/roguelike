#pragma once

#include <string>
#include <GameObject.h>
#include <Vector.h>
#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    class Projectile
    {
    public:
        static void Spawn(const XYZEngine::Vector2Df& position, const XYZEngine::Vector2Df& direction, float damage, float speed,
            XYZEngine::GameObject* shooter, WeaponId weapon);
    };
}
