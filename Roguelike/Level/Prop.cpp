#include "Prop.h"
#include "ContainerComponent.h"
#include "DestructibleComponent.h"
#include "ExplosiveComponent.h"
#include "FuseComponent.h"
#include "Fx.h"
#include "GameSettings.h"
#include "HealthComponent.h"
#include "ItemCatalog.h"
#include "LootDrop.h"
#include "Noise.h"
#include "PropVisualComponent.h"
#include <BoxColliderComponent.h>
#include "LevelGrid.h"
#include "PathService.h"
#include <GameWorld.h>
#include <RectangleRendererComponent.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
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

        PropVisualComponent* AddVisual(XYZEngine::GameObject* gameObject, const PropDefinition& definition)
        {
            const sf::Texture* frame = definition.HasFrame()
                ? XYZEngine::ResourceSystem::Instance()->GetTextureShared(PropTextureName(definition.id, false))
                : nullptr;

            if (frame != nullptr)
            {
                auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
                renderer->SetTexture(*frame);
                renderer->SetPixelSize(static_cast<int>(definition.size), static_cast<int>(definition.Height()));
            }
            else
            {
                auto renderer = gameObject->AddComponent<XYZEngine::RectangleRendererComponent>();
                renderer->SetSize(definition.size, definition.Height());
                renderer->SetColor(definition.color);
            }

            auto visual = gameObject->AddComponent<PropVisualComponent>();
            visual->SetSize(definition.size, definition.Height());

            if (definition.HasSpentFrame())
            {
                visual->SetSpentTexture(XYZEngine::ResourceSystem::Instance()->GetTextureShared(PropTextureName(definition.id, true)));
            }

            return visual;
        }

        void AddContainer(XYZEngine::GameObject* gameObject, const PropDefinition& definition, const ItemCatalog& items,
            PropVisualComponent* visual)
        {
            auto reach = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
            reach->SetSize(definition.size + CONTAINER_REACH_MARGIN, definition.Height() + CONTAINER_REACH_MARGIN);
            reach->SetTrigger(true);
            reach->SetCollisionLayer(ITEM_COLLISION_LAYER);

            auto container = gameObject->AddComponent<ContainerComponent>();
            container->SetTitle(definition.name);
            container->SetKeyItem(definition.keyItem, KeyNameOf(definition.keyItem, items));
            container->SetReach(reach);

            std::string lootTable = definition.lootTable;
            container->SubscribeOpened([gameObject, lootTable, visual](const XYZEngine::Vector2Df& place)
            {
                visual->ShowSpent();
                DropLoot(lootTable, gameObject, place);
            });
        }

        void AddDestructible(XYZEngine::GameObject* gameObject, const PropDefinition& definition, PropVisualComponent* visual)
        {
            auto health = gameObject->AddComponent<HealthComponent>();
            health->SetMaxHealth(definition.health);

            std::string hitEffect = definition.hitEffect;
            health->SubscribeDamage([hitEffect](const DamageInfo& damage)
            {
                Fx::SpawnHit(hitEffect, damage.source.position, damage.source.direction);
            });

            visual->SetSpentLayer(PROP_DEBRIS_RENDER_LAYER);

            auto destructible = gameObject->AddComponent<DestructibleComponent>();
            destructible->SetLeavesWreck(definition.leavesWreck);

            std::string lootTable = definition.lootTable;
            destructible->SubscribeBroken([gameObject, lootTable, visual](const XYZEngine::Vector2Df& place)
            {
                visual->ShowSpent();
                Fx::SpawnImpact(place, {0.f, 1.f});
                DropLoot(lootTable, gameObject, place);
            });
        }

        void AddBlast(XYZEngine::GameObject* gameObject, const PropDefinition& definition)
        {
            auto destructible = gameObject->GetComponent<DestructibleComponent>();
            if (destructible == nullptr)
            {
                return;
            }

            float radius = definition.blastRadius;

            auto blast = gameObject->AddComponent<ExplosiveComponent>();
            blast->SetRadius(radius);
            blast->SetCoreRadius(EXPLOSION_CORE_RADIUS);
            blast->SetCenterDamage(definition.blastDamage);
            blast->SetEdgeDamagePart(PROP_BLAST_EDGE_PART);
            blast->SetSelfDamagePart(0.f);
            blast->SetOwner(gameObject->GetId(), gameObject->GetName(), Faction::Neutral);

            blast->SubscribeExplode([radius](const XYZEngine::Vector2Df& place)
            {
                Fx::SpawnExplosion(place, radius);
                Fx::ShakeCamera(CAMERA_SHAKE_BLAST);

                Noise noise;
                noise.position = place;
                noise.radius = radius * PROP_BLAST_NOISE_SCALE;
                RaiseNoise(noise);
            });

            auto fuse = gameObject->AddComponent<FuseComponent>();
            fuse->SubscribeBurnedOut([gameObject, blast]()
            {
                blast->Explode(gameObject->GetTransform()->GetWorldPosition());
            });

            float seconds = definition.blastFuse;
            destructible->SubscribeBroken([fuse, seconds](const XYZEngine::Vector2Df&)
            {
                fuse->Light(seconds);
            });
        }
    }

    std::string PropTextureName(const std::string& propId, bool spent)
    {
        return "prop_" + propId + (spent ? "_spent" : "");
    }

    XYZEngine::GameObject* CreateProp(const PropDefinition& definition, const XYZEngine::Vector2Df& position,
        const ItemCatalog& items)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(PROP_OBJECT_PREFIX + definition.id);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        PropVisualComponent* visual = AddVisual(gameObject, definition);

        if (definition.isSolid)
        {
            auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
            collider->SetSize(definition.size, definition.Height());
        }

        if (definition.IsOpenable())
        {
            visual->SetSpentColor(definition.openedColor);
            AddContainer(gameObject, definition, items, visual);
        }

        if (definition.IsDestructible())
        {
            visual->SetSpentColor(definition.brokenColor);
            AddDestructible(gameObject, definition, visual);

            if (auto destructible = gameObject->GetComponent<DestructibleComponent>())
            {
                bool leavesWreck = definition.leavesWreck;
                destructible->SubscribeBroken([leavesWreck](const XYZEngine::Vector2Df& where)
                {
                    if (!leavesWreck)
                    {
                        LevelGrid::OpenCell(where);
                        PathService::Reset();
                    }

                    Noise noise;
                    noise.position = where;
                    noise.radius = PROP_NOISE_RADIUS;
                    RaiseNoise(noise);
                });
            }

            if (definition.IsExplosive())
            {
                AddBlast(gameObject, definition);
            }
        }

        LOG_INFO("Prop " + definition.id + " created at " + std::to_string(static_cast<int>(position.x)) + ";"
            + std::to_string(static_cast<int>(position.y)));

        return gameObject;
    }
}
