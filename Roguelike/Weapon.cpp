#include "Weapon.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <LoggerRegistry.h>
#include <stdexcept>

namespace RoguelikeGame
{
    Weapon::Weapon(XYZEngine::GameObject* owner, WeaponId id, XYZEngine::SpriteMovementAnimationComponent* ownerAnimation)
    {
        if (owner == nullptr)
        {
            throw std::runtime_error("weapon needs an owner");
        }

        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(WEAPONS_TEXTURE,
                                                                                         WeaponFrameIndex(id, WEAPON_DEFAULT_VARIANT));
        if (texture == nullptr)
        {
            throw std::runtime_error(std::string("weapon texture is not loaded: ") + GetWeapon(id).id);
        }

        gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Weapon");

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetParent(owner->GetComponent<XYZEngine::TransformComponent>());
        transform->SetLocalPosition(0.f, 0.f);

        renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(WEAPON_FRAME_WIDTH, WEAPON_FRAME_HEIGHT);

        layer = gameObject->AddComponent<WeaponLayerComponent>();
        layer->SetOwnerAnimation(ownerAnimation);

        CreateMuzzleFlash();
        SetWeaponId(id);

        LOG_INFO(std::string("Weapon ") + GetWeapon(id).id + " created for " + owner->GetName());
    }

    void Weapon::SetWeaponId(WeaponId id)
    {
        const WeaponDefinition& weapon = GetWeapon(id);

        layer->SetRecoil(weapon.recoil);
        layer->SetWeaponId(id);

        if (muzzleFlashTransform == nullptr)
        {
            return;
        }

        muzzleFlashTransform->SetLocalPosition(ToWorldOffset(weapon.muzzleX, weapon.muzzleY));

        hasMuzzleFlash = weapon.flashScale > 0.f;
        muzzleFlashTransform->SetLocalScale(hasMuzzleFlash ? weapon.flashScale : 1.f, hasMuzzleFlash ? weapon.flashScale : 1.f);

        if (!hasMuzzleFlash)
        {
            muzzleFlash->Stop();
            muzzleFlashRenderer->SetVisible(false);
        }
    }

    void Weapon::PlayMuzzleFlash()
    {
        if (muzzleFlash != nullptr && hasMuzzleFlash)
        {
            muzzleFlash->Play();
        }
    }

    XYZEngine::GameObject* Weapon::GetGameObject()
    {
        return gameObject;
    }

    XYZEngine::SpriteRendererComponent* Weapon::GetRenderer()
    {
        return renderer;
    }

    XYZEngine::SpriteAnimationComponent* Weapon::GetMuzzleFlash()
    {
        return muzzleFlash;
    }

    void Weapon::CreateMuzzleFlash()
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(MUZZLE_FLASH_TEXTURE, 0);
        if (texture == nullptr)
        {
            LOG_ERROR("Muzzle flash texture is not loaded");
            return;
        }

        auto flashObject = XYZEngine::GameWorld::Instance()->CreateGameObject("MuzzleFlash");

        muzzleFlashTransform = flashObject->GetComponent<XYZEngine::TransformComponent>();
        muzzleFlashTransform->SetParent(gameObject->GetComponent<XYZEngine::TransformComponent>());

        muzzleFlashRenderer = flashObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        muzzleFlashRenderer->SetTexture(*texture);
        muzzleFlashRenderer->SetPixelSize(FX_MUZZLE_FLASH.width, FX_MUZZLE_FLASH.height);
        muzzleFlashRenderer->SetPivot(FX_MUZZLE_FLASH.pivotX / FX_MUZZLE_FLASH.width, FX_MUZZLE_FLASH.pivotY / FX_MUZZLE_FLASH.height);
        muzzleFlashRenderer->SetVisible(false);

        muzzleFlash = flashObject->AddComponent<XYZEngine::SpriteAnimationComponent>();
        muzzleFlash->SetFrames(MUZZLE_FLASH_TEXTURE, 0, FX_MUZZLE_FLASH.frames, FramesPerSecond(FX_MUZZLE_FLASH.millisecondsPerFrame));
        muzzleFlash->SetEndBehaviour(XYZEngine::SpriteAnimationEnd::Hide);
    }
}
