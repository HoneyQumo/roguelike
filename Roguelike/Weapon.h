#pragma once

#include <string>
#include <GameObject.h>
#include <TransformComponent.h>
#include <SpriteRendererComponent.h>

namespace RoguelikeGame
{
	class Weapon
	{
	public:
		Weapon(XYZEngine::GameObject* owner, const std::string& textureMapName, int frameIndex);

		XYZEngine::GameObject* GetGameObject();
		XYZEngine::TransformComponent* GetTransform();
		XYZEngine::SpriteRendererComponent* GetRenderer();
	private:
		XYZEngine::GameObject* gameObject;
		XYZEngine::TransformComponent* transform;
		XYZEngine::SpriteRendererComponent* renderer;
	};
}
