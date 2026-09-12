#include "HudScreen.h"
#include "GameSettings.h"
#include "WeaponComponent.h"
#include <ResourceSystem.h>
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
        vitals->SetSize({VITALS_HUD_WIDTH, VITALS_HUD_HEALTH_HEIGHT + VITALS_HUD_GAP + VITALS_HUD_STAMINA_HEIGHT});

        healthBar = vitals->AddChild<XYZEngine::UiProgressBar>();
        healthBar->SetAnchor(XYZEngine::UiAnchor::TopLeft);
        healthBar->SetPivot(XYZEngine::UiAnchor::TopLeft);
        healthBar->SetSize({VITALS_HUD_WIDTH, VITALS_HUD_HEALTH_HEIGHT});
        healthBar->SetColors(VITALS_HUD_HEALTH_COLOR, VITALS_HUD_BACK_COLOR);

        staminaBar = vitals->AddChild<XYZEngine::UiProgressBar>();
        staminaBar->SetAnchor(XYZEngine::UiAnchor::BottomLeft);
        staminaBar->SetPivot(XYZEngine::UiAnchor::BottomLeft);
        staminaBar->SetSize({VITALS_HUD_WIDTH, VITALS_HUD_STAMINA_HEIGHT});
        staminaBar->SetColors(VITALS_HUD_STAMINA_COLOR, VITALS_HUD_BACK_COLOR);
    }

    void HudScreen::SetVitals(const VitalsHudState& state)
    {
        healthBar->SetValue(state.healthPart);
        staminaBar->SetValue(state.staminaPart);

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
