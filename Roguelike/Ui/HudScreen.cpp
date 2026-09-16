#include "HudScreen.h"
#include "GameSettings.h"
#include "WeaponComponent.h"
#include <ResourceSystem.h>
#include <TextUtils.h>
#include <UiWidget.h>

namespace RoguelikeGame
{
    HudScreen::HudScreen()
    {
        const sf::Font* font = XYZEngine::ResourceSystem::Instance()->GetFont(HUD_FONT);

        auto block = GetRoot().AddChild<XYZEngine::UiWidget>();
        block->SetAnchor(XYZEngine::UiAnchor::BottomLeft);
        block->SetPivot(XYZEngine::UiAnchor::BottomLeft);
        block->SetOffset({AMMO_HUD_MARGIN_X, -AMMO_HUD_MARGIN_Y});
        block->SetSize({AMMO_HUD_WIDTH, AMMO_HUD_HEIGHT});

        nameLabel = block->AddChild<XYZEngine::UiLabel>();
        nameLabel->SetAnchor(XYZEngine::UiAnchor::TopLeft);
        nameLabel->SetPivot(XYZEngine::UiAnchor::TopLeft);
        nameLabel->SetSize({AMMO_HUD_WIDTH, AMMO_HUD_NAME_HEIGHT});
        nameLabel->SetAlign(XYZEngine::UiAnchor::Left);
        nameLabel->SetCharacterSize(AMMO_HUD_NAME_FONT_SIZE);
        nameLabel->SetColor(AMMO_HUD_COLOR);
        nameLabel->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        nameLabel->SetFont(font);

        ammoLabel = block->AddChild<XYZEngine::UiLabel>();
        ammoLabel->SetAnchor(XYZEngine::UiAnchor::BottomLeft);
        ammoLabel->SetPivot(XYZEngine::UiAnchor::BottomLeft);
        ammoLabel->SetSize({AMMO_HUD_WIDTH, AMMO_HUD_AMMO_HEIGHT});
        ammoLabel->SetAlign(XYZEngine::UiAnchor::Left);
        ammoLabel->SetCharacterSize(AMMO_HUD_FONT_SIZE);
        ammoLabel->SetColor(AMMO_HUD_COLOR);
        ammoLabel->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        ammoLabel->SetFont(font);
        ammoLabel->SetVisible(false);

        auto vitals = GetRoot().AddChild<XYZEngine::UiWidget>();
        vitals->SetAnchor(XYZEngine::UiAnchor::TopLeft);
        vitals->SetPivot(XYZEngine::UiAnchor::TopLeft);
        vitals->SetOffset({VITALS_HUD_MARGIN_X, VITALS_HUD_MARGIN_Y});
        vitals->SetSize({VITALS_HUD_WIDTH, VITALS_HUD_HEALTH_HEIGHT + VITALS_HUD_ARMOR_HEIGHT + VITALS_HUD_STAMINA_HEIGHT
            + 2.f * VITALS_HUD_GAP});

        healthBar = vitals->AddChild<XYZEngine::UiProgressBar>();
        healthBar->SetAnchor(XYZEngine::UiAnchor::TopLeft);
        healthBar->SetPivot(XYZEngine::UiAnchor::TopLeft);
        healthBar->SetSize({VITALS_HUD_WIDTH, VITALS_HUD_HEALTH_HEIGHT});
        healthBar->SetColors(VITALS_HUD_HEALTH_COLOR, VITALS_HUD_BACK_COLOR);

        armorBar = vitals->AddChild<XYZEngine::UiProgressBar>();
        armorBar->SetAnchor(XYZEngine::UiAnchor::TopLeft);
        armorBar->SetPivot(XYZEngine::UiAnchor::TopLeft);
        armorBar->SetOffset({0.f, VITALS_HUD_HEALTH_HEIGHT + VITALS_HUD_GAP});
        armorBar->SetSize({VITALS_HUD_WIDTH, VITALS_HUD_ARMOR_HEIGHT});
        armorBar->SetColors(VITALS_HUD_ARMOR_COLOR, VITALS_HUD_BACK_COLOR);

        staminaBar = vitals->AddChild<XYZEngine::UiProgressBar>();
        staminaBar->SetAnchor(XYZEngine::UiAnchor::BottomLeft);
        staminaBar->SetPivot(XYZEngine::UiAnchor::BottomLeft);
        staminaBar->SetSize({VITALS_HUD_WIDTH, VITALS_HUD_STAMINA_HEIGHT});
        staminaBar->SetColors(VITALS_HUD_STAMINA_COLOR, VITALS_HUD_BACK_COLOR);

        noticeLabel = GetRoot().AddChild<XYZEngine::UiLabel>();
        noticeLabel->SetAnchor(XYZEngine::UiAnchor::Bottom);
        noticeLabel->SetPivot(XYZEngine::UiAnchor::Bottom);
        noticeLabel->SetOffset({0.f, -HUD_NOTICE_MARGIN_Y});
        noticeLabel->SetSize({OVERLAY_LINE_WIDTH, HUD_NOTICE_FONT_SIZE * AMMO_HUD_LINE_HEIGHT});
        noticeLabel->SetAlign(XYZEngine::UiAnchor::Center);
        noticeLabel->SetCharacterSize(HUD_NOTICE_FONT_SIZE);
        noticeLabel->SetColor(AMMO_HUD_LOW_COLOR);
        noticeLabel->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        noticeLabel->SetFont(font);
        noticeLabel->SetVisible(false);

        wavePanel = GetRoot().AddChild<XYZEngine::UiWidget>();
        wavePanel->SetAnchor(XYZEngine::UiAnchor::Top);
        wavePanel->SetPivot(XYZEngine::UiAnchor::Top);
        wavePanel->SetOffset({0.f, WAVE_HUD_MARGIN_Y});
        wavePanel->SetSize({WAVE_HUD_WIDTH, WAVE_HUD_HEIGHT});
        wavePanel->SetVisible(false);

        waveLabel = wavePanel->AddChild<XYZEngine::UiLabel>();
        waveLabel->SetAnchor(XYZEngine::UiAnchor::Top);
        waveLabel->SetPivot(XYZEngine::UiAnchor::Top);
        waveLabel->SetSize({WAVE_HUD_WIDTH, WAVE_HUD_TITLE_HEIGHT});
        waveLabel->SetAlign(XYZEngine::UiAnchor::Center);
        waveLabel->SetCharacterSize(WAVE_HUD_TITLE_FONT_SIZE);
        waveLabel->SetColor(WAVE_HUD_COLOR);
        waveLabel->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        waveLabel->SetFont(font);

        waveBar = wavePanel->AddChild<XYZEngine::UiProgressBar>();
        waveBar->SetAnchor(XYZEngine::UiAnchor::Top);
        waveBar->SetPivot(XYZEngine::UiAnchor::Top);
        waveBar->SetOffset({0.f, WAVE_HUD_TITLE_HEIGHT + WAVE_HUD_GAP});
        waveBar->SetSize({WAVE_HUD_WIDTH, WAVE_HUD_BAR_HEIGHT});
        waveBar->SetColors(WAVE_HUD_BAR_COLOR, VITALS_HUD_BACK_COLOR);

        waveCountLabel = wavePanel->AddChild<XYZEngine::UiLabel>();
        waveCountLabel->SetAnchor(XYZEngine::UiAnchor::Bottom);
        waveCountLabel->SetPivot(XYZEngine::UiAnchor::Bottom);
        waveCountLabel->SetSize({WAVE_HUD_WIDTH, WAVE_HUD_COUNT_HEIGHT});
        waveCountLabel->SetAlign(XYZEngine::UiAnchor::Center);
        waveCountLabel->SetCharacterSize(WAVE_HUD_COUNT_FONT_SIZE);
        waveCountLabel->SetColor(WAVE_HUD_COLOR);
        waveCountLabel->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        waveCountLabel->SetFont(font);

        chasePanel = GetRoot().AddChild<XYZEngine::UiWidget>();
        chasePanel->SetAnchor(XYZEngine::UiAnchor::Top);
        chasePanel->SetPivot(XYZEngine::UiAnchor::Top);
        chasePanel->SetOffset({0.f, WAVE_HUD_MARGIN_Y});
        chasePanel->SetSize({WAVE_HUD_WIDTH, WAVE_HUD_BAR_HEIGHT + WAVE_HUD_GAP + WAVE_HUD_COUNT_HEIGHT});
        chasePanel->SetVisible(false);

        chaseBar = chasePanel->AddChild<XYZEngine::UiProgressBar>();
        chaseBar->SetAnchor(XYZEngine::UiAnchor::Top);
        chaseBar->SetPivot(XYZEngine::UiAnchor::Top);
        chaseBar->SetSize({WAVE_HUD_WIDTH, WAVE_HUD_BAR_HEIGHT});
        chaseBar->SetColors(CHASE_HUD_AWAY_COLOR, VITALS_HUD_BACK_COLOR);

        chaseLabel = chasePanel->AddChild<XYZEngine::UiLabel>();
        chaseLabel->SetAnchor(XYZEngine::UiAnchor::Bottom);
        chaseLabel->SetPivot(XYZEngine::UiAnchor::Bottom);
        chaseLabel->SetSize({WAVE_HUD_WIDTH, WAVE_HUD_COUNT_HEIGHT});
        chaseLabel->SetAlign(XYZEngine::UiAnchor::Center);
        chaseLabel->SetCharacterSize(WAVE_HUD_COUNT_FONT_SIZE);
        chaseLabel->SetColor(WAVE_HUD_COLOR);
        chaseLabel->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        chaseLabel->SetFont(font);

        promptLabel = GetRoot().AddChild<XYZEngine::UiLabel>();
        promptLabel->SetAnchor(XYZEngine::UiAnchor::Bottom);
        promptLabel->SetPivot(XYZEngine::UiAnchor::Bottom);
        promptLabel->SetOffset({0.f, -HUD_PROMPT_MARGIN_Y});
        promptLabel->SetSize({OVERLAY_LINE_WIDTH, HUD_PROMPT_FONT_SIZE * AMMO_HUD_LINE_HEIGHT});
        promptLabel->SetAlign(XYZEngine::UiAnchor::Center);
        promptLabel->SetCharacterSize(HUD_PROMPT_FONT_SIZE);
        promptLabel->SetColor(AMMO_HUD_COLOR);
        promptLabel->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        promptLabel->SetFont(font);
        promptLabel->SetVisible(false);
    }

