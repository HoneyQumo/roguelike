#include "Item.h"
#include "GameSettings.h"
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

    XYZEngine::GameObject* CreateItem(const ItemDefinition& definition, const XYZEngine::Vector2Df& position)
    {
        const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(ItemTextureName(definition.id));
        if (texture == nullptr)
        {
            LOG_ERROR("Item texture is not loaded: " + definition.id);
            return nullptr;
        }

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Item_" + definition.id);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(static_cast<int>(definition.icon.rect.width * definition.icon.worldScale),
            static_cast<int>(definition.icon.rect.height * definition.icon.worldScale));
        renderer->SetColor(definition.icon.tint);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(ITEM_PICKUP_SIZE, ITEM_PICKUP_SIZE);
        collider->SetTrigger(true);
        collider->SetCollisionLayer(ITEM_COLLISION_LAYER);

        gameObject->AddComponent<ItemPickupComponent>()->SetDefinition(&definition);

        return gameObject;
    }
}
