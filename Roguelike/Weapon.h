#pragma once

#include "WeaponLayerComponent.h"
#include <GameObject.h>
#include <SpriteMovementAnimationComponent.h>

namespace RoguelikeGame
{
    /**
    *	Слой оружия: позиция и угол, тоже что у тела, отличается только пивот.
    *	Дульная вспышка висит на оружии, следует за отдачей оружия.
    *	Дочерний объект живёт и умирает вместе с владельцем.
    */
    WeaponLayerComponent* CreateWeapon(XYZEngine::GameObject* owner, WeaponId id, XYZEngine::SpriteMovementAnimationComponent* ownerAnimation);
}
