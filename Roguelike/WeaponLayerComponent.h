#pragma once

#include <Component.h>
#include <TransformComponent.h>
#include <SpriteRendererComponent.h>
#include <SpriteAnimationComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include <SFML/Graphics/Texture.hpp>
#include "SpriteAtlas.h"
#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    class WeaponLayerComponent : public XYZEngine::Component
    {
    public:
        WeaponLayerComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetOwnerAnimation(XYZEngine::SpriteMovementAnimationComponent* newOwnerAnimation);
        void SetWeaponId(WeaponId newWeaponId);
        void PlayMuzzleFlash();

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        XYZEngine::SpriteMovementAnimationComponent* ownerAnimation = nullptr;

        XYZEngine::TransformComponent* muzzleFlashTransform = nullptr;
        XYZEngine::SpriteRendererComponent* muzzleFlashRenderer = nullptr;
        XYZEngine::SpriteAnimationComponent* muzzleFlash = nullptr;
        bool hasMuzzleFlash = false;

        float recoil = 1.f;

        // Четыре варианта хвата из weapons.png: обычный, рука на магазине, магазин вынут, без перчаток.
        const sf::Texture* variants[WEAPON_VARIANTS] = {nullptr, nullptr, nullptr, nullptr};
        int currentVariant = WEAPON_DEFAULT_VARIANT;

        void CreateMuzzleFlash();
        static FrameOffset GetFrameOffset(XYZEngine::MovementAnimation animation, int frame);
        static float GetFrameRotation(XYZEngine::MovementAnimation animation, int frame);
        static int GetFrameVariant(XYZEngine::MovementAnimation animation, int frame);
        static bool IsWeaponHidden(XYZEngine::MovementAnimation animation, int frame);
        void ShowVariant(int variant);
    };
}
