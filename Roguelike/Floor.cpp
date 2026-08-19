#include "Floor.h"
#include "GameSettings.h"
#include <RectangleRendererComponent.h>

namespace RoguelikeGame
{
    Floor::Floor(const XYZEngine::Vector2Df& position)
    {
        gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Floor");

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::RectangleRendererComponent>();
        renderer->SetSize(TILE_SIZE, TILE_SIZE);
        renderer->SetColor(FLOOR_COLOR);
    }

    XYZEngine::GameObject* Floor::GetGameObject()
    {
        return gameObject;
    }
}
