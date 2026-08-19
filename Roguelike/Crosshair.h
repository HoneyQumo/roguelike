#pragma once

#include <GameObject.h>

namespace RoguelikeGame
{
    class Crosshair
    {
    public:
        Crosshair();
        XYZEngine::GameObject* GetGameObject();

    private:
        XYZEngine::GameObject* gameObject;
    };
}
