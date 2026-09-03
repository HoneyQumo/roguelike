#include "pch.h"
#include "Engine.h"
#include <iostream>
#include "GameWorld.h"
#include "RenderSystem.h"
#include "InputSystem.h"
#include "LoggerRegistry.h"

namespace XYZEngine
{
	Engine* Engine::Instance()
	{
		static Engine instance;
		return &instance;
	}

	Engine::Engine()
	{
		unsigned int seed = (unsigned int)time(nullptr);
		srand(seed);
	}

	void Engine::Run()
	{
		LOG_INFO("Engine loop started");

		sf::Clock gameClock;
		sf::Event event;

		RenderSystem::Instance()->GetMainWindow().setKeyRepeatEnabled(false);

		while (RenderSystem::Instance()->GetMainWindow().isOpen())
		{
			sf::Time dt = gameClock.restart();
			float deltaTime = dt.asSeconds();

			InputSystem::Instance()->BeginFrame();
			while (RenderSystem::Instance()->GetMainWindow().pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					RenderSystem::Instance()->GetMainWindow().close();
				}

				InputSystem::Instance()->HandleEvent(event);
			}

			if (!RenderSystem::Instance()->GetMainWindow().isOpen())
			{
				break;
			}

			RenderSystem::Instance()->GetMainWindow().clear();

			GameWorld::Instance()->Update(deltaTime);
			GameWorld::Instance()->FixedUpdate(deltaTime);
			GameWorld::Instance()->Render();
			GameWorld::Instance()->LateUpdate();

			RenderSystem::Instance()->GetMainWindow().display();
		}

		LOG_INFO("Engine loop finished");
	}
}