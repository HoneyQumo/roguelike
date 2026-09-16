#include "GameResources.h"
#include "Item.h"
#include "Prop.h"
#include "ItemCatalogLoader.h"
#include "GameSettings.h"
#include "WeaponCatalog.h"
#include "BossCatalog.h"
#include "BossSpriteAtlas.h"
#include "EnemyCatalog.h"
#include "EnemyVoice.h"
#include "Fixtures.h"
#include "TileAtlas.h"
#include <PixelBounds.h>
#include <ResourceSystem.h>
#include <randomizer.h>
#include <LoggerRegistry.h>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <map>

namespace RoguelikeGame
{
    namespace
    {
        ItemCatalog items;
        LevelCatalog levels;
        LootCatalog loot;
        PropCatalog props;
    }

    namespace
    {
        void TrimItemIcons(ItemCatalog& catalog)
        {
            std::map<std::string, sf::Image> sources;

            for (ItemDefinition& item : catalog)
            {
                auto source = sources.find(item.icon.texturePath);

                if (source == sources.end())
                {
                    sf::Image image;

                    if (!image.loadFromFile(item.icon.texturePath))
                    {
                        LOG_ERROR("Item icon texture is not read: " + item.icon.texturePath);
                        continue;
                    }

                    source = sources.emplace(item.icon.texturePath, std::move(image)).first;
                }

                item.icon.rect = XYZEngine::OpaqueBounds(source->second, item.icon.rect, ITEM_ICON_ALPHA_THRESHOLD);
            }
        }

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

        for (const BossDefinition& boss : BOSSES)
        {
            if (boss.textureMapName != nullptr)
            {
                LoadCharacterAtlas(boss.textureMapName, BOSS_ATLAS_FRAMES, BOSS_FRAME_SIZE);
            }
        }

        XYZEngine::ResourceSystem::Instance()->LoadTextureMap(WEAPONS_TEXTURE, WEAPONS_ATLAS_FILE,
                                                             {WEAPON_FRAME_WIDTH, WEAPON_FRAME_HEIGHT}, WEAPON_ATLAS_FRAMES, false);

        XYZEngine::ResourceSystem::Instance()->LoadTexture(FX_ATLAS_TEXTURE, FX_ATLAS_FILE, false);
        XYZEngine::ResourceSystem::Instance()->LoadTexturePart(DOOR_LOCKED_TEXTURE, DOORS_ATLAS_FILE,
                                                              {0, 0, static_cast<int>(TILE_SIZE), static_cast<int>(TILE_SIZE)}, false);
        XYZEngine::ResourceSystem::Instance()->LoadTexturePart(DOOR_OPEN_TEXTURE, DOORS_ATLAS_FILE,
                                                              {static_cast<int>(TILE_SIZE), 0, static_cast<int>(TILE_SIZE), static_cast<int>(TILE_SIZE)}, false);
        XYZEngine::ResourceSystem::Instance()->LoadTexturePart(ESCAPE_CAR_OPEN_TEXTURE, ESCAPE_CAR_ATLAS_FILE,
                                                              {ESCAPE_CAR_OPEN_FRAME_LEFT, 0, ESCAPE_CAR_OPEN_FRAME_SIDE, ESCAPE_CAR_OPEN_FRAME_SIDE}, false);

        LoadFxStrip(MUZZLE_FLASH_TEXTURE, FX_MUZZLE_FLASH);
        LoadFxStrip(BLOOD_POOL_TEXTURE, FX_BLOOD_POOL);
        LoadFxStrip(BLOOD_HIT_TEXTURE, FX_BLOOD_HIT);
        LoadFxStrip(IMPACT_TEXTURE, FX_IMPACT);
        LoadFxStrip(BULLET_TEXTURE, FX_BULLET);
        LoadFxStrip(ROCKET_TEXTURE, FX_ROCKET);
        LoadFxStrip(EXPLOSION_TEXTURE, FX_EXPLOSION);

