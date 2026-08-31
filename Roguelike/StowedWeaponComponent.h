#pragma once

#include <Component.h>
#include <TransformComponent.h>
#include <SpriteRendererComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include <SFML/Graphics/Texture.hpp>
#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    class StowedWeaponComponent : public XYZEngine::Component
    {
    public:
        StowedWeaponComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetOwnerAnimation(XYZEngine::SpriteMovementAnimationComponent* newOwnerAnimation);
        void SetWeaponId(WeaponId newWeaponId);

    private:
        XYZEngine::TransformComponent* transform;
        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        XYZEngine::SpriteMovementAnimationComponent* ownerAnimation = nullptr;

        const sf::Texture* texture = nullptr;
    };
}
