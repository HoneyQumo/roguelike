#include "UiRoot.h"
#include "GameSettings.h"
#include "PlayerHudBinderComponent.h"
#include "ThreatMarkComponent.h"
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <UiManager.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateUiRoot(HudScreen& hud, InventoryScreen& inventory, MessageScreen& message, FadeScreen& fade)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(UI_ROOT_OBJECT_NAME);
        gameObject->SetRenderLayer(UI_RENDER_LAYER);

        XYZEngine::UiManager::Instance()->Push(&hud);
        XYZEngine::UiManager::Instance()->Push(&inventory);
        XYZEngine::UiManager::Instance()->Push(&message);
        XYZEngine::UiManager::Instance()->Push(&fade);

        // Знаки рисуются в мировых координатах, поэтому живут своим объектом, а не на HUD.
        auto markObject = XYZEngine::GameWorld::Instance()->CreateGameObject(THREAT_MARK_OBJECT_NAME);
        markObject->SetRenderLayer(THREAT_MARK_RENDER_LAYER);
        auto threatMarks = markObject->AddComponent<ThreatMarkComponent>();

        auto binder = gameObject->AddComponent<PlayerHudBinderComponent>();
        binder->SetThreatMarks(threatMarks);
        binder->SetScreen(&hud);
        binder->SetInventoryScreen(&inventory);
        binder->SetTargetName(PLAYER_OBJECT_NAME);

        LOG_INFO("Ui root created");
        return gameObject;
    }
}
