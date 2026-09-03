#pragma once

#include "Scene.h"
#include "Level.h"
#include <GameObject.h>

namespace RoguelikeGame
{
    class DeveloperLevel : public XYZEngine::Scene
    {
    public:
        void Start() override;
        void Restart() override;
        void Stop() override;

    private:
        Level level;
        XYZEngine::GameObject* player = nullptr;
        XYZEngine::GameObject* music = nullptr;
        XYZEngine::GameObject* crosshair = nullptr;
        XYZEngine::GameObject* ammoHud = nullptr;
    };
}
