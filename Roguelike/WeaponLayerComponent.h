#pragma once

#include <Component.h>
#include <TransformComponent.h>
#include <SpriteRendererComponent.h>
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

        void Update(float deltaTime) override;
        void Render() override;

        void SetOwnerAnimation(XYZEngine::SpriteMovementAnimationComponent* newOwnerAnimation);
        void SetRecoil(float newRecoil);
        void SetWeaponId(WeaponId newWeaponId);

    private:
        XYZEngine::TransformComponent* transform;
        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        XYZEngine::SpriteMovementAnimationComponent* ownerAnimation = nullptr;

        float recoil = 1.f;

        // Три варианта хвата из weapons.png: обычный, рука на магазине, магазин вынут.
        const sf::Texture* variants[WEAPON_VARIANTS] = {nullptr, nullptr, nullptr};
        int currentVariant = WEAPON_DEFAULT_VARIANT;

        static FrameOffset GetFrameOffset(XYZEngine::MovementAnimation animation, int frame);
        static int GetFrameVariant(XYZEngine::MovementAnimation animation, int frame);
        void ShowVariant(int variant);
    };
}
