#include "Crosshair.h"
#include "GameSettings.h"
#include "ReloadIndicatorComponent.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <CursorFollowComponent.h>
#include <LoggerRegistry.h>
#include <stdexcept>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateCrosshair()
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(CROSSHAIR_TEXTURE);
        if (texture == nullptr)
        {
            throw std::runtime_error("crosshair texture is not loaded");
        }

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Crosshair");
        gameObject->SetRenderLayer(UI_RENDER_LAYER);

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(CROSSHAIR_SIZE, CROSSHAIR_SIZE);
        renderer->SetColor(CROSSHAIR_COLOR);
        renderer->SetAdditiveBlending(true);

        gameObject->AddComponent<XYZEngine::CursorFollowComponent>();

        auto reloadIndicator = gameObject->AddComponent<ReloadIndicatorComponent>();
        reloadIndicator->SetTargetName(PLAYER_OBJECT_NAME);

        LOG_INFO("Crosshair created");
        return gameObject;
    }
}
