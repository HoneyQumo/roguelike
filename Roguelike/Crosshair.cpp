#include "Crosshair.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <SpriteRendererComponent.h>
#include <CursorFollowComponent.h>
#include <LoggerRegistry.h>
#include <stdexcept>

namespace RoguelikeGame
{
    Crosshair::Crosshair()
    {
        auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(CROSSHAIR_TEXTURE);
        if (texture == nullptr)
        {
            throw std::runtime_error("crosshair texture is not loaded");
        }

        gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Crosshair");

        auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
        renderer->SetTexture(*texture);
        renderer->SetPixelSize(CROSSHAIR_SIZE, CROSSHAIR_SIZE);
        renderer->SetColor(CROSSHAIR_COLOR);
        renderer->SetAdditiveBlending(true);

        gameObject->AddComponent<XYZEngine::CursorFollowComponent>();

        LOG_INFO("Crosshair created");
    }

    XYZEngine::GameObject* Crosshair::GetGameObject()
    {
        return gameObject;
    }
}
