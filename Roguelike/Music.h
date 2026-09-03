#pragma once

#include <string>
#include <GameObject.h>

namespace RoguelikeGame
{
    XYZEngine::GameObject* CreateMusic(const std::string& musicName, float volume);
}
