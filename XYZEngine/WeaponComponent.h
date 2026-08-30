#pragma once

#include <functional>
#include "Component.h"
#include "TransformComponent.h"
#include "AmmoPouchComponent.h"
#include "Vector.h"

namespace XYZEngine
{
    constexpr int INFINITE_AMMO = -1;

    class WeaponComponent : public Component
    {
    public:
        WeaponComponent(GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetCooldown(float newCooldown);
        void SetDamage(float newDamage);
        void SetProjectileSpeed(float newProjectileSpeed);
        void SetMuzzleOffset(const Vector2Df& newMuzzleOffset);
        void SetShotAction(std::function<void(const Vector2Df&, const Vector2Df&, float, float)> newShotAction);

        void SetMagazine(int newMagazineSize, int newAmmoKind);
        void SetReloadTime(float newReloadTime);
        void SetReloadStartAction(std::function<void()> newReloadStartAction);
        void SetReloadFinishAction(std::function<void()> newReloadFinishAction);

        bool HasMagazine() const;
        int GetMagazineSize() const;
        int GetAmmoInMagazine() const;
        // INFINITE_AMMO если объект не носит AmmoPouchComponent.
        int GetReserveAmmo() const;

        bool IsMagazineEmpty() const;
        bool IsReloading() const;
        bool CanReload() const;
        bool TryReload();
        void CancelReload();

        bool IsReady() const;
        bool TryShootAt(const Vector2Df& targetPosition);

    private:
        TransformComponent* transform;
        AmmoPouchComponent* pouch = nullptr;
        bool isPouchSearched = false;

        float cooldown = 0.5f;
        float cooldownTimer = 0.f;
        float damage = 10.f;
        float projectileSpeed = 600.f;
        Vector2Df muzzleOffset = {40.f, 0.f};

        int magazineSize = 0;
        int ammoInMagazine = 0;
        int ammoKind = 0;

        float reloadTime = 0.f;
        float reloadTimer = 0.f;
        bool isReloading = false;

        std::function<void(const Vector2Df&, const Vector2Df&, float, float)> shotAction;
        std::function<void()> reloadStartAction;
        std::function<void()> reloadFinishAction;

        void FinishReload();
    };
}
