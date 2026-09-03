#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics/Texture.hpp>
#include <Component.h>
#include <SpriteRendererComponent.h>
#include <WeaponComponent.h>

namespace RoguelikeGame
{
    class ReloadIndicatorComponent : public XYZEngine::Component
    {
    public:
        ReloadIndicatorComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);

    private:
        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        XYZEngine::WeaponComponent* weapon = nullptr;

        std::string targetName;

        const sf::Texture* crosshairTexture = nullptr;
        std::vector<const sf::Texture*> magazineFrames;

        int currentFrame = -1;
        bool isIndicatorShown = false;

        void FindWeapon();
        void ShowMagazineFrame(int frame);
        void ShowCrosshair();
    };
}
