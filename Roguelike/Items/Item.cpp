#include "Item.h"
#include "FogVisibilityComponent.h"
#include "GameSettings.h"
#include "ItemIconLayout.h"
#include "ItemPickupComponent.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>

namespace RoguelikeGame
{
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
        sf::Vector2f iconSize = IconWorldSize(definition.icon);
        renderer->SetPixelSize(static_cast<int>(iconSize.x), static_cast<int>(iconSize.y));
        renderer->SetColor(definition.icon.tint);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(ITEM_PICKUP_SIZE, ITEM_PICKUP_SIZE);
        collider->SetTrigger(true);
        collider->SetCollisionLayer(ITEM_COLLISION_LAYER);

        gameObject->AddComponent<ItemPickupComponent>()->SetDefinition(&definition);

        HideInFog(gameObject);

        return gameObject;
    }
}