        // Огонь лежит в своём атласе, поэтому грузится не через LoadFxStrip.
        XYZEngine::ResourceSystem::Instance()->LoadTextureStrip(FIRE_BIG_TEXTURE, FIRE_TEXTURE_FILE,
                                                               {FX_FIRE_BIG.x, FX_FIRE_BIG.y, FX_FIRE_BIG.width, FX_FIRE_BIG.height},
                                                               FX_FIRE_BIG.frames, false);
        XYZEngine::ResourceSystem::Instance()->LoadTextureStrip(FIRE_SMALL_TEXTURE, FIRE_TEXTURE_FILE,
                                                               {FX_FIRE_SMALL.x, FX_FIRE_SMALL.y, FX_FIRE_SMALL.width, FX_FIRE_SMALL.height},
                                                               FX_FIRE_SMALL.frames, false);

        XYZEngine::ResourceSystem::Instance()->LoadTextureStrip(SMOKE_TEXTURE, SMOKE_TEXTURE_FILE,
                                                               {FX_TIRE_SMOKE.x, FX_TIRE_SMOKE.y, FX_TIRE_SMOKE.width, FX_TIRE_SMOKE.height},
                                                               FX_TIRE_SMOKE.frames, false);

        LoadBossFxStrip(PUPPETEER_RIFT_TEXTURE, FX_PUPPETEER_RIFT);
        LoadBossFxStrip(PUPPETEER_CLOUD_TEXTURE, FX_PUPPETEER_CLOUD);
        LoadBossFxStrip(PUPPETEER_SNAP_TEXTURE, FX_PUPPETEER_SNAP);
        LoadBossFxStrip(PUPPETEER_MARK_TEXTURE, FX_PUPPETEER_MARK);

        XYZEngine::ResourceSystem::Instance()->LoadTextureStrip(RELOAD_MAG_TEXTURE, RELOAD_MAG_FILE,
                                                                {0, 0, RELOAD_MAG_FRAME_SIZE, RELOAD_MAG_FRAME_SIZE}, RELOAD_MAG_FRAMES, false);

        LoadItems();
        LoadLevels();
        LoadLoot();
        LoadProps();

        XYZEngine::ResourceSystem::Instance()->LoadShader(HIT_FLASH_SHADER, HIT_FLASH_SHADER_FILE, sf::Shader::Fragment);

        XYZEngine::ResourceSystem::Instance()->LoadFont(HUD_FONT, HUD_FONT_FILE);

        XYZEngine::ResourceSystem::Instance()->LoadSound(SHOT_SOUND, SHOT_SOUND_FILE);
        XYZEngine::ResourceSystem::Instance()->LoadSound(HURT_SOUND, HURT_SOUND_FILE);
        XYZEngine::ResourceSystem::Instance()->LoadSound(DOOR_OPEN_SOUND, DOOR_OPEN_SOUND_FILE);
        XYZEngine::ResourceSystem::Instance()->LoadSound(LEVER_SOUND, LEVER_SOUND_FILE);
        XYZEngine::ResourceSystem::Instance()->LoadSound(HATCH_SOUND, HATCH_SOUND_FILE);
        XYZEngine::ResourceSystem::Instance()->LoadSound(CAR_ENGINE_SOUND, CAR_ENGINE_SOUND_FILE);
        XYZEngine::ResourceSystem::Instance()->LoadSound(CAR_SKID_SOUND, CAR_SKID_SOUND_FILE);
        LoadFixtures();
        LoadVoiceLines();

        LoadWeaponSounds();

        XYZEngine::ResourceSystem::Instance()->LoadMusic(MAIN_THEME_MUSIC, MAIN_THEME_FILE);
    }

