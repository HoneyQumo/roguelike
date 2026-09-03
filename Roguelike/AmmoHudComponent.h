#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <Component.h>
#include <WeaponComponent.h>
#include "WeaponCatalog.h"
#include "PlayerLoadoutComponent.h"

namespace RoguelikeGame
{
    class AmmoHudComponent : public XYZEngine::Component
    {
    public:
        AmmoHudComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);

    private:
        XYZEngine::WeaponComponent* weapon = nullptr;
        PlayerLoadoutComponent* loadout = nullptr;

        std::string targetName;

        sf::Text nameText;
        sf::Text ammoText;
        bool isFontReady = false;
        bool hasShownWeapon = false;
        WeaponId shownWeapon = WeaponId::Ak47;

        void FindTarget();
        void ShowWeaponName(WeaponId weaponId);
        std::string GetAmmoLine() const;
        sf::Color GetAmmoColor() const;
    };
}
