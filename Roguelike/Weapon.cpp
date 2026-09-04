#include "Weapon.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <LoggerRegistry.h>
#include <stdexcept>

namespace RoguelikeGame
{
    XYZEngine::SpriteRendererComponent* AddWeaponSprite(XYZEngine::GameObject* gameObject, WeaponId id, int variant)
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(WEAPONS_TEXTURE, WeaponFrameIndex(id, variant));
        if (texture == nullptr)
        {
            LOG_ERROR(std::string("Weapon texture is not loaded: ") + GetWeapon(id).id);
            return nullptr;
        }

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(WEAPON_FRAME_WIDTH, WEAPON_FRAME_HEIGHT);

        return renderer;
    }

    WeaponLayerComponent* CreateWeapon(XYZEngine::GameObject* owner, WeaponId id, XYZEngine::SpriteMovementAnimationComponent* ownerAnimation)
    {
        if (owner == nullptr)
        {
            throw std::runtime_error("weapon needs an owner");
        }

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Weapon", owner);

        if (AddWeaponSprite(gameObject, id, WEAPON_DEFAULT_VARIANT) == nullptr)
        {
            XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
            throw std::runtime_error(std::string("weapon texture is not loaded: ") + GetWeapon(id).id);
        }

        auto layer = gameObject->AddComponent<WeaponLayerComponent>();
        layer->SetOwnerAnimation(ownerAnimation);
        layer->SetWeaponId(id);

        LOG_INFO(std::string("Weapon ") + GetWeapon(id).id + " created for " + owner->GetName());
        return layer;
    }
}
