#pragma once

#include <string>
#include <SFML/Audio/SoundBuffer.hpp>
#include "ItemCatalog.h"
#include "SpriteAtlas.h"
#include "WeaponCatalog.h"

namespace RoguelikeGame
{
    class GameResources
    {
    public:
        static void Load();
        static const sf::SoundBuffer* GetWeaponSound(const char* key);
        static const sf::SoundBuffer* GetMeleeHitSound(const MeleeDefinition& melee);
        static const ItemCatalog& GetItems();

    private:
        static void LoadCharacterAtlas(const std::string& name, int framesCount);
        static void LoadWeaponSound(const std::string& key);
        static void LoadWeaponSounds();
        static void LoadFxStrip(const std::string& name, const FxStrip& strip);
        static void LoadItems();
    };
}
