#include "Weapon.h"
#include "GameSettings.h"
#include "WeaponLayerComponent.h"
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

        const WeaponDefinition& weapon = GetWeapon(id);

        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(WEAPONS_TEXTURE,
                                                                                         WeaponFrameIndex(id, WEAPON_DEFAULT_VARIANT));
        if (texture == nullptr)
        {
            throw std::runtime_error(std::string("weapon texture is not loaded: ") + weapon.id);
        }

        gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Weapon");

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetParent(owner->GetComponent<XYZEngine::TransformComponent>());
        transform->SetLocalPosition(0.f, 0.f);

        renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(WEAPON_FRAME_WIDTH, WEAPON_FRAME_HEIGHT);

        auto layer = gameObject->AddComponent<WeaponLayerComponent>();
        layer->SetOwnerAnimation(ownerAnimation);
        layer->SetRecoil(weapon.recoil);
        layer->SetWeaponId(id);

        if (weapon.flashScale > 0.f)
        {
            CreateMuzzleFlash(weapon);
        }

        LOG_INFO(std::string("Weapon ") + weapon.id + " created for " + owner->GetName());
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

    void Weapon::CreateMuzzleFlash(const WeaponDefinition& weapon)
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(MUZZLE_FLASH_TEXTURE, 0);
        if (texture == nullptr)
        {
            LOG_ERROR("Muzzle flash texture is not loaded");
            return;
        }

        auto flashObject = XYZEngine::GameWorld::Instance()->CreateGameObject("MuzzleFlash");

        auto transform = flashObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetParent(gameObject->GetComponent<XYZEngine::TransformComponent>());
        transform->SetLocalPosition(ToWorldOffset(weapon.muzzleX, weapon.muzzleY));
        transform->SetLocalScale(weapon.flashScale, weapon.flashScale);

        auto flashRenderer = flashObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        flashRenderer->SetTexture(*texture);
        flashRenderer->SetPixelSize(FX_MUZZLE_FLASH.width, FX_MUZZLE_FLASH.height);
        flashRenderer->SetPivot(FX_MUZZLE_FLASH.pivotX / FX_MUZZLE_FLASH.width, FX_MUZZLE_FLASH.pivotY / FX_MUZZLE_FLASH.height);
        flashRenderer->SetVisible(false);

        muzzleFlash = flashObject->AddComponent<XYZEngine::SpriteAnimationComponent>();
        muzzleFlash->SetFrames(MUZZLE_FLASH_TEXTURE, 0, FX_MUZZLE_FLASH.frames, FramesPerSecond(FX_MUZZLE_FLASH.millisecondsPerFrame));
        muzzleFlash->SetEndBehaviour(XYZEngine::SpriteAnimationEnd::Hide);
    }
}
