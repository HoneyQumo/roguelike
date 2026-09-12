#include "UiRoot.h"
#include "GameSettings.h"
#include "PlayerHudBinderComponent.h"
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <UiManager.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateUiRoot(HudScreen& hud, MessageScreen& message)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(UI_ROOT_OBJECT_NAME);
        gameObject->SetRenderLayer(UI_RENDER_LAYER);

        XYZEngine::UiManager::Instance()->Push(&hud);
        XYZEngine::UiManager::Instance()->Push(&message);

        auto binder = gameObject->AddComponent<PlayerHudBinderComponent>();
        binder->SetScreen(&hud);
        binder->SetTargetName(PLAYER_OBJECT_NAME);

        LOG_INFO("Ui root created");
        return gameObject;
    }
}
