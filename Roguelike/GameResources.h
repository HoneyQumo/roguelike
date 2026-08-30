#pragma once

#include <string>
#include <SFML/Audio/SoundBuffer.hpp>
#include "SpriteAtlas.h"

namespace RoguelikeGame
{
    class GameResources
    {
    public:
        static void Load();
        static const sf::SoundBuffer* GetWeaponSound(const char* key);

    private:
        static void LoadCharacterAtlas(const std::string& name);
        static void LoadWeaponSounds();
        static void LoadFxStrip(const std::string& name, const FxStrip& strip);
    };
}
