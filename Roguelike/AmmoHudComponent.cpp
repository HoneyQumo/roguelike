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

        nameText.setFont(*font);
        nameText.setCharacterSize(AMMO_HUD_NAME_FONT_SIZE);
        nameText.setFillColor(AMMO_HUD_COLOR);
        nameText.setOutlineColor(AMMO_HUD_OUTLINE_COLOR);
        nameText.setOutlineThickness(AMMO_HUD_OUTLINE);

        ammoText.setFont(*font);
        ammoText.setCharacterSize(AMMO_HUD_FONT_SIZE);
        ammoText.setOutlineColor(AMMO_HUD_OUTLINE_COLOR);
        ammoText.setOutlineThickness(AMMO_HUD_OUTLINE);

        isFontReady = true;
    }

    void AmmoHudComponent::Update(float deltaTime)
    {
        if (weapon == nullptr || loadout == nullptr)
        {
            FindTarget();
        }

        if (loadout == nullptr)
        {
            return;
        }

        WeaponId currentWeapon = loadout->GetCurrentWeapon();
        if (!hasShownWeapon || currentWeapon != shownWeapon)
        {
            ShowWeaponName(currentWeapon);
        }
    }

    void AmmoHudComponent::Render()
    {
        if (!isFontReady || !hasShownWeapon)
        {
            return;
        }

        auto& window = XYZEngine::RenderSystem::Instance()->GetMainWindow();
        sf::View worldView = window.getView();
        window.setView(window.getDefaultView());

        sf::Vector2f screenSize = window.getDefaultView().getSize();
        float ammoY = screenSize.y - AMMO_HUD_MARGIN_Y - AMMO_HUD_FONT_SIZE * AMMO_HUD_LINE_HEIGHT;
        float nameY = ammoY - AMMO_HUD_NAME_FONT_SIZE * AMMO_HUD_LINE_HEIGHT;

        nameText.setPosition(AMMO_HUD_MARGIN_X, nameY);
        XYZEngine::RenderSystem::Instance()->Render(nameText);

        if (weapon != nullptr && weapon->HasMagazine())
        {
            ammoText.setString(GetAmmoLine());
            ammoText.setFillColor(GetAmmoColor());
            ammoText.setPosition(AMMO_HUD_MARGIN_X, ammoY);

            XYZEngine::RenderSystem::Instance()->Render(ammoText);
        }

        window.setView(worldView);
    }

    void AmmoHudComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
        weapon = nullptr;
        loadout = nullptr;
    }

    void AmmoHudComponent::FindTarget()
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
        loadout = target->GetComponent<PlayerLoadoutComponent>();
    }

    void AmmoHudComponent::ShowWeaponName(WeaponId weaponId)
    {
        const char* name = GetWeapon(weaponId).name;
        nameText.setString(sf::String::fromUtf8(name, name + std::char_traits<char>::length(name)));

        shownWeapon = weaponId;
        hasShownWeapon = true;
    }

    std::string AmmoHudComponent::GetAmmoLine() const
    {
        int reserve = weapon->GetReserveAmmo();
        std::string reserveText = reserve == XYZEngine::INFINITE_AMMO ? "--" : std::to_string(reserve);

        return std::to_string(weapon->GetAmmoInMagazine()) + " / " + reserveText;
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
