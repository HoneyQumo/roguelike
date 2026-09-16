#pragma once

#include <GameObject.h>
#include <Vector.h>

namespace RoguelikeGame
{
    // Камера живёт отдельно от игрока, иначе её нельзя увести на время сцены.
    XYZEngine::GameObject* CreateCamera(XYZEngine::GameObject* follow);

    XYZEngine::GameObject* CreatePlayer(const XYZEngine::Vector2Df& position);
}
