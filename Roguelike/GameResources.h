#pragma once

#include <string>
#include "SpriteAtlas.h"

namespace RoguelikeGame
{
    class GameResources
    {
    public:
        static void Load();

    private:
        static void LoadCharacterAtlas(const std::string& name);
        static void LoadFxStrip(const std::string& name, const FxStrip& strip);
    };
}
