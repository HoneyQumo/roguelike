#include "Door.h"
#include "DoorComponent.h"
#include "GameSettings.h"
#include "PropVisualComponent.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <RectangleRendererComponent.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateDoor(const std::string& doorId, const XYZEngine::Vector2Df& position)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(DOOR_OBJECT_PREFIX + doorId);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::RectangleRendererComponent>();
        renderer->SetSize(TILE_SIZE, TILE_SIZE);
        renderer->SetColor(DOOR_LOCKED_COLOR);

        auto visual = gameObject->AddComponent<PropVisualComponent>();
        visual->SetSize(TILE_SIZE);
        visual->SetSpentColor(DOOR_OPEN_COLOR);
        visual->SetSpentLayer(GROUND_RENDER_LAYER);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(TILE_SIZE, TILE_SIZE);

        auto door = gameObject->AddComponent<DoorComponent>();
        door->SetDoorId(doorId);
        door->SetVisual(visual);

        LOG_INFO("Door " + doorId + " created at " + std::to_string(static_cast<int>(position.x)) + ";"
            + std::to_string(static_cast<int>(position.y)));

        return gameObject;
    }
}
