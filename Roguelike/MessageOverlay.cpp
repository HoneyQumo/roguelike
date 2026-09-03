#include "MessageOverlay.h"
#include "MessageOverlayComponent.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateMessageOverlay()
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("MessageOverlay");
        gameObject->SetRenderLayer(UI_RENDER_LAYER);
        gameObject->AddComponent<MessageOverlayComponent>();

        LOG_INFO("Message overlay created");
        return gameObject;
    }
}
