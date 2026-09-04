#include "pch.h"
#include "Engine.h"
#include <iostream>
#include "GameWorld.h"
#include "RenderSystem.h"
#include "InputSystem.h"
#include "FrameClock.h"
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

	void Engine::Run(Scene& scene)
	{
		scene.Start();

		LOG_INFO("Engine loop started");

		sf::Clock gameClock;
		sf::Event event;

		RenderSystem::Instance()->GetMainWindow().setKeyRepeatEnabled(false);

		while (RenderSystem::Instance()->GetMainWindow().isOpen())
		{
			sf::Time dt = gameClock.restart();
			float deltaTime = std::min(dt.asSeconds(), MAX_FRAME_TIME);
			FrameClock::Instance()->Advance(deltaTime);

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

			scene.Update(deltaTime);

			RenderSystem::Instance()->GetMainWindow().clear();

			if (!isPaused)
			{
				GameWorld::Instance()->Update(deltaTime);
				GameWorld::Instance()->FixedUpdate(deltaTime);
			}
			GameWorld::Instance()->Render();
			GameWorld::Instance()->LateUpdate();

			RenderSystem::Instance()->GetMainWindow().display();
		}

		LOG_INFO("Engine loop finished");

		scene.Stop();
	}

	void Engine::SetPaused(bool newIsPaused)
	{
		isPaused = newIsPaused;
	}
	bool Engine::IsPaused() const
	{
		return isPaused;
	}
}