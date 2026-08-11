#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Engine.h"
#include "ResourceSystem.h"
#include "RenderSystem.h"
#include "DeveloperLevel.h"
#include "GameSettings.h"
#include "Matrix2D.h"

using namespace RoguelikeGame;

int main()
{
	XYZEngine::RenderSystem::Instance()->SetMainWindow(new sf::RenderWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Roguelike by HoneyQumo"));

	XYZEngine::ResourceSystem::Instance()->LoadTextureMap("player", "Resources/Textures/vietnam_war1.png", { CHARACTER_FRAME_SIZE, CHARACTER_FRAME_SIZE }, CHARACTER_FRAMES_IN_MAP, false);
	XYZEngine::ResourceSystem::Instance()->LoadTextureMap("enemy", "Resources/Textures/prisoner.png", { CHARACTER_FRAME_SIZE, CHARACTER_FRAME_SIZE }, CHARACTER_FRAMES_IN_MAP, false);

	XYZEngine::ResourceSystem::Instance()->LoadMusic("main_theme", "Resources/Audio/music.wav");

	auto developerLevel = std::make_shared<DeveloperLevel>();
	developerLevel->Start();

	XYZEngine::Engine::Instance()->Run();

	return 0;
}
