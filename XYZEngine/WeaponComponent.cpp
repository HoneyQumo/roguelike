#include "pch.h"
#include "WeaponComponent.h"
#include "MathUtils.h"
#include "GameObject.h"
#include "LoggerRegistry.h"
#include "randomizer.h"
#include <algorithm>
#include <cassert>
#include <cmath>

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

        shotCooldown.Tick(deltaTime);

        if (isReloading)
        {
            reload.Tick(deltaTime);
            if (reload.IsReady())
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
        shotCooldown.SetDuration(newCooldown);
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

    void WeaponComponent::SetPellets(int newPellets)
    {
        assert(newPellets >= 1);
        pellets = std::max(newPellets, 1);
    }

    void WeaponComponent::SetConeDegrees(float newConeDegrees)
    {
        assert(newConeDegrees >= 0.f);
        coneDegrees = std::max(newConeDegrees, 0.f);
    }

    void WeaponComponent::SetShotStartAction(std::function<void()> newShotStartAction)
    {
        shotStartAction = newShotStartAction;
    }

    void WeaponComponent::SetShotAction(std::function<void(const Vector2Df&, const Vector2Df&, float, float)> newShotAction)
    {
        shotAction = newShotAction;
    }

    int WeaponComponent::GetPellets() const
    {
        return pellets;
    }

    float WeaponComponent::GetConeDegrees() const
    {
        return coneDegrees;
    }

    void WeaponComponent::SetMagazine(int newMagazineSize, int newAmmoKind)
    {
        magazineSize = std::max(newMagazineSize, 0);
        ammoKind = newAmmoKind;

        ammoInMagazine = magazineSize;

        isReloading = false;
        reload.Stop();
    }

    void WeaponComponent::SetAmmoInMagazine(int newAmmoInMagazine)
    {
        ammoInMagazine = std::min(std::max(newAmmoInMagazine, 0), magazineSize);
    }

    void WeaponComponent::SetReloadTime(float newReloadTime)
    {
        assert(newReloadTime >= 0.f);
        reload.SetDuration(newReloadTime);
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

    float WeaponComponent::GetReloadProgress() const
    {
        if (!isReloading)
        {
            return 0.f;
        }

        return reload.GetProgress();
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
        reload.Restart();

        if (reloadStartAction != nullptr)
        {
            reloadStartAction();
        }

        LOG_INFO(gameObject->GetName() + " reloads");

        if (reload.GetDuration() <= 0.f)
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
        reload.Stop();
    }

    bool WeaponComponent::IsReady() const
    {
        return shotCooldown.IsReady() && !isReloading && !IsMagazineEmpty();
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

        if (shotStartAction != nullptr)
        {
            shotStartAction();
        }

        for (int pellet = 0; pellet < pellets; pellet++)
        {
            shotAction(shotPosition, RotateDirection(shotDirection, PelletAngle(pellet)), damage, projectileSpeed);
        }

        shotCooldown.Restart();

        if (HasMagazine())
        {
            ammoInMagazine--;
        }

        LOG_INFO(gameObject->GetName() + " shoots");
        return true;
    }

    float WeaponComponent::PelletAngle(int index) const
    {
        if (coneDegrees <= 0.f)
        {
            return 0.f;
        }

        float step = coneDegrees / pellets;
        float slotStart = index * step - 0.5f * coneDegrees;

        return random<float>(slotStart, slotStart + step);
    }

    Vector2Df WeaponComponent::RotateDirection(const Vector2Df& direction, float degrees)
    {
        return RotateByDegrees(direction, degrees);
    }

    void WeaponComponent::FinishReload()
    {
        isReloading = false;
        reload.Stop();

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
