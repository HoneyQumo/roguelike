#pragma once

#include <string>
#include <GameObject.h>
#include <GameWorld.h>
#include "FogVisibilityComponent.h"

namespace RoguelikeGame
{
    // Общая фабрика эффектов: и принадлежность локации, и скрытие туманом легко забыть поодиночке.
    inline XYZEngine::GameObject* CreateEffectObject(const std::string& name, int renderLayer)
    {
        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(name);
        gameObject->SetTemporary(true);
        gameObject->SetRenderLayer(renderLayer);
        HideInFog(gameObject);

        return gameObject;
    }
}
