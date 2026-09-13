#include "Item.h"
#include "GameSettings.h"
#include "ItemPickupComponent.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <algorithm>

namespace RoguelikeGame
{
    namespace
    {
        float IconScale(const ItemIcon& icon)
        {
            float longest = static_cast<float>(std::max(icon.rect.width, icon.rect.height));

            return longest > 0.f ? ITEM_WORLD_SIZE / longest * icon.worldScale : icon.worldScale;
        }
    }

    std::string ItemTextureName(const std::string& itemId)
    {
        return "item_" + itemId;
    }

    XYZEngine::GameObject* CreateItem(const ItemDefinition& definition, const XYZEngine::Vector2Df& position,
        XYZEngine::GameObject* parent)
    {
        const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(ItemTextureName(definition.id));
        if (texture == nullptr)
        {
            LOG_ERROR("Item texture is not loaded: " + definition.id);
            return nullptr;
        }

        auto gameObject = parent == nullptr
            ? XYZEngine::GameWorld::Instance()->CreateGameObject("Item_" + definition.id)
            : XYZEngine::GameWorld::Instance()->CreateGameObject("Item_" + definition.id, parent);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(static_cast<int>(definition.icon.rect.width * IconScale(definition.icon)),
            static_cast<int>(definition.icon.rect.height * IconScale(definition.icon)));
        renderer->SetColor(definition.icon.tint);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(ITEM_PICKUP_SIZE, ITEM_PICKUP_SIZE);
        collider->SetTrigger(true);
        collider->SetCollisionLayer(ITEM_COLLISION_LAYER);

        gameObject->AddComponent<ItemPickupComponent>()->SetDefinition(&definition);

        return gameObject;
    }
}
