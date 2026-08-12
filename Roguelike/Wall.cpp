#include "Wall.h"
#include "GameSettings.h"
#include <RectangleRendererComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>

namespace RoguelikeGame
{
    Wall::Wall(const XYZEngine::Vector2Df& position)
    {
        gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Wall");

        auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
        transform->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::RectangleRendererComponent>();
        renderer->SetSize(TILE_SIZE, TILE_SIZE);
        renderer->SetColor(WALL_COLOR);

        // Kinematic body keeps the wall in place when something bumps into it.
        auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
        body->SetKinematic(true);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(TILE_SIZE, TILE_SIZE);
    }

    XYZEngine::GameObject* Wall::GetGameObject()
    {
        return gameObject;
    }
}
