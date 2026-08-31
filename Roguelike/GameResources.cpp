#include "GameResources.h"
#include "GameSettings.h"
#include "WeaponCatalog.h"
#include <ResourceSystem.h>
#include <randomizer.h>
#include <SFML/Graphics/Shader.hpp>

namespace RoguelikeGame
{
    void GameResources::Load()
    {
        XYZEngine::ResourceSystem::Instance()->LoadTexture(CROSSHAIR_TEXTURE, CROSSHAIR_FILE, false);

        LoadCharacterAtlas(PLAYER_TEXTURE, PLAYER_ATLAS_FRAMES);
        LoadCharacterAtlas(GRUNT_CONFIG.textureMapName, ENEMY_ATLAS_FRAMES);
        LoadCharacterAtlas(ASSAULT_CONFIG.textureMapName, ENEMY_ATLAS_FRAMES);
        LoadCharacterAtlas(SHIELD_CONFIG.textureMapName, ENEMY_ATLAS_FRAMES);
        LoadCharacterAtlas(HEAVY_CONFIG.textureMapName, ENEMY_ATLAS_FRAMES);
        LoadCharacterAtlas(RADIO_CONFIG.textureMapName, ENEMY_ATLAS_FRAMES);
        LoadCharacterAtlas(BOSS_CONFIG.textureMapName, ENEMY_ATLAS_FRAMES);

        XYZEngine::ResourceSystem::Instance()->LoadTextureMap(WEAPONS_TEXTURE, WEAPONS_ATLAS_FILE,
                                                             {WEAPON_FRAME_WIDTH, WEAPON_FRAME_HEIGHT}, WEAPON_ATLAS_FRAMES, false);

        LoadFxStrip(MUZZLE_FLASH_TEXTURE, FX_MUZZLE_FLASH);
        LoadFxStrip(BLOOD_POOL_TEXTURE, FX_BLOOD_POOL);
        LoadFxStrip(BLOOD_HIT_TEXTURE, FX_BLOOD_HIT);
        LoadFxStrip(IMPACT_TEXTURE, FX_IMPACT);
        LoadFxStrip(BULLET_TEXTURE, FX_BULLET);

        XYZEngine::ResourceSystem::Instance()->LoadTextureStrip(RELOAD_MAG_TEXTURE, RELOAD_MAG_FILE,
                                                                {0, 0, RELOAD_MAG_FRAME_SIZE, RELOAD_MAG_FRAME_SIZE}, RELOAD_MAG_FRAMES, false);

        XYZEngine::ResourceSystem::Instance()->LoadShader(HIT_FLASH_SHADER, HIT_FLASH_SHADER_FILE, sf::Shader::Fragment);

        XYZEngine::ResourceSystem::Instance()->LoadFont(HUD_FONT, HUD_FONT_FILE);

        XYZEngine::ResourceSystem::Instance()->LoadSound(SHOT_SOUND, SHOT_SOUND_FILE);
        XYZEngine::ResourceSystem::Instance()->LoadSound(HURT_SOUND, HURT_SOUND_FILE);

        LoadWeaponSounds();

        XYZEngine::ResourceSystem::Instance()->LoadMusic(MAIN_THEME_MUSIC, MAIN_THEME_FILE);
    }

    // Имя карты совпадает с именем файла.
    void GameResources::LoadCharacterAtlas(const std::string& name, int framesCount)
    {
        XYZEngine::ResourceSystem::Instance()->LoadTextureMap(name, TEXTURES_PATH + name + ".png",
                                                             {CHARACTER_FRAME_SIZE, CHARACTER_FRAME_SIZE}, framesCount, false);
    }

    const sf::SoundBuffer* GameResources::GetWeaponSound(const char* key)
    {
        return key == nullptr ? nullptr : XYZEngine::ResourceSystem::Instance()->GetSound(key);
    }

    const sf::SoundBuffer* GameResources::GetMeleeHitSound(const MeleeDefinition& melee)
    {
        if (melee.hitSound == nullptr || melee.hitSoundVariants <= 0)
        {
            return nullptr;
        }

        int variant = random<int>(1, melee.hitSoundVariants);
        return XYZEngine::ResourceSystem::Instance()->GetSound(std::string(melee.hitSound) + "_" + std::to_string(variant));
    }

    void GameResources::LoadWeaponSounds()
    {
        for (const MeleeDefinition& melee : MELEE_WEAPONS)
        {
            for (int variant = 1; variant <= melee.hitSoundVariants; variant++)
            {
                std::string key = std::string(melee.hitSound) + "_" + std::to_string(variant);
                XYZEngine::ResourceSystem::Instance()->LoadSound(key, WEAPONS_AUDIO_PATH + key + ".wav");
            }
        }

        for (const WeaponDefinition& weapon : WEAPONS)
        {
            if (weapon.shotSound != nullptr)
            {
                XYZEngine::ResourceSystem::Instance()->LoadSound(weapon.shotSound, WEAPONS_AUDIO_PATH + weapon.shotSound + ".wav");
            }

            if (weapon.reloadSound != nullptr)
            {
                XYZEngine::ResourceSystem::Instance()->LoadSound(weapon.reloadSound, WEAPONS_AUDIO_PATH + weapon.reloadSound + ".wav");
            }
        }
    }

    void GameResources::LoadFxStrip(const std::string& name, const FxStrip& strip)
    {
        XYZEngine::ResourceSystem::Instance()->LoadTextureStrip(name, FX_ATLAS_FILE,
                                                               {strip.x, strip.y, strip.width, strip.height}, strip.frames, false);
    }
}
