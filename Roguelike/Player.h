#pragma once

#include <memory>
#include "GameObject.h"
#include "Vector.h"
#include "BloodPool.h"
#include "Weapon.h"

namespace RoguelikeGame
{
    class Player
    {
    public:
        Player(const XYZEngine::Vector2Df& position);
        XYZEngine::GameObject* GetGameObject();

    private:
        XYZEngine::GameObject* gameObject;
        std::unique_ptr<BloodPool> bloodPool;
        std::unique_ptr<Weapon> weapon;
    };
}
