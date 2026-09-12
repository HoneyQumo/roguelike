#include "PlayerHudBinderComponent.h"
#include "GameSettings.h"
#include "WeaponCatalog.h"
#include <GameWorld.h>

namespace RoguelikeGame
{
    PlayerHudBinderComponent::PlayerHudBinderComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void PlayerHudBinderComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
        weapon = nullptr;
        loadout = nullptr;
    }

    void PlayerHudBinderComponent::SetScreen(HudScreen* newScreen)
    {
        screen = newScreen;
    }

    void PlayerHudBinderComponent::Update(float deltaTime)
    {
        if (weapon == nullptr || loadout == nullptr)
        {
            FindTarget();
        }

        if (screen == nullptr || loadout == nullptr)
        {
            return;
        }

        screen->SetAmmo(ReadState());
    }

    void PlayerHudBinderComponent::Render()
    {
    }

    void PlayerHudBinderComponent::FindTarget()
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

        weapon = target->GetComponent<WeaponComponent>();
        loadout = target->GetComponent<PlayerLoadoutComponent>();
    }

    AmmoHudState PlayerHudBinderComponent::ReadState() const
    {
        AmmoHudState state;
        state.weaponName = GetWeapon(loadout->GetCurrentWeapon()).name;

        if (weapon == nullptr || !weapon->HasMagazine())
        {
            return state;
        }

        state.hasMagazine = true;
        state.inMagazine = weapon->GetAmmoInMagazine();
        state.reserve = weapon->GetReserveAmmo();
        state.isReloading = weapon->IsReloading();
        state.isLow = weapon->GetAmmoInMagazine() <= static_cast<int>(weapon->GetMagazineSize() * AMMO_HUD_LOW_PART);

        return state;
    }
}
