#include "DeveloperLevel.h"

using namespace XYZEngine;

namespace RoguelikeGame
{
	void DeveloperLevel::Start()
	{
		player = std::make_shared<Player>();
		music = std::make_unique<Music>("main_theme", 15.f);
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