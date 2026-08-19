#pragma once

#include "GameWorld.h"
#include "GameObject.h"
#include "SpriteRendererComponent.h"
#include "Vector.h"

namespace RoguelikeGame
{
	class Enemy
	{
	public:
		Enemy(const XYZEngine::Vector2Df& position);
		XYZEngine::GameObject* GetGameObject();
	private:
		XYZEngine::GameObject* gameObject;
	};
}
