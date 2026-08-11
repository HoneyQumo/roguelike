#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Engine.h"
#include "ResourceSystem.h"
#include "RenderSystem.h"
#include "DeveloperLevel.h"
#include "Matrix2D.h"

using namespace RoguelikeGame;

int main()
{
	XYZEngine::RenderSystem::Instance()->SetMainWindow(new sf::RenderWindow(sf::VideoMode(1280, 720), "Roguelike by HoneyQumo"));

	XYZEngine::ResourceSystem::Instance()->LoadTexture("ball", "Resources/Textures/ball.png");

	auto developerLevel = std::make_shared<DeveloperLevel>();
	developerLevel->Start();

	std::cout << "TEST" << std::endl;
	
	XYZEngine::Engine::Instance()->Run();

	return 0;
}
