#pragma once

#include <string>
#include <Component.h>
#include "HudScreen.h"
#include "PlayerLoadoutComponent.h"
#include "WeaponComponent.h"

namespace RoguelikeGame
{
    class PlayerHudBinderComponent : public XYZEngine::Component
    {
    public:
        PlayerHudBinderComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetTargetName(const std::string& newTargetName);
        void SetScreen(HudScreen* newScreen);

    private:
        HudScreen* screen = nullptr;
        WeaponComponent* weapon = nullptr;
        PlayerLoadoutComponent* loadout = nullptr;

        std::string targetName;

        void FindTarget();
        AmmoHudState ReadState() const;
    };
}
