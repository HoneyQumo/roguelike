#pragma once

#include <Vector.h>

namespace XYZEngine
{
    class GameObject;
}

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateFire(const XYZEngine::Vector2Df& position, float seconds);
    XYZEngine::GameObject* CreateEmber(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to, float seconds);

    // Раскидывает вокруг точки горящие очаги: каждый летит из эпицентра на своё место.
    int ScatterEmbers(const XYZEngine::Vector2Df& center, float radius, int count);
}
