#include "LevelExit.h"
#include "FogVisibilityComponent.h"
#include "GameSettings.h"
#include "LevelExitComponent.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <RectangleRendererComponent.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateLevelExit(const XYZEngine::Vector2Df& position)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(LEVEL_EXIT_OBJECT_NAME);
        gameObject->SetRenderLayer(ITEM_RENDER_LAYER);
        gameObject->GetTransform()->SetWorldPosition(position);

        auto renderer = gameObject->AddComponent<XYZEngine::RectangleRendererComponent>();
        renderer->SetSize(TILE_SIZE, TILE_SIZE);
        renderer->SetColor(LEVEL_EXIT_COLOR);

        auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
        collider->SetSize(TILE_SIZE * 0.9f, TILE_SIZE * 0.9f);
        collider->SetTrigger(true);

        gameObject->AddComponent<LevelExitComponent>();

        HideInFog(gameObject);

        return gameObject;
    }
}
