#pragma once

#include <string>
#include <UiLabel.h>
#include <UiProgressBar.h>
#include <UiScreen.h>
#include <UiWidget.h>

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

    struct WaveHudState
    {
        int current = 0;
        int total = 0;
        int left = 0;
        float nextIn = 0.f;
        bool isRunning = false;
        bool isPause = false;
    };

    class HudScreen : public XYZEngine::UiScreen
    {
    public:
        HudScreen();

        void SetAmmo(const AmmoHudState& state);
        void SetVitals(const VitalsHudState& state);
        void SetWaves(const WaveHudState& state);

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

        const XYZEngine::UiLabel& GetWaveLabel() const;
        const XYZEngine::UiLabel& GetWaveCountLabel() const;
        const XYZEngine::UiProgressBar& GetWaveBar() const;
        bool IsWavePanelShown() const;

    private:
        XYZEngine::UiLabel* nameLabel = nullptr;
        XYZEngine::UiLabel* ammoLabel = nullptr;
        XYZEngine::UiProgressBar* healthBar = nullptr;
        XYZEngine::UiProgressBar* staminaBar = nullptr;
        XYZEngine::UiProgressBar* armorBar = nullptr;
        XYZEngine::UiLabel* noticeLabel = nullptr;
        XYZEngine::UiLabel* promptLabel = nullptr;
        XYZEngine::UiWidget* wavePanel = nullptr;
        XYZEngine::UiLabel* waveLabel = nullptr;
        XYZEngine::UiLabel* waveCountLabel = nullptr;
        XYZEngine::UiProgressBar* waveBar = nullptr;
        float noticeTimeLeft = 0.f;

        std::string shownName;
        std::string shownAmmo;
        std::string shownWave;
        std::string shownWaveCount;
    };
}
