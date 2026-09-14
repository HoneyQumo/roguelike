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
        float armorPart = 0.f;
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

        void ShowNotice(const std::string& text);
        void SetPrompt(const std::string& text);
        const XYZEngine::UiLabel& GetPromptLabel() const;
        void Update(float deltaTime) override;
        bool IsNoticeShown() const;

        const XYZEngine::UiLabel& GetNameLabel() const;
        const XYZEngine::UiLabel& GetAmmoLabel() const;
        const XYZEngine::UiProgressBar& GetHealthBar() const;
        const XYZEngine::UiProgressBar& GetStaminaBar() const;
        const XYZEngine::UiProgressBar& GetArmorBar() const;

    private:
        XYZEngine::UiLabel* nameLabel = nullptr;
        XYZEngine::UiLabel* ammoLabel = nullptr;
        XYZEngine::UiProgressBar* healthBar = nullptr;
        XYZEngine::UiProgressBar* staminaBar = nullptr;
        XYZEngine::UiProgressBar* armorBar = nullptr;
        XYZEngine::UiLabel* noticeLabel = nullptr;
        XYZEngine::UiLabel* promptLabel = nullptr;
        float noticeTimeLeft = 0.f;

        std::string shownName;
        std::string shownAmmo;
    };
}
