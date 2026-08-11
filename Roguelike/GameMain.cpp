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

	auto window = new sf::RenderWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Roguelike by HoneyQumo");
	window->setMouseCursorVisible(false);
	XYZEngine::RenderSystem::Instance()->SetMainWindow(window);

	XYZEngine::ResourceSystem::Instance()->LoadTexture("crosshair", "Resources/Textures/crosshair.png", false);

	XYZEngine::ResourceSystem::Instance()->LoadTextureMap("player", "Resources/Textures/vietnam_war1.png", { CHARACTER_FRAME_SIZE, CHARACTER_FRAME_SIZE }, CHARACTER_FRAMES_IN_MAP, false);
	XYZEngine::ResourceSystem::Instance()->LoadTextureMap("enemy", "Resources/Textures/prisoner.png", { CHARACTER_FRAME_SIZE, CHARACTER_FRAME_SIZE }, CHARACTER_FRAMES_IN_MAP, false);
	XYZEngine::ResourceSystem::Instance()->LoadTextureMap("rifleman", "Resources/Textures/vietnam_war3.png", { CHARACTER_FRAME_SIZE, CHARACTER_FRAME_SIZE }, CHARACTER_FRAMES_IN_MAP, false);

	XYZEngine::ResourceSystem::Instance()->LoadTexturePart("player_weapon", "Resources/Textures/vietnam_war1.png", { 64, 104, WEAPON_REGION_WIDTH, WEAPON_REGION_HEIGHT }, false);
	XYZEngine::ResourceSystem::Instance()->LoadTexturePart("rifleman_weapon", "Resources/Textures/vietnam_war3.png", { 64, 70, WEAPON_REGION_WIDTH, WEAPON_REGION_HEIGHT }, false);

	XYZEngine::ResourceSystem::Instance()->LoadSound("shot", "Resources/Audio/shot.wav");
	XYZEngine::ResourceSystem::Instance()->LoadSound("hurt", "Resources/Audio/hurt.wav");

	XYZEngine::ResourceSystem::Instance()->LoadMusic("main_theme", "Resources/Audio/main_music_1.ogg");

	auto developerLevel = std::make_shared<DeveloperLevel>();
	developerLevel->Start();

	XYZEngine::Engine::Instance()->Run();

	LOG_INFO("Game closed");

	return 0;
}
