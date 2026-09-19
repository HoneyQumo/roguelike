#pragma once

#include <string>
#include <SFML/Audio/SoundBuffer.hpp>
#include "ItemCatalog.h"
#include "LevelCatalog.h"
#include "LootCatalog.h"
#include "SpeechCatalog.h"
#include "PropCatalog.h"
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
        static const sf::SoundBuffer* GetStep(const char* set, int variant);
        static void LoadFixtures();
        static const ItemCatalog& GetItems();
        static const LevelCatalog& GetLevels();
        static const LootCatalog& GetLoot();
        static const SpeechCatalog& GetSpeech();
        static const PropCatalog& GetProps();

    private:
        static void LoadCharacterAtlas(const std::string& name, int framesCount, int frameSize = CHARACTER_FRAME_SIZE);
        static void LoadBossFxStrip(const std::string& name, const FxStrip& strip);
        static void LoadWeaponSound(const std::string& key);
        static void LoadSteps();
        static void LoadWeaponSounds();
        static void LoadVoiceLines();
        static void LoadFxStrip(const std::string& name, const FxStrip& strip);
        static void LoadItems();
        static void LoadLevels();
        static void LoadLoot();
        static void LoadProps();
    };
}
