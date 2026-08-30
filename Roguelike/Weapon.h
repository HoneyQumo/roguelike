#pragma once

#include <GameObject.h>
#include <TransformComponent.h>
#include <SpriteRendererComponent.h>
#include <SpriteAnimationComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    /**	
    *	Слой оружия: позиция и угол, тоже что у тела, отличается только пивот.
    *	Дульная вспышка висит на оружии, следует за отдачей оружия.
    */
    class Weapon
    {
    public:
        Weapon(XYZEngine::GameObject* owner, WeaponId id, XYZEngine::SpriteMovementAnimationComponent* ownerAnimation);

        XYZEngine::GameObject* GetGameObject();
        XYZEngine::SpriteRendererComponent* GetRenderer();
        XYZEngine::SpriteAnimationComponent* GetMuzzleFlash();

    private:
        XYZEngine::GameObject* gameObject;
        XYZEngine::SpriteRendererComponent* renderer;
        XYZEngine::SpriteAnimationComponent* muzzleFlash = nullptr;

        void CreateMuzzleFlash(const WeaponDefinition& weapon);
    };
}
