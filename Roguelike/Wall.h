#pragma once

#include "GameWorld.h"
#include "GameObject.h"
#include "Vector.h"

namespace RoguelikeGame
{
	class Wall
	{
	public:
		Wall(const XYZEngine::Vector2Df& position);
		XYZEngine::GameObject* GetGameObject();
	private:
		XYZEngine::GameObject* gameObject;
	};
}
