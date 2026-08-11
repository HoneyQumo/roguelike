#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Engine.h"
#include "ResourceSystem.h"
#include "RenderSystem.h"
#include "DeveloperLevel.h"
#include "GameSettings.h"
#include "Matrix2D.h"
#include "Logger.h"
#include "LoggerRegistry.h"
#include "ConsoleSink.h"
#include "FileSink.h"

using namespace RoguelikeGame;

/**	Assets resource links
*	https://momongaa.itch.io/1970s-soldiers
*/ 

void SetupLogger()
{
	auto logger = std::make_shared<XYZEngine::Logger>();
	logger->AddSink(std::make_shared<XYZEngine::ConsoleSink>());
	logger->AddSink(std::make_shared<XYZEngine::FileSink>(LOG_FILE_PATH));

	XYZEngine::LoggerRegistry::Instance()->RegisterLogger("global", logger);
	XYZEngine::LoggerRegistry::Instance()->SetDefaultLogger(logger);
}

int main()
{
	SetupLogger();
	LOG_INFO("Game started");

	XYZEngine::RenderSystem::Instance()->SetMainWindow(new sf::RenderWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Roguelike by HoneyQumo"));

	XYZEngine::ResourceSystem::Instance()->LoadTextureMap("player", "Resources/Textures/vietnam_war1.png", { CHARACTER_FRAME_SIZE, CHARACTER_FRAME_SIZE }, CHARACTER_FRAMES_IN_MAP, false);
	XYZEngine::ResourceSystem::Instance()->LoadTextureMap("enemy", "Resources/Textures/prisoner.png", { CHARACTER_FRAME_SIZE, CHARACTER_FRAME_SIZE }, CHARACTER_FRAMES_IN_MAP, false);

	XYZEngine::ResourceSystem::Instance()->LoadMusic("main_theme", "Resources/Audio/main_music_1.ogg");

	auto developerLevel = std::make_shared<DeveloperLevel>();
	developerLevel->Start();

	XYZEngine::Engine::Instance()->Run();

	LOG_INFO("Game closed");

	return 0;
}