    // Имя карты совпадает с именем файла.
    void GameResources::LoadCharacterAtlas(const std::string& name, int framesCount, int frameSize)
    {
        XYZEngine::ResourceSystem::Instance()->LoadTextureMap(name, TEXTURES_PATH + name + ".png",
                                                             {static_cast<unsigned int>(frameSize), static_cast<unsigned int>(frameSize)},
                                                             framesCount, false);
    }

    void GameResources::LoadBossFxStrip(const std::string& name, const FxStrip& strip)
    {
        XYZEngine::ResourceSystem::Instance()->LoadTextureStrip(name, BOSS_FX_ATLAS_FILE,
                                                               {strip.x, strip.y, strip.width, strip.height}, strip.frames, false);
    }

    const sf::SoundBuffer* GameResources::GetWeaponSound(const char* key)
    {
        return key == nullptr ? nullptr : XYZEngine::ResourceSystem::Instance()->GetSound(key);
    }

    const sf::SoundBuffer* GameResources::GetVoiceLine(const char* voice, int line)
    {
        std::string key = VoiceKey(voice, line);

        return key.empty() ? nullptr : XYZEngine::ResourceSystem::Instance()->GetSound(key);
    }

    void GameResources::LoadVoiceLines()
    {
        for (const EnemyDefinition& enemy : ENEMIES)
        {
            for (int line = 1; line <= enemy.config.voiceLines; line++)
            {
                std::string key = VoiceKey(enemy.config.voice, line);
                if (!key.empty() && !XYZEngine::ResourceSystem::Instance()->HasSound(key))
                {
                    XYZEngine::ResourceSystem::Instance()->LoadSound(key, VoiceFilePath(enemy.config.voice, line));
                }
            }
        }
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

        TrimItemIcons(items);

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

    void GameResources::LoadLoot()
    {
        try
        {
            loot = LootCatalog::Load(LOOT_CATALOG_FILE);
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Loot catalog is not loaded: ") + exception.what());
        }
    }

    const LootCatalog& GameResources::GetLoot()
    {
        return loot;
    }

    void GameResources::LoadFixtures()
    {
        for (int frame = 0; frame < HATCH_FRAMES; frame++)
        {
            XYZEngine::ResourceSystem::Instance()->LoadTexturePart(
                std::string(HATCH_TEXTURE_PREFIX) + std::to_string(frame), FIXTURES_TEXTURE_FILE,
                sf::IntRect(frame * TILE_FRAME_SIZE, 0, TILE_FRAME_SIZE, TILE_FRAME_SIZE), false);
        }

        for (int frame = 0; frame < LEVER_FRAMES; frame++)
        {
            XYZEngine::ResourceSystem::Instance()->LoadTexturePart(
                std::string(LEVER_TEXTURE_PREFIX) + std::to_string(frame), FIXTURES_TEXTURE_FILE,
                sf::IntRect((HATCH_FRAMES + frame) * TILE_FRAME_SIZE, 0, TILE_FRAME_SIZE, TILE_FRAME_SIZE), false);
        }
    }

    void GameResources::LoadProps()
    {
        try
        {
            props = PropCatalog::Load(PROPS_CATALOG_FILE);
        }
        catch (const std::exception& exception)
        {
            LOG_ERROR(std::string("Prop catalog is not loaded: ") + exception.what());
            return;
        }

        for (const PropDefinition& prop : props)
        {
            if (prop.HasFrame())
            {
                XYZEngine::ResourceSystem::Instance()->LoadTexturePart(PropTextureName(prop.id, false), prop.texturePath,
                                                                       prop.frame, false);
            }

            if (prop.HasSpentFrame())
            {
                XYZEngine::ResourceSystem::Instance()->LoadTexturePart(PropTextureName(prop.id, true), prop.texturePath,
                                                                       prop.spentFrame, false);
            }
        }
    }

    const PropCatalog& GameResources::GetProps()
    {
        return props;
    }

    const LevelCatalog& GameResources::GetLevels()
    {
        return levels;
    }
}
