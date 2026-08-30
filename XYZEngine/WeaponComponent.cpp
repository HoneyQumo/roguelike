#include "pch.h"
#include "WeaponComponent.h"
#include "GameObject.h"
#include "LoggerRegistry.h"
#include <algorithm>
#include <cassert>

namespace XYZEngine
{
    constexpr float MIN_AIM_CORRECTION_DISTANCE = 64.f;

    WeaponComponent::WeaponComponent(GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetComponent<TransformComponent>();
        pouch = gameObject->GetComponent<AmmoPouchComponent>();
    }

    void WeaponComponent::Update(float deltaTime)
    {
        if (!isPouchSearched)
        {
            if (pouch == nullptr)
            {
                pouch = gameObject->GetComponent<AmmoPouchComponent>();
            }
            isPouchSearched = true;
        }

        if (cooldownTimer > 0.f)
        {
            cooldownTimer -= deltaTime;
        }

        if (isReloading)
        {
            reloadTimer -= deltaTime;
            if (reloadTimer <= 0.f)
            {
                FinishReload();
            }
        }
    }

    void WeaponComponent::Render()
    {
    }

    void WeaponComponent::SetCooldown(float newCooldown)
    {
        assert(newCooldown >= 0.f);
        cooldown = newCooldown;
    }

    void WeaponComponent::SetDamage(float newDamage)
    {
        assert(newDamage >= 0.f);
        damage = newDamage;
    }

    void WeaponComponent::SetProjectileSpeed(float newProjectileSpeed)
    {
        assert(newProjectileSpeed > 0.f);
        projectileSpeed = newProjectileSpeed;
    }

    void WeaponComponent::SetMuzzleOffset(const Vector2Df& newMuzzleOffset)
    {
        muzzleOffset = newMuzzleOffset;
    }

    void WeaponComponent::SetShotAction(std::function<void(const Vector2Df&, const Vector2Df&, float, float)> newShotAction)
    {
        shotAction = newShotAction;
    }

    void WeaponComponent::SetMagazine(int newMagazineSize, int newAmmoKind)
    {
        magazineSize = std::max(newMagazineSize, 0);
        ammoKind = newAmmoKind;

        ammoInMagazine = magazineSize;

        isReloading = false;
        reloadTimer = 0.f;
    }

    void WeaponComponent::SetReloadTime(float newReloadTime)
    {
        assert(newReloadTime >= 0.f);
        reloadTime = std::max(newReloadTime, 0.f);
    }

    void WeaponComponent::SetReloadStartAction(std::function<void()> newReloadStartAction)
    {
        reloadStartAction = newReloadStartAction;
    }

    void WeaponComponent::SetReloadFinishAction(std::function<void()> newReloadFinishAction)
    {
        reloadFinishAction = newReloadFinishAction;
    }

    bool WeaponComponent::HasMagazine() const
    {
        return magazineSize > 0;
    }

    int WeaponComponent::GetMagazineSize() const
    {
        return magazineSize;
    }

    int WeaponComponent::GetAmmoInMagazine() const
    {
        return HasMagazine() ? ammoInMagazine : INFINITE_AMMO;
    }

    int WeaponComponent::GetReserveAmmo() const
    {
        return pouch == nullptr ? INFINITE_AMMO : pouch->GetAmmo(ammoKind);
    }

    bool WeaponComponent::IsMagazineEmpty() const
    {
        return HasMagazine() && ammoInMagazine <= 0;
    }

    bool WeaponComponent::IsReloading() const
    {
        return isReloading;
    }

    bool WeaponComponent::CanReload() const
    {
        if (!HasMagazine() || isReloading || ammoInMagazine >= magazineSize)
        {
            return false;
        }

        return GetReserveAmmo() != 0;
    }

    bool WeaponComponent::TryReload()
    {
        if (!CanReload())
        {
            return false;
        }

        isReloading = true;
        reloadTimer = reloadTime;

        if (reloadStartAction != nullptr)
        {
            reloadStartAction();
        }

        LOG_INFO(gameObject->GetName() + " reloads");

        if (reloadTime <= 0.f)
        {
            FinishReload();
        }

        return true;
    }

    void WeaponComponent::CancelReload()
    {
        if (!isReloading)
        {
            return;
        }

        isReloading = false;
        reloadTimer = 0.f;
    }

    bool WeaponComponent::IsReady() const
    {
        return cooldownTimer <= 0.f && !isReloading && !IsMagazineEmpty();
    }

    bool WeaponComponent::TryShootAt(const Vector2Df& targetPosition)
    {
        if (isReloading)
        {
            return false;
        }

        if (IsMagazineEmpty())
        {
            TryReload();
            return false;
        }

        if (!IsReady())
        {
            return false;
        }

        if (shotAction == nullptr)
        {
            LOG_ERROR("Weapon has no shot action on " + gameObject->GetName());
            return false;
        }

        Vector2Df ownerPosition = transform->GetWorldPosition();
        Vector2Df toTarget = targetPosition - ownerPosition;
        float distance = toTarget.GetLength();
        if (distance <= 0.f)
        {
            LOG_WARN("Weapon can't shoot at its own position on " + gameObject->GetName());
            return false;
        }

        Vector2Df aimDirection = (1.f / distance) * toTarget;
        Vector2Df sideDirection = {-aimDirection.y, aimDirection.x};
        Vector2Df shotPosition = ownerPosition + muzzleOffset.x * aimDirection + muzzleOffset.y * sideDirection;

        Vector2Df shotDirection = aimDirection;
        if (distance > MIN_AIM_CORRECTION_DISTANCE)
        {
            Vector2Df fromMuzzle = targetPosition - shotPosition;
            float muzzleDistance = fromMuzzle.GetLength();
            if (muzzleDistance > 0.f)
            {
                shotDirection = (1.f / muzzleDistance) * fromMuzzle;
            }
        }

        shotAction(shotPosition, shotDirection, damage, projectileSpeed);
        cooldownTimer = cooldown;

        if (HasMagazine())
        {
            ammoInMagazine--;
        }

        LOG_INFO(gameObject->GetName() + " shoots");
        return true;
    }

    void WeaponComponent::FinishReload()
    {
        isReloading = false;
        reloadTimer = 0.f;

        int missing = magazineSize - ammoInMagazine;
        int loaded = pouch == nullptr ? missing : pouch->TakeAmmo(ammoKind, missing);
        ammoInMagazine += loaded;

        if (reloadFinishAction != nullptr)
        {
            reloadFinishAction();
        }

        LOG_INFO(gameObject->GetName() + " loaded " + std::to_string(loaded) + " rounds");
    }
}
