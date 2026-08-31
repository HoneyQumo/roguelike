#pragma once

#include <memory>
#include "GameObject.h"
#include "Vector.h"
#include "Weapon.h"
#include "StowedWeaponComponent.h"

namespace RoguelikeGame
{
    class Player
    {
    public:
        Player(const XYZEngine::Vector2Df& position);
        XYZEngine::GameObject* GetGameObject();

    private:
        XYZEngine::GameObject* gameObject;
        std::unique_ptr<Weapon> weapon;

        StowedWeaponComponent* CreateStowedWeapon(WeaponId startWeapon, XYZEngine::SpriteMovementAnimationComponent* animation);
    };
}
