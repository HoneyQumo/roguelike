#pragma once

#include <functional>
#include <Component.h>
#include <EventList.h>
#include "WeaponCatalog.h"
#include <TransformComponent.h>
#include "AmmoPouchComponent.h"
#include <Vector.h>
#include <Cooldown.h>

namespace RoguelikeGame
{
    constexpr int INFINITE_AMMO = -1;

    class WeaponComponent : public XYZEngine::Component
    {
    public:
        WeaponComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetCooldown(float newCooldown);
        void SetDamage(float newDamage);
        void SetProjectileSpeed(float newProjectileSpeed);
        void SetMuzzleOffset(const XYZEngine::Vector2Df& newMuzzleOffset);
        void SetPellets(int newPellets);
        void SetConeDegrees(float newConeDegrees);
        void SetWeaponId(WeaponId newWeaponId);
        WeaponId GetWeaponId() const;

        XYZEngine::SubscriptionId SubscribeShotStart(std::function<void()> onShotStart);
        XYZEngine::SubscriptionId SubscribeShot(std::function<void(const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&, float, float)> onShot);

        int GetPellets() const;
        float GetConeDegrees() const;

        void SetMagazine(int newMagazineSize, int newAmmoKind);
        void SetAmmoInMagazine(int newAmmoInMagazine);
        void SetReloadTime(float newReloadTime);
        XYZEngine::SubscriptionId SubscribeReloadStart(std::function<void()> onReloadStart);
        XYZEngine::SubscriptionId SubscribeReloadFinish(std::function<void()> onReloadFinish);

        bool HasMagazine() const;
        int GetMagazineSize() const;
        int GetAmmoInMagazine() const;
        // INFINITE_AMMO если объект не носит AmmoPouchComponent.
        int GetReserveAmmo() const;

        bool IsMagazineEmpty() const;
        bool IsReloading() const;
        float GetReloadProgress() const;
        bool CanReload() const;
        bool TryReload();
        void CancelReload();

        bool IsReady() const;
        bool TryShootAt(const XYZEngine::Vector2Df& targetPosition);

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        AmmoPouchComponent* pouch = nullptr;

        XYZEngine::Cooldown shotCooldown{0.5f};
        float damage = 10.f;
        float projectileSpeed = 600.f;
        XYZEngine::Vector2Df muzzleOffset = {40.f, 0.f};

        int pellets = 1;
        float coneDegrees = 0.f;

        int magazineSize = 0;
        int ammoInMagazine = 0;
        int ammoKind = 0;

        XYZEngine::Cooldown reload;
        bool isReloading = false;

        WeaponId weaponId = WeaponId::Ak47;

        XYZEngine::EventList<> shotStartEvent;
        XYZEngine::EventList<const XYZEngine::Vector2Df&, const XYZEngine::Vector2Df&, float, float> shotEvent;
        XYZEngine::EventList<> reloadStartEvent;
        XYZEngine::EventList<> reloadFinishEvent;

        float PelletAngle(int index) const;
        static XYZEngine::Vector2Df RotateDirection(const XYZEngine::Vector2Df& direction, float degrees);
        void FinishReload();
    };
}