    void HudScreen::SetPrompt(const std::string& text)
    {
        if (text.empty())
        {
            promptLabel->SetVisible(false);
            return;
        }

        promptLabel->SetText(XYZEngine::FromUtf8(text.c_str()));
        promptLabel->SetVisible(true);
    }

    const XYZEngine::UiLabel& HudScreen::GetPromptLabel() const
    {
        return *promptLabel;
    }

    void HudScreen::ShowNotice(const std::string& text)
    {
        noticeLabel->SetUtf8Text(text.c_str());
        noticeLabel->SetVisible(true);
        noticeTimeLeft = HUD_NOTICE_TIME;
    }

    void HudScreen::Update(float deltaTime)
    {
        if (noticeTimeLeft <= 0.f)
        {
            return;
        }

        noticeTimeLeft -= deltaTime;
        if (noticeTimeLeft <= 0.f)
        {
            noticeTimeLeft = 0.f;
            noticeLabel->SetVisible(false);
        }
    }

    bool HudScreen::IsNoticeShown() const
    {
        return noticeLabel->IsVisible();
    }

    void HudScreen::SetVitals(const VitalsHudState& state)
    {
        healthBar->SetValue(state.healthPart);
        staminaBar->SetValue(state.staminaPart);
        armorBar->SetValue(state.armorPart);
        armorBar->SetVisible(state.armorPart > 0.f);

        if (state.healthPart <= VITALS_HUD_CRITICAL_PART)
        {
            healthBar->SetColors(VITALS_HUD_HEALTH_CRITICAL_COLOR, VITALS_HUD_BACK_COLOR);
        }
        else if (state.healthPart <= VITALS_HUD_LOW_PART)
        {
            healthBar->SetColors(VITALS_HUD_HEALTH_LOW_COLOR, VITALS_HUD_BACK_COLOR);
        }
        else
        {
            healthBar->SetColors(VITALS_HUD_HEALTH_COLOR, VITALS_HUD_BACK_COLOR);
        }

        staminaBar->SetColors(state.isExhausted ? VITALS_HUD_STAMINA_EMPTY_COLOR : VITALS_HUD_STAMINA_COLOR,
            VITALS_HUD_BACK_COLOR);
    }

