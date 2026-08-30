#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <Component.h>
#include <WeaponComponent.h>
#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    class AmmoHudComponent : public XYZEngine::Component
    {
    public:
        AmmoHudComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);
        void SetWeapon(WeaponId newWeaponId);

    private:
        XYZEngine::WeaponComponent* weapon = nullptr;

        std::string targetName;

        sf::Text nameText;
        sf::Text ammoText;
        bool isFontReady = false;

        void FindWeapon();
        std::string GetAmmoLine() const;
        sf::Color GetAmmoColor() const;
    };
}
