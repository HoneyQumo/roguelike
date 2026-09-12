#include "Wall.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateWall(const XYZEngine::Vector2Df& position)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Wall");

        auto transform = gameObject->GetTransform();
        transform->SetWorldPosition(position);

        // Kinematic body keeps the wall in place when something bumps into it.
        auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
        body->SetKinematic(true);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(TILE_SIZE, TILE_SIZE);

        return gameObject;
    }
}
