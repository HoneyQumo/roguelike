#pragma once

#include <string>
#include <UiLabel.h>
#include <UiProgressBar.h>
#include <UiScreen.h>

namespace RoguelikeGame
{
    struct VitalsHudState
    {
        float healthPart = 1.f;
        float staminaPart = 1.f;
        bool isExhausted = false;
    };

    struct AmmoHudState
    {
        const char* weaponName = nullptr;
        int inMagazine = 0;
        int reserve = 0;
        bool hasMagazine = false;
        bool isReloading = false;
        bool isLow = false;
    };

    class HudScreen : public XYZEngine::UiScreen
    {
    public:
        HudScreen();

        void SetAmmo(const AmmoHudState& state);
        void SetVitals(const VitalsHudState& state);

        const XYZEngine::UiLabel& GetNameLabel() const;
        const XYZEngine::UiLabel& GetAmmoLabel() const;
        const XYZEngine::UiProgressBar& GetHealthBar() const;
        const XYZEngine::UiProgressBar& GetStaminaBar() const;

    private:
        XYZEngine::UiLabel* nameLabel = nullptr;
        XYZEngine::UiLabel* ammoLabel = nullptr;
        XYZEngine::UiProgressBar* healthBar = nullptr;
        XYZEngine::UiProgressBar* staminaBar = nullptr;

        std::string shownName;
        std::string shownAmmo;
    };
}
