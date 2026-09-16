#pragma once

#include <Vector.h>

namespace XYZEngine
{
    class GameObject;
}

namespace RoguelikeGame
{
    /**
    *	Во сколько раз растянуть кадр пламени, чтобы картинка легла ровно на ту
    *	зону, которую огонь жжёт: кадр квадратный, очаг круглый.
    */
    constexpr float FlameScale(float radius, int frameWidth)
    {
        return frameWidth > 0 && radius > 0.f ? 2.f * radius / static_cast<float>(frameWidth) : 1.f;
    }

    XYZEngine::GameObject* CreateFire(const XYZEngine::Vector2Df& position, float seconds);
    XYZEngine::GameObject* CreateEmber(const XYZEngine::Vector2Df& from, const XYZEngine::Vector2Df& to, float seconds);

    // Раскидывает вокруг точки горящие очаги: каждый летит из эпицентра на своё место.
    int ScatterEmbers(const XYZEngine::Vector2Df& center, float radius, int count);
}
