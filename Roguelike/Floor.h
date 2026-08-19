#pragma once

#include "GameWorld.h"
#include "GameObject.h"
#include "Vector.h"

namespace RoguelikeGame
{
	class Floor
	{
	public:
		Floor(const XYZEngine::Vector2Df& position);
		XYZEngine::GameObject* GetGameObject();
	private:
		XYZEngine::GameObject* gameObject;
	};
}
