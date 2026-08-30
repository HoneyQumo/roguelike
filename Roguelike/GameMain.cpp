#include <SFML/Graphics.hpp>
#include "Engine.h"
#include "RenderSystem.h"
#include "DeveloperLevel.h"
#include "GameResources.h"
#include "GameSettings.h"
#include "Logger.h"
#include "LoggerRegistry.h"
#include "ConsoleSink.h"
#include "FileSink.h"

using namespace RoguelikeGame;

/**
 *	Спецификация по спрайтам лежит в: Docs/Sprites/SPRITE_SPEC.md.
*/

void SetupLogger()
{
    auto logger = std::make_shared<Logger>();
    logger->AddSink(std::make_shared<ConsoleSink>());
    logger->AddSink(std::make_shared<FileSink>(LOG_FILE_PATH));

    LoggerRegistry::Instance()->RegisterLogger("global", logger);
    LoggerRegistry::Instance()->SetDefaultLogger(logger);
}

int main()
{
    SetupLogger();
    LOG_INFO("Game started");

    auto window = new sf::RenderWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Roguelike by HoneyQumo");
    window->setMouseCursorVisible(false);
    RenderSystem::Instance()->SetMainWindow(window);

    GameResources::Load();

    auto developerLevel = std::make_shared<DeveloperLevel>();
    developerLevel->Start();

    Engine::Instance()->Run();

    LOG_INFO("Game closed");

    return 0;
}