    void HudScreen::SetWaves(const WaveHudState& state)
    {
        wavePanel->SetVisible(state.isRunning);
        if (!state.isRunning)
        {
            return;
        }

        std::string title = state.isPause
            ? std::string(WAVE_HUD_CALM)
            : std::string(WAVE_HUD_TITLE) + std::to_string(state.current) + WAVE_HUD_OF + std::to_string(state.total);

        if (shownWave != title)
        {
            shownWave = title;
            waveLabel->SetUtf8Text(title.c_str());
        }

        // Полоса меряет всю осаду: сколько волн позади из тех, что будут.
        float part = state.total > 0 ? static_cast<float>(state.current) / static_cast<float>(state.total) : 0.f;
        waveBar->SetValue(part);
        waveBar->SetColors(state.isPause ? WAVE_HUD_CALM_COLOR : WAVE_HUD_BAR_COLOR, VITALS_HUD_BACK_COLOR);

        std::string count = state.isPause
            ? std::string(WAVE_HUD_NEXT_IN) + std::to_string(static_cast<int>(state.nextIn + 1.f))
            : std::string(WAVE_HUD_LEFT) + std::to_string(state.left);

        if (shownWaveCount != count)
        {
            shownWaveCount = count;
            waveCountLabel->SetUtf8Text(count.c_str());
        }
    }

    const XYZEngine::UiLabel& HudScreen::GetWaveLabel() const
    {
        return *waveLabel;
    }

    const XYZEngine::UiLabel& HudScreen::GetWaveCountLabel() const
    {
        return *waveCountLabel;
    }

    const XYZEngine::UiProgressBar& HudScreen::GetWaveBar() const
    {
        return *waveBar;
    }

    bool HudScreen::IsWavePanelShown() const
    {
        return wavePanel->IsVisible();
    }

    /**
    *	В беге считать нечего: важно, сколько моста позади и дышат ли в спину.
    */
    void HudScreen::SetChase(const ChaseHudState& state)
    {
        chasePanel->SetVisible(state.isRunning);
        if (!state.isRunning)
        {
            return;
        }

        float part = state.progress < 0.f ? 0.f : (state.progress > 1.f ? 1.f : state.progress);
        chaseBar->SetValue(part);
        chaseBar->SetColors(state.isClose ? CHASE_HUD_CLOSE_COLOR : CHASE_HUD_AWAY_COLOR, VITALS_HUD_BACK_COLOR);

        std::string text = state.isClose ? CHASE_HUD_CLOSE : CHASE_HUD_AWAY;
        if (shownChase != text)
        {
            shownChase = text;
            chaseLabel->SetUtf8Text(text.c_str());
        }
    }

    const XYZEngine::UiLabel& HudScreen::GetChaseLabel() const
    {
        return *chaseLabel;
    }

    const XYZEngine::UiProgressBar& HudScreen::GetChaseBar() const
    {
        return *chaseBar;
    }

    bool HudScreen::IsChasePanelShown() const
    {
        return chasePanel->IsVisible();
    }

    void HudScreen::SetAmmo(const AmmoHudState& state)
    {
        if (state.weaponName != nullptr && shownName != state.weaponName)
        {
            shownName = state.weaponName;
            nameLabel->SetUtf8Text(state.weaponName);
        }

        ammoLabel->SetVisible(state.hasMagazine);
        if (!state.hasMagazine)
        {
            return;
        }

        std::string reserveText = state.reserve == INFINITE_AMMO ? "--" : std::to_string(state.reserve);
        std::string ammoLine = std::to_string(state.inMagazine) + " / " + reserveText;

        if (shownAmmo != ammoLine)
        {
            shownAmmo = ammoLine;
            ammoLabel->SetText(sf::String(ammoLine));
        }

        if (state.isReloading)
        {
            ammoLabel->SetColor(AMMO_HUD_RELOADING_COLOR);
            return;
        }

        ammoLabel->SetColor(state.isLow ? AMMO_HUD_LOW_COLOR : AMMO_HUD_COLOR);
    }

    const XYZEngine::UiProgressBar& HudScreen::GetHealthBar() const
    {
        return *healthBar;
    }

    const XYZEngine::UiProgressBar& HudScreen::GetArmorBar() const
    {
        return *armorBar;
    }

    const XYZEngine::UiProgressBar& HudScreen::GetStaminaBar() const
    {
        return *staminaBar;
    }

    const XYZEngine::UiLabel& HudScreen::GetNameLabel() const
    {
        return *nameLabel;
    }

    const XYZEngine::UiLabel& HudScreen::GetAmmoLabel() const
    {
        return *ammoLabel;
    }
}
