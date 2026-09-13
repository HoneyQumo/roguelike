#include "Prop.h"
#include "DestructibleComponent.h"
#include "Fx.h"
#include "GameSettings.h"
#include "HealthComponent.h"
#include "LootDrop.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <RectangleRendererComponent.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateProp(const PropDefinition& definition, const XYZEngine::Vector2Df& position)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(PROP_OBJECT_PREFIX + definition.id);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::RectangleRendererComponent>();
        renderer->SetSize(definition.size, definition.size);
        renderer->SetColor(definition.color);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(definition.size, definition.size);

        if (!definition.IsDestructible())
        {
            return gameObject;
        }

        auto health = gameObject->AddComponent<HealthComponent>();
        health->SetMaxHealth(definition.health);

        auto destructible = gameObject->AddComponent<DestructibleComponent>();
        destructible->SetBrokenColor(definition.brokenColor);

        std::string lootTable = definition.lootTable;
        destructible->SubscribeBroken([gameObject, lootTable](const XYZEngine::Vector2Df& place)
        {
            Fx::SpawnImpact(place, {0.f, 1.f});
            DropLoot(lootTable, gameObject, place);
        });

        LOG_INFO("Prop " + definition.id + " created at " + std::to_string(static_cast<int>(position.x)) + ";"
            + std::to_string(static_cast<int>(position.y)));

        return gameObject;
    }
}
