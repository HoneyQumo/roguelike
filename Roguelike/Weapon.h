#pragma once

#include <GameObject.h>
#include <TransformComponent.h>
#include <SpriteRendererComponent.h>
#include <SpriteAnimationComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include "WeaponCatalog.h"
#include "WeaponLayerComponent.h"

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

        void SetWeaponId(WeaponId id);
        void PlayMuzzleFlash();

        XYZEngine::GameObject* GetGameObject();
        XYZEngine::SpriteRendererComponent* GetRenderer();
        XYZEngine::SpriteAnimationComponent* GetMuzzleFlash();

    private:
        XYZEngine::GameObject* gameObject;
        XYZEngine::SpriteRendererComponent* renderer;
        WeaponLayerComponent* layer = nullptr;
        XYZEngine::SpriteAnimationComponent* muzzleFlash = nullptr;
        XYZEngine::SpriteRendererComponent* muzzleFlashRenderer = nullptr;
        XYZEngine::TransformComponent* muzzleFlashTransform = nullptr;
        bool hasMuzzleFlash = false;

        void CreateMuzzleFlash();
    };
}
