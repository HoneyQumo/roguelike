#include "pch.h"
#include "Engine.h"
#include <iostream>
#include "GameWorld.h"
#include "RenderSystem.h"
#include "InputSystem.h"
#include "FrameClock.h"
#include "DebugDraw.h"
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

		auto& mainWindow = RenderSystem::Instance()->GetMainWindow();
		mainWindow.setKeyRepeatEnabled(false);
		InputSystem::Instance()->SetMousePosition(sf::Mouse::getPosition(mainWindow));

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
				if (event.type == sf::Event::Resized)
				{
					RenderSystem::Instance()->HandleResize(event.size.width, event.size.height);
				}

				InputSystem::Instance()->HandleEvent(event);

				if (event.type == sf::Event::GainedFocus)
				{
					InputSystem::Instance()->SyncWithDevice();
				}
			}

			if (!RenderSystem::Instance()->GetMainWindow().isOpen())
			{
				break;
			}

			if (InputSystem::Instance()->WasKeyPressed(DEBUG_DRAW_KEY))
			{
				DebugDraw::Instance()->Toggle();
			}

			scene.Update(deltaTime);
			float gameDeltaTime = FrameClock::Instance()->GetDeltaTime();

			RenderSystem::Instance()->GetMainWindow().clear();

			if (!isPaused)
			{
				GameWorld::Instance()->Update(gameDeltaTime);
				GameWorld::Instance()->UpdatePhysics();
			}
			GameWorld::Instance()->Render();
			DebugDraw::Instance()->Render();
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