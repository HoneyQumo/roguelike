#include "Prop.h"
#include "ContainerComponent.h"
#include "DestructibleComponent.h"
#include "Fx.h"
#include "GameSettings.h"
#include "HealthComponent.h"
#include "ItemCatalog.h"
#include "LootDrop.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <RectangleRendererComponent.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    namespace
    {
        std::string KeyNameOf(const std::string& keyItem, const ItemCatalog& items)
        {
            const ItemDefinition* key = items.Find(keyItem);

            return key != nullptr ? key->name : keyItem;
        }

        void AddContainer(XYZEngine::GameObject* gameObject, const PropDefinition& definition, const ItemCatalog& items)
        {
            auto reach = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
            reach->SetSize(definition.size + CONTAINER_REACH_MARGIN, definition.size + CONTAINER_REACH_MARGIN);
            reach->SetTrigger(true);
            reach->SetCollisionLayer(ITEM_COLLISION_LAYER);

            auto container = gameObject->AddComponent<ContainerComponent>();
            container->SetTitle(definition.name);
            container->SetOpenedColor(definition.openedColor);
            container->SetKeyItem(definition.keyItem, KeyNameOf(definition.keyItem, items));
            container->SetReach(reach);

            std::string lootTable = definition.lootTable;
            container->SubscribeOpened([gameObject, lootTable](const XYZEngine::Vector2Df& place)
            {
                DropLoot(lootTable, gameObject, place);
            });
        }

        void AddDestructible(XYZEngine::GameObject* gameObject, const PropDefinition& definition)
        {
            auto health = gameObject->AddComponent<HealthComponent>();
            health->SetMaxHealth(definition.health);

            std::string hitEffect = definition.hitEffect;
            health->SubscribeDamage([hitEffect](const DamageInfo& damage)
            {
                Fx::SpawnHit(hitEffect, damage.source.position, damage.source.direction);
            });

            auto destructible = gameObject->AddComponent<DestructibleComponent>();
            destructible->SetBrokenColor(definition.brokenColor);

            std::string lootTable = definition.lootTable;
            destructible->SubscribeBroken([gameObject, lootTable](const XYZEngine::Vector2Df& place)
            {
                Fx::SpawnImpact(place, {0.f, 1.f});
                DropLoot(lootTable, gameObject, place);
            });
        }
    }

    XYZEngine::GameObject* CreateProp(const PropDefinition& definition, const XYZEngine::Vector2Df& position,
        const ItemCatalog& items)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(PROP_OBJECT_PREFIX + definition.id);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::RectangleRendererComponent>();
        renderer->SetSize(definition.size, definition.size);
        renderer->SetColor(definition.color);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(definition.size, definition.size);

        if (definition.IsOpenable())
        {
            AddContainer(gameObject, definition, items);
        }

        if (definition.IsDestructible())
        {
            AddDestructible(gameObject, definition);
        }

        LOG_INFO("Prop " + definition.id + " created at " + std::to_string(static_cast<int>(position.x)) + ";"
            + std::to_string(static_cast<int>(position.y)));

        return gameObject;
    }
}
