#pragma once

#include <string>
#include <vector>
#include <UiIcon.h>
#include <UiLabel.h>
#include <UiPanel.h>
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

    /**
    *	Один слот оружия на экране. Иконку выбирает тот, кто знает про каталог,
    *	а цифру - тот, кто знает про привязки: экрану остаётся нарисовать.
    */
    // Запас неизвестен: у объекта нет подсумка.
    constexpr int NO_RESERVE = -1;

    struct WeaponSlotHudState
    {
        bool hasWeapon = false;
        bool isCurrent = false;
        int key = 0;
        const sf::Texture* icon = nullptr;

        // Число ячейки: у ствола патроны, у расходника - стопка.
        bool hasCount = false;
        int count = 0;
        int reserve = NO_RESERVE;
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

    struct ChaseHudState
    {
        float progress = 0.f;
        bool isRunning = false;
        bool isClose = false;
    };

    class HudScreen : public XYZEngine::UiScreen
    {
    public:
        HudScreen();

        void SetWeaponSlots(const std::vector<WeaponSlotHudState>& slots);
        void SetVitals(const VitalsHudState& state);
        void SetWaves(const WaveHudState& state);
        void SetChase(const ChaseHudState& state);

        void ShowNotice(const std::string& text);
        void SetPrompt(const std::string& text);
        const XYZEngine::UiLabel& GetPromptLabel() const;
        void Update(float deltaTime) override;
        bool IsNoticeShown() const;

        int GetWeaponSlotsShown() const;
        const XYZEngine::UiPanel& GetWeaponSlotPanel(int index) const;
        const XYZEngine::UiIcon& GetWeaponSlotIcon(int index) const;
        const XYZEngine::UiLabel& GetWeaponSlotKey(int index) const;
        const XYZEngine::UiLabel& GetWeaponSlotCount(int index) const;
        const XYZEngine::UiProgressBar& GetHealthBar() const;
        const XYZEngine::UiProgressBar& GetStaminaBar() const;
        const XYZEngine::UiProgressBar& GetArmorBar() const;

        const XYZEngine::UiLabel& GetWaveLabel() const;
        const XYZEngine::UiLabel& GetWaveCountLabel() const;
        const XYZEngine::UiProgressBar& GetWaveBar() const;
        bool IsWavePanelShown() const;

        const XYZEngine::UiLabel& GetChaseLabel() const;
        const XYZEngine::UiProgressBar& GetChaseBar() const;
        bool IsChasePanelShown() const;

    private:
        struct WeaponCell
        {
            XYZEngine::UiPanel* panel = nullptr;
            XYZEngine::UiIcon* icon = nullptr;
            XYZEngine::UiLabel* key = nullptr;
            XYZEngine::UiLabel* count = nullptr;
        };

        std::vector<WeaponCell> weaponCells;
        XYZEngine::UiProgressBar* healthBar = nullptr;
        XYZEngine::UiProgressBar* staminaBar = nullptr;
        XYZEngine::UiProgressBar* armorBar = nullptr;
        XYZEngine::UiLabel* noticeLabel = nullptr;
        XYZEngine::UiLabel* promptLabel = nullptr;
        XYZEngine::UiWidget* wavePanel = nullptr;
        XYZEngine::UiWidget* chasePanel = nullptr;
        XYZEngine::UiLabel* chaseLabel = nullptr;
        XYZEngine::UiProgressBar* chaseBar = nullptr;
        XYZEngine::UiLabel* waveLabel = nullptr;
        XYZEngine::UiLabel* waveCountLabel = nullptr;
        XYZEngine::UiProgressBar* waveBar = nullptr;
        float noticeTimeLeft = 0.f;

        void BuildWeaponRow(const sf::Font* font);


        std::string shownWave;
        std::string shownWaveCount;
        std::string shownChase;
    };
}
