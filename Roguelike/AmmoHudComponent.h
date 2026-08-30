#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <Component.h>
#include <WeaponComponent.h>

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

        std::string targetName;

        sf::Text ammoText;
        sf::Text statusText;
        bool isFontReady = false;

        void FindWeapon();
        std::string GetAmmoLine() const;
        std::string GetStatusLine() const;
        sf::Color GetAmmoColor() const;
    };
}
