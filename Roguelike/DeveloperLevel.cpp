#include "DeveloperLevel.h"
#include "GameSettings.h"
#include "LevelLoader.h"

using namespace XYZEngine;

namespace RoguelikeGame
{
	void DeveloperLevel::Start()
	{
		LevelData levelData;
		if (LevelLoader::Load(TEST_LEVEL_PATH, levelData))
		{
			levelBuilder.Build(levelData);
		}

		music = std::make_unique<Music>("main_theme", MUSIC_VOLUME);
	}
	void DeveloperLevel::Restart()
	{
		Stop();
		Start();
	}
	void DeveloperLevel::Stop()
	{
		music.reset();
		levelBuilder.Clear();

		GameWorld::Instance()->Clear();
	}
}
