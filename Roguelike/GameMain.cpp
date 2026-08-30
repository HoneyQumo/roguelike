#include <SFML/Graphics.hpp>
#include <string>
#include "Player.h"
#include "Engine.h"
#include "ResourceSystem.h"
#include "RenderSystem.h"
#include "DeveloperLevel.h"
#include "GameSettings.h"
#include "Logger.h"
#include "LoggerRegistry.h"
#include "ConsoleSink.h"
#include "FileSink.h"

using namespace RoguelikeGame;

/**	
 *	Спецификация по спрайтам лежит в: Docs/Sprites/SPRITE_SPEC.md.
*/

const std::string TEXTURES_PATH = "Resources/Textures/";
const std::string FX_ATLAS_PATH = TEXTURES_PATH + "fx.png";

void SetupLogger()
{
    auto logger = std::make_shared<Logger>();
    logger->AddSink(std::make_shared<ConsoleSink>());
    logger->AddSink(std::make_shared<FileSink>(LOG_FILE_PATH));

    LoggerRegistry::Instance()->RegisterLogger("global", logger);
    LoggerRegistry::Instance()->SetDefaultLogger(logger);
}

// Имя карты совпадает с именем файла.
void LoadCharacterAtlas(const std::string& name)
{
    ResourceSystem::Instance()->LoadTextureMap(name, TEXTURES_PATH + name + ".png",
                                               {CHARACTER_FRAME_SIZE, CHARACTER_FRAME_SIZE}, CHARACTER_ATLAS_FRAMES, false);
}

void LoadFxStrip(const std::string& name, const FxStrip& strip)
{
    ResourceSystem::Instance()->LoadTextureStrip(name, FX_ATLAS_PATH,
                                                 {strip.x, strip.y, strip.width, strip.height}, strip.frames, false);
}

void LoadResources()
{
    ResourceSystem::Instance()->LoadTexture(CROSSHAIR_TEXTURE, TEXTURES_PATH + "crosshair.png", false);

    LoadCharacterAtlas(PLAYER_TEXTURE);
    LoadCharacterAtlas(GRUNT_CONFIG.textureMapName);
    LoadCharacterAtlas(ASSAULT_CONFIG.textureMapName);
    LoadCharacterAtlas(SHIELD_CONFIG.textureMapName);
    LoadCharacterAtlas(HEAVY_CONFIG.textureMapName);
    LoadCharacterAtlas(RADIO_CONFIG.textureMapName);
    LoadCharacterAtlas(BOSS_CONFIG.textureMapName);

    ResourceSystem::Instance()->LoadTextureMap(WEAPONS_TEXTURE, TEXTURES_PATH + "weapons.png",
                                               {WEAPON_FRAME_WIDTH, WEAPON_FRAME_HEIGHT}, WEAPON_ATLAS_FRAMES, false);

    LoadFxStrip(MUZZLE_FLASH_TEXTURE, FX_MUZZLE_FLASH);
    LoadFxStrip(BLOOD_POOL_TEXTURE, FX_BLOOD_POOL);
    LoadFxStrip(BLOOD_HIT_TEXTURE, FX_BLOOD_HIT);
    LoadFxStrip(IMPACT_TEXTURE, FX_IMPACT);
    LoadFxStrip(BULLET_TEXTURE, FX_BULLET);

    ResourceSystem::Instance()->LoadShader(HIT_FLASH_SHADER, "Resources/Shaders/hit_flash.frag", sf::Shader::Fragment);

    ResourceSystem::Instance()->LoadSound("shot", "Resources/Audio/shot.wav");
    ResourceSystem::Instance()->LoadSound("hurt", "Resources/Audio/hurt.wav");

    ResourceSystem::Instance()->LoadMusic("main_theme", "Resources/Audio/main_music_1.ogg");
}

int main()
{
    SetupLogger();
    LOG_INFO("Game started");

    auto window = new sf::RenderWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Roguelike by HoneyQumo");
    window->setMouseCursorVisible(false);
    RenderSystem::Instance()->SetMainWindow(window);

    LoadResources();

    auto developerLevel = std::make_shared<DeveloperLevel>();
    developerLevel->Start();

    Engine::Instance()->Run();

    LOG_INFO("Game closed");

    return 0;
}
