#include "Weapon.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <LoggerRegistry.h>
#include <stdexcept>

namespace RoguelikeGame
{
    WeaponLayerComponent* CreateWeapon(XYZEngine::GameObject* owner, WeaponId id, XYZEngine::SpriteMovementAnimationComponent* ownerAnimation)
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

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Weapon");

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetParent(owner->GetComponent<XYZEngine::TransformComponent>());
        transform->SetLocalPosition(0.f, 0.f);

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(WEAPON_FRAME_WIDTH, WEAPON_FRAME_HEIGHT);

        auto layer = gameObject->AddComponent<WeaponLayerComponent>();
        layer->SetOwnerAnimation(ownerAnimation);
        layer->SetWeaponId(id);

        LOG_INFO(std::string("Weapon ") + GetWeapon(id).id + " created for " + owner->GetName());
        return layer;
    }
}
