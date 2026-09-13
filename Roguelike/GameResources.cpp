#include "GameResources.h"
#include "Item.h"
#include "ItemCatalogLoader.h"
#include "GameSettings.h"
#include "WeaponCatalog.h"
#include "EnemyCatalog.h"
#include <ResourceSystem.h>
#include <randomizer.h>
#include <SFML/Graphics/Shader.hpp>

namespace RoguelikeGame
{
    namespace
    {
        ItemCatalog items;
        LevelCatalog levels;
    }

    namespace
    {
        std::string MeleeHitSoundKey(const MeleeDefinition& melee, int variant)
        {
            return std::string(melee.hitSound) + "_" + std::to_string(variant);
        }
    }

    void GameResources::Load()
    {
        XYZEngine::ResourceSystem::Instance()->LoadTexture(CROSSHAIR_TEXTURE, CROSSHAIR_FILE, false);

        LoadCharacterAtlas(PLAYER_TEXTURE, PLAYER_ATLAS_FRAMES);
        for (const EnemyDefinition& enemy : ENEMIES)
        {
            LoadCharacterAtlas(enemy.config.textureMapName, ENEMY_ATLAS_FRAMES);
        }

        XYZEngine::ResourceSystem::Instance()->LoadTextureMap(WEAPONS_TEXTURE, WEAPONS_ATLAS_FILE,
                                                             {WEAPON_FRAME_WIDTH, WEAPON_FRAME_HEIGHT}, WEAPON_ATLAS_FRAMES, false);

        XYZEngine::ResourceSystem::Instance()->LoadTexture(FX_ATLAS_TEXTURE, FX_ATLAS_FILE, false);

        LoadFxStrip(MUZZLE_FLASH_TEXTURE, FX_MUZZLE_FLASH);
        LoadFxStrip(BLOOD_POOL_TEXTURE, FX_BLOOD_POOL);
        LoadFxStrip(BLOOD_HIT_TEXTURE, FX_BLOOD_HIT);
        LoadFxStrip(IMPACT_TEXTURE, FX_IMPACT);
        LoadFxStrip(BULLET_TEXTURE, FX_BULLET);
        LoadFxStrip(ROCKET_TEXTURE, FX_ROCKET);
        LoadFxStrip(EXPLOSION_TEXTURE, FX_EXPLOSION);

        XYZEngine::ResourceSystem::Instance()->LoadTextureStrip(RELOAD_MAG_TEXTURE, RELOAD_MAG_FILE,
                                                                {0, 0, RELOAD_MAG_FRAME_SIZE, RELOAD_MAG_FRAME_SIZE}, RELOAD_MAG_FRAMES, false);

        LoadItems();
        LoadLevels();

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
        return XYZEngine::ResourceSystem::Instance()->GetSound(MeleeHitSoundKey(melee, variant));
    }

    void GameResources::LoadWeaponSound(const std::string& key)
    {
        XYZEngine::ResourceSystem::Instance()->LoadSound(key, WEAPONS_AUDIO_PATH + key + ".wav");
    }

    void GameResources::LoadWeaponSounds()
    {
        for (const WeaponDefinition& weapon : WEAPONS)
        {
            if (weapon.shotSound != nullptr)
            {
                LoadWeaponSound(weapon.shotSound);
            }

            if (weapon.reloadSound != nullptr)
            {
                LoadWeaponSound(weapon.reloadSound);
            }

            if (weapon.melee == nullptr)
            {
                continue;
            }

            for (int variant = 1; variant <= weapon.melee->hitSoundVariants; variant++)
            {
                LoadWeaponSound(MeleeHitSoundKey(*weapon.melee, variant));
            }
        }
    }

    void GameResources::LoadFxStrip(const std::string& name, const FxStrip& strip)
    {
        XYZEngine::ResourceSystem::Instance()->LoadTextureStrip(name, FX_ATLAS_FILE,
                                                               {strip.x, strip.y, strip.width, strip.height}, strip.frames, false);
    }

    void GameResources::LoadItems()
    {
        try
        {
            items = ItemCatalogLoader::Load(ITEMS_CATALOG_FILE);
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Item catalog is not loaded: ") + exception.what());
            return;
        }

        for (const ItemDefinition& item : items)
        {
            XYZEngine::ResourceSystem::Instance()->LoadTexturePart(ItemTextureName(item.id), item.icon.texturePath,
                                                                   item.icon.rect, false);
        }

        LOG_INFO("Items loaded: " + std::to_string(items.Size()));
    }

    const ItemCatalog& GameResources::GetItems()
    {
        return items;
    }

    void GameResources::LoadLevels()
    {
        try
        {
            levels = LevelCatalog::Load(LEVELS_CATALOG_FILE);
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Level catalog is not loaded: ") + exception.what());
            return;
        }

        LOG_INFO("Levels loaded: " + std::to_string(levels.Size()));
    }

    const LevelCatalog& GameResources::GetLevels()
    {
        return levels;
    }
}
