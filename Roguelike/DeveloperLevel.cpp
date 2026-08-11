#include "DeveloperLevel.h"

using namespace XYZEngine;

namespace RoguelikeGame
{
	void DeveloperLevel::Start()
	{
		player = std::make_shared<Player>(XYZEngine::Vector2Df{ 0.f, 0.f });
		music = std::make_unique<Music>("main_theme", MUSIC_VOLUME);
	}
	void DeveloperLevel::Restart()
	{
		Stop();
		Start();
	}
	void DeveloperLevel::Stop() 
	{
		GameWorld::Instance()->Clear();
	}
}