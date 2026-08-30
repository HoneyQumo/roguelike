#include "AmmoHudComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <RenderSystem.h>
#include <ResourceSystem.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    AmmoHudComponent::AmmoHudComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        auto font = XYZEngine::ResourceSystem::Instance()->GetFont(HUD_FONT);
        if (font == nullptr)
        {
            LOG_ERROR("Ammo hud has no font");
            return;
        }

        ammoText.setFont(*font);
        ammoText.setCharacterSize(AMMO_HUD_FONT_SIZE);
        ammoText.setOutlineColor(AMMO_HUD_OUTLINE_COLOR);
        ammoText.setOutlineThickness(AMMO_HUD_OUTLINE);

        statusText.setFont(*font);
        statusText.setCharacterSize(AMMO_HUD_STATUS_FONT_SIZE);
        statusText.setOutlineColor(AMMO_HUD_OUTLINE_COLOR);
        statusText.setOutlineThickness(AMMO_HUD_OUTLINE);

        isFontReady = true;
    }

    void AmmoHudComponent::Update(float deltaTime)
    {
        if (weapon == nullptr)
        {
            FindWeapon();
        }
    }

    void AmmoHudComponent::Render()
    {
        if (!isFontReady || weapon == nullptr || !weapon->HasMagazine())
        {
            return;
        }

        auto& window = XYZEngine::RenderSystem::Instance()->GetMainWindow();
        sf::View worldView = window.getView();
        window.setView(window.getDefaultView());

        sf::Vector2f screenSize = window.getDefaultView().getSize();
        float statusY = screenSize.y - AMMO_HUD_MARGIN_Y - AMMO_HUD_STATUS_FONT_SIZE * AMMO_HUD_LINE_HEIGHT;
        float ammoY = statusY - AMMO_HUD_FONT_SIZE * AMMO_HUD_LINE_HEIGHT;

        ammoText.setString(GetAmmoLine());
        ammoText.setFillColor(GetAmmoColor());
        ammoText.setPosition(AMMO_HUD_MARGIN_X, ammoY);

        statusText.setString(GetStatusLine());
        statusText.setFillColor(weapon->IsReloading() ? AMMO_HUD_RELOADING_COLOR : AMMO_HUD_COLOR);
        statusText.setPosition(AMMO_HUD_MARGIN_X, statusY);

        XYZEngine::RenderSystem::Instance()->Render(ammoText);
        XYZEngine::RenderSystem::Instance()->Render(statusText);

        window.setView(worldView);
    }

    void AmmoHudComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
        weapon = nullptr;
    }

    void AmmoHudComponent::FindWeapon()
    {
        if (targetName.empty())
        {
            return;
        }

        XYZEngine::GameObject* target = XYZEngine::GameWorld::Instance()->FindGameObject(targetName);
        if (target == nullptr)
        {
            return;
        }

        weapon = target->GetComponent<XYZEngine::WeaponComponent>();
    }

    std::string AmmoHudComponent::GetAmmoLine() const
    {
        int reserve = weapon->GetReserveAmmo();
        std::string reserveText = reserve == XYZEngine::INFINITE_AMMO ? "--" : std::to_string(reserve);

        return std::to_string(weapon->GetAmmoInMagazine()) + " / " + reserveText;
    }

    std::string AmmoHudComponent::GetStatusLine() const
    {
        if (weapon->IsReloading())
        {
            return "RELOADING";
        }

        if (weapon->CanReload())
        {
            return "R - RELOAD";
        }

        if (weapon->IsMagazineEmpty())
        {
            return "NO AMMO";
        }

        return "";
    }

    sf::Color AmmoHudComponent::GetAmmoColor() const
    {
        if (weapon->IsReloading())
        {
            return AMMO_HUD_RELOADING_COLOR;
        }

        bool isLow = weapon->GetAmmoInMagazine() <= static_cast<int>(weapon->GetMagazineSize() * AMMO_HUD_LOW_PART);
        return isLow ? AMMO_HUD_LOW_COLOR : AMMO_HUD_COLOR;
    }
}
