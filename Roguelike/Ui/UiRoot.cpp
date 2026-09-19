#include "UiRoot.h"
#include "GameSettings.h"
#include "PlayerHudBinderComponent.h"
#include <GameWorld.h>
#include <LoggerRegistry.h>
#include <UiManager.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateUiRoot(HudScreen& hud, SubtitleScreen& subtitles, InventoryScreen& inventory,
                                        MessageScreen& message, FadeScreen& fade)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(UI_ROOT_OBJECT_NAME);
        gameObject->SetRenderLayer(UI_RENDER_LAYER);

        XYZEngine::UiManager::Instance()->Push(&hud);
        // Субтитры ниже сумки: проброса событий вниз нет, и они не должны
        // перехватывать то, что предназначено открытому окну.
        XYZEngine::UiManager::Instance()->Push(&subtitles);
        XYZEngine::UiManager::Instance()->Push(&inventory);
        XYZEngine::UiManager::Instance()->Push(&message);
        XYZEngine::UiManager::Instance()->Push(&fade);

        auto binder = gameObject->AddComponent<PlayerHudBinderComponent>();
        binder->SetScreen(&hud);
        binder->SetInventoryScreen(&inventory);
        binder->SetTargetName(PLAYER_OBJECT_NAME);

        LOG_INFO("Ui root created");
        return gameObject;
    }
}
