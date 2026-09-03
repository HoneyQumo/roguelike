#pragma once

#include <GameObject.h>

namespace RoguelikeGame
{
    class AmmoHud
    {
    public:
        AmmoHud();
        XYZEngine::GameObject* GetGameObject();

    private:
        XYZEngine::GameObject* gameObject;
    };
}
