#include "DeveloperLevel.h"
#include "GameSettings.h"
#include "LevelLoader.h"
#include <LoggerRegistry.h>

using namespace XYZEngine;

namespace RoguelikeGame
{
	void DeveloperLevel::Start()
	{
		LOG_INFO("Developer level is starting");

		try
		{
			levelBuilder.Build(LevelLoader::Load(TEST_LEVEL_PATH));
		}
		catch (const std::exception& exception)
		{
			LOG_ERROR(std::string("Level is not loaded: ") + exception.what());
			LOG_WARN("Game continues with an empty level");
		}

		music = std::make_unique<Music>("main_theme", MUSIC_VOLUME);

		try
		{
			crosshair = std::make_unique<Crosshair>();
		}
		catch (const std::exception& exception)
		{
			LOG_ERROR(std::string("Crosshair is not created: ") + exception.what());
		}
	}
	void DeveloperLevel::Restart()
	{
		Stop();
		Start();
	}
	void DeveloperLevel::Stop()
	{
		LOG_INFO("Developer level is stopping");

		crosshair.reset();
		music.reset();
		levelBuilder.Clear();

		GameWorld::Instance()->Clear();
	}
}
