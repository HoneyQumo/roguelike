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

        BuildWeaponRow(font);
        BuildThreatMarks();

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

    namespace
    {
        // У ствола две цифры, у расходника одна: запас отсутствует.
        std::string CountLine(const SlotHudState& state)
        {
            if (state.reserve == NO_RESERVE)
            {
                return std::to_string(state.count);
            }

            return std::to_string(state.count) + "/" + std::to_string(state.reserve);
        }
    }

    HudScreen::WeaponCell HudScreen::BuildCell(XYZEngine::UiWidget* row, const sf::Font* font, float left)
    {
        WeaponCell cell;

        cell.panel = row->AddChild<XYZEngine::UiPanel>();
        cell.panel->SetAnchor(XYZEngine::UiAnchor::TopLeft);
        cell.panel->SetPivot(XYZEngine::UiAnchor::TopLeft);
        cell.panel->SetOffset({left, 0.f});
        cell.panel->SetSize({WEAPON_ROW_SLOT_SIZE, WEAPON_ROW_SLOT_SIZE});
        cell.panel->SetFillColor(INVENTORY_SLOT_EMPTY_COLOR);
        cell.panel->SetOutline(2.f, INVENTORY_SLOT_OUTLINE_COLOR);

        cell.icon = cell.panel->AddChild<XYZEngine::UiIcon>();
        cell.icon->SetAnchor(XYZEngine::UiAnchor::Center);
        cell.icon->SetPivot(XYZEngine::UiAnchor::Center);
        cell.icon->SetSize({WEAPON_ROW_SLOT_SIZE - WEAPON_ROW_SLOT_GAP, WEAPON_ROW_SLOT_SIZE - WEAPON_ROW_SLOT_GAP});
        cell.icon->SetKeepAspect(true);
        cell.icon->SetVisible(false);

        cell.key = cell.panel->AddChild<XYZEngine::UiLabel>();
        cell.key->SetAnchor(XYZEngine::UiAnchor::BottomLeft);
        cell.key->SetPivot(XYZEngine::UiAnchor::BottomLeft);
        cell.key->SetSize({WEAPON_ROW_KEY_WIDTH, WEAPON_ROW_KEY_HEIGHT});
        cell.key->SetAlign(XYZEngine::UiAnchor::Left);
        cell.key->SetCharacterSize(WEAPON_ROW_KEY_FONT_SIZE);
        cell.key->SetColor(AMMO_HUD_COLOR);
        cell.key->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        cell.key->SetFont(font);

        cell.count = cell.panel->AddChild<XYZEngine::UiLabel>();
        cell.count->SetAnchor(XYZEngine::UiAnchor::BottomRight);
        cell.count->SetPivot(XYZEngine::UiAnchor::BottomRight);
        cell.count->SetSize({WEAPON_ROW_COUNT_WIDTH, WEAPON_ROW_KEY_HEIGHT});
        cell.count->SetAlign(XYZEngine::UiAnchor::Right);
        cell.count->SetCharacterSize(WEAPON_ROW_COUNT_FONT_SIZE);
        cell.count->SetColor(AMMO_HUD_COLOR);
        cell.count->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        cell.count->SetFont(font);
        cell.count->SetVisible(false);

        return cell;
    }

    /**
    *	Руки и пояс - одна линия внизу экрана, но две группы: между тройками зазор,
    *	чтобы глаз читал их порознь, а не как шесть одинаковых ячеек.
    */
    void HudScreen::BuildWeaponRow(const sf::Font* font)
    {
        float step = WEAPON_ROW_SLOT_SIZE + WEAPON_ROW_SLOT_GAP;
        float weaponsWidth = PLAYER_WEAPON_SLOTS * WEAPON_ROW_SLOT_SIZE + (PLAYER_WEAPON_SLOTS - 1) * WEAPON_ROW_SLOT_GAP;
        float beltWidth = QUICK_BELT_HOOKS * WEAPON_ROW_SLOT_SIZE + (QUICK_BELT_HOOKS - 1) * WEAPON_ROW_SLOT_GAP;

        auto row = GetRoot().AddChild<XYZEngine::UiWidget>();
        row->SetAnchor(XYZEngine::UiAnchor::Bottom);
        row->SetPivot(XYZEngine::UiAnchor::Bottom);
        row->SetOffset({0.f, -WEAPON_ROW_MARGIN_Y});
        row->SetSize({weaponsWidth + BELT_ROW_GAP + beltWidth, WEAPON_ROW_SLOT_SIZE});

        for (int slot = 0; slot < PLAYER_WEAPON_SLOTS; slot++)
        {
            weaponCells.push_back(BuildCell(row, font, slot * step));
        }

        for (int hook = 0; hook < QUICK_BELT_HOOKS; hook++)
        {
            beltCells.push_back(BuildCell(row, font, weaponsWidth + BELT_ROW_GAP + hook * step));
        }
    }

    void HudScreen::SetWeaponSlots(const std::vector<SlotHudState>& slots)
    {
        FillCells(weaponCells, slots);
    }

    void HudScreen::SetBeltSlots(const std::vector<SlotHudState>& slots)
    {
        FillCells(beltCells, slots);
    }

    // Метка - прямоугольник, а не знак: в шрифте HUD нет геометрических символов,
    // и вместо ромба рисовалась пустая рамка.
    void HudScreen::BuildThreatMarks()
    {
        for (int sector = 0; sector < THREAT_MARK_SECTORS; sector++)
        {
            auto mark = GetRoot().AddChild<XYZEngine::UiPanel>();
            mark->SetAnchor(XYZEngine::UiAnchor::Center);
            mark->SetPivot(XYZEngine::UiAnchor::Center);
            mark->SetOffset(ThreatMarkOffset(sector, THREAT_MARK_SECTORS,
                SCREEN_WIDTH * 0.5f - THREAT_MARK_MARGIN_X, SCREEN_HEIGHT * 0.5f - THREAT_MARK_MARGIN_Y));
            mark->SetSize({THREAT_MARK_SIZE, THREAT_MARK_SIZE});
            mark->SetVisible(false);

            threatMarks.push_back(mark);
        }
    }

    /**
    *	Метка живёт на своём секторе всегда и только гаснет: переставлять
    *	виджеты по ходу значило бы дёргать раскладку каждый кадр.
    */
    void HudScreen::SetThreatMarks(const std::vector<ThreatMark>& marks)
    {
        for (int sector = 0; sector < static_cast<int>(threatMarks.size()); sector++)
        {
            auto found = std::find_if(marks.begin(), marks.end(),
                [sector](const ThreatMark& mark) { return mark.sector == sector; });

            if (found == marks.end())
            {
                threatMarks[sector]->SetVisible(false);
                continue;
            }

            bool isEnemy = found->kind == ThreatKind::Enemy;

            sf::Color color = isEnemy ? THREAT_ENEMY_COLOR : THREAT_NOISE_COLOR;
            color.a = static_cast<sf::Uint8>(255.f * std::clamp(found->strength, 0.f, 1.f));

            sf::Color outline = AMMO_HUD_OUTLINE_COLOR;
            outline.a = color.a;

            // Враг крупнее шума: цвет один различать их не должен.
            float size = isEnemy ? THREAT_MARK_SIZE : THREAT_MARK_SIZE * 0.6f;

            threatMarks[sector]->SetVisible(true);
            threatMarks[sector]->SetSize({size, size});
            threatMarks[sector]->SetFillColor(color);
            threatMarks[sector]->SetOutline(2.f, outline);
        }
    }

    int HudScreen::GetThreatMarksShown() const
    {
        int total = 0;

        for (const XYZEngine::UiPanel* mark : threatMarks)
        {
            total += mark->IsVisible() ? 1 : 0;
        }

        return total;
    }

    const XYZEngine::UiPanel& HudScreen::GetThreatMark(int index) const
    {
        return *threatMarks[index];
    }

    void HudScreen::FillCells(std::vector<WeaponCell>& cells, const std::vector<SlotHudState>& slots)
    {
        for (std::size_t index = 0; index < cells.size(); index++)
        {
            const WeaponCell& cell = cells[index];
            bool isShown = index < slots.size();

            cell.panel->SetVisible(isShown);
            if (!isShown)
            {
                continue;
            }

            const SlotHudState& state = slots[index];

            // Пустой слот всё равно показывает свою цифру: игрок должен видеть, куда класть.
            cell.panel->SetFillColor(state.isCurrent ? INVENTORY_SLOT_SELECTED_COLOR
                : state.isFilled ? INVENTORY_SLOT_FILLED_COLOR : INVENTORY_SLOT_EMPTY_COLOR);

            cell.icon->SetTexture(state.icon);
            cell.icon->SetVisible(state.isFilled && state.icon != nullptr);

            cell.key->SetUtf8Text(std::to_string(state.key).c_str());

            cell.count->SetVisible(state.hasCount);
            if (!state.hasCount)
            {
                continue;
            }

            cell.count->SetText(sf::String(CountLine(state)));
            cell.count->SetColor(state.isReloading ? AMMO_HUD_RELOADING_COLOR
                : state.isLow ? AMMO_HUD_LOW_COLOR : AMMO_HUD_COLOR);
        }
    }

    int HudScreen::GetWeaponSlotsShown() const
    {
        int total = 0;

        for (const WeaponCell& cell : weaponCells)
        {
            total += cell.panel->IsVisible() ? 1 : 0;
        }

        return total;
    }

    int HudScreen::GetBeltSlotsShown() const
    {
        int total = 0;

        for (const WeaponCell& cell : beltCells)
        {
            total += cell.panel->IsVisible() ? 1 : 0;
        }

        return total;
    }

    const XYZEngine::UiPanel& HudScreen::GetBeltSlotPanel(int index) const
    {
        return *beltCells[index].panel;
    }

    const XYZEngine::UiIcon& HudScreen::GetBeltSlotIcon(int index) const
    {
        return *beltCells[index].icon;
    }

    const XYZEngine::UiLabel& HudScreen::GetBeltSlotKey(int index) const
    {
        return *beltCells[index].key;
    }

    const XYZEngine::UiLabel& HudScreen::GetBeltSlotCount(int index) const
    {
        return *beltCells[index].count;
    }

    const XYZEngine::UiPanel& HudScreen::GetWeaponSlotPanel(int index) const
    {
        return *weaponCells[index].panel;
    }

    const XYZEngine::UiIcon& HudScreen::GetWeaponSlotIcon(int index) const
    {
        return *weaponCells[index].icon;
    }

    const XYZEngine::UiLabel& HudScreen::GetWeaponSlotKey(int index) const
    {
        return *weaponCells[index].key;
    }

    const XYZEngine::UiLabel& HudScreen::GetWeaponSlotCount(int index) const
    {
        return *weaponCells[index].count;
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

}
