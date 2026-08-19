#pragma once

#include <string>
#include "GameObject.h"
#include "GameWorld.h"
#include "MusicComponent.h"

namespace RoguelikeGame
{
	class Music
	{
	public:
		Music(const std::string& musicName, float volume);
		XYZEngine::GameObject* GetGameObject();
	private:
		XYZEngine::GameObject* gameObject;
	};
}
