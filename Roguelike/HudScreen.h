#pragma once

#include <string>
#include <UiLabel.h>
#include <UiScreen.h>

namespace RoguelikeGame
{
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

        const XYZEngine::UiLabel& GetNameLabel() const;
        const XYZEngine::UiLabel& GetAmmoLabel() const;

    private:
        XYZEngine::UiLabel* nameLabel = nullptr;
        XYZEngine::UiLabel* ammoLabel = nullptr;

        std::string shownName;
        std::string shownAmmo;
    };
}
