#include "Door.h"
#include "DoorComponent.h"
#include "DoorHinge.h"
#include "DoorRules.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "PropVisualComponent.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <RectangleRendererComponent.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    namespace
    {
        std::string KeyNameFor(const std::string& doorId, const ItemCatalog& items)
        {
            const ItemDefinition* key = FindKeyIn(items, doorId);

            return key != nullptr ? key->name : std::string();
        }

        XYZEngine::GameObject* CreateLeaf(XYZEngine::GameObject* door, const sf::Texture& closed)
        {
            auto leaf = XYZEngine::GameWorld::Instance()->CreateGameObject(DOOR_LEAF_OBJECT_NAME, door);
            leaf->SetRenderLayer(ITEM_RENDER_LAYER);

            auto renderer = leaf->AddComponent<XYZEngine::SpriteRendererComponent>();
            renderer->SetTexture(closed);
            renderer->SetPixelSize(static_cast<int>(TILE_SIZE), static_cast<int>(TILE_SIZE));
            renderer->SetPivot(0.5f, 1.f);

            auto visual = leaf->AddComponent<PropVisualComponent>();
            visual->SetSize(TILE_SIZE);
            visual->SetSpentColor(DOOR_OPEN_COLOR);
            visual->SetSpentTexture(XYZEngine::ResourceSystem::Instance()->GetTextureShared(DOOR_OPEN_TEXTURE));

            return leaf;
        }

        PropVisualComponent* CreatePlainLook(XYZEngine::GameObject* door)
        {
            auto renderer = door->AddComponent<XYZEngine::RectangleRendererComponent>();
            renderer->SetSize(TILE_SIZE, TILE_SIZE);
            renderer->SetColor(DOOR_LOCKED_COLOR);

            auto visual = door->AddComponent<PropVisualComponent>();
            visual->SetSize(TILE_SIZE);
            visual->SetSpentColor(DOOR_OPEN_COLOR);
            visual->SetSpentLayer(GROUND_RENDER_LAYER);

            return visual;
        }
    }

    XYZEngine::GameObject* CreateDoor(const std::string& doorId, const XYZEngine::Vector2Df& position,
        const ItemCatalog& items)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(DOOR_OBJECT_PREFIX + doorId);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(TILE_SIZE, TILE_SIZE);

        auto door = gameObject->AddComponent<DoorComponent>();
        door->SetDoorId(doorId);
        door->SetKeyName(KeyNameFor(doorId, items));

        int column = 0;
        int row = 0;
        LevelGrid::Current().ToCell(position, column, row);
        door->SetHinge(HingeFor(LevelGrid::Current(), column, row));

        auto reach = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        reach->SetSize(TILE_SIZE + DOOR_REACH_MARGIN, TILE_SIZE + DOOR_REACH_MARGIN);
        reach->SetTrigger(true);
        reach->SetCollisionLayer(ITEM_COLLISION_LAYER);
        door->SetReach(reach);

        const sf::Texture* closed = XYZEngine::ResourceSystem::Instance()->GetTextureShared(DOOR_LOCKED_TEXTURE);
        if (closed != nullptr)
        {
            XYZEngine::GameObject* leaf = CreateLeaf(gameObject, *closed);
            door->SetLeaf(leaf->GetTransform());
            door->SetVisual(leaf->GetComponent<PropVisualComponent>());
        }
        else
        {
            door->SetVisual(CreatePlainLook(gameObject));
        }

        LOG_INFO("Door " + doorId + " created at " + std::to_string(static_cast<int>(position.x)) + ";"
            + std::to_string(static_cast<int>(position.y)));

        return gameObject;
    }
}
