#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Engine.h"
#include <windows.h>
#include <iostream>

const std::string RESOURCES_PATH = "Resources/";

int main()
{
	if (AllocConsole())
	{
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
		freopen_s(&fp, "CONOUT$", "w", stderr);
	}
	else
	{
		std::cerr << "Не удалось выделить консоль" << std::endl;
	}

	Engine engine;
	engine.Initialize();
	engine.Run();

	sf::RenderWindow window(sf::VideoMode(800, 600), "Roguelike by HoneyQumo");
	
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.display();
	}

	return 0;
}
