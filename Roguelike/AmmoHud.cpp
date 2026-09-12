#include "AmmoHud.h"
#include "AmmoHudComponent.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateAmmoHud()
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("AmmoHud");
        gameObject->SetRenderLayer(UI_RENDER_LAYER);

        auto hud = gameObject->AddComponent<AmmoHudComponent>();
        hud->SetTargetName(PLAYER_OBJECT_NAME);

        LOG_INFO("Ammo hud created");
        return gameObject;
    }
}
