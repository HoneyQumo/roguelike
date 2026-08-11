#include "Weapon.h"
#include "GameSettings.h"
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <LoggerRegistry.h>
#include <stdexcept>

namespace RoguelikeGame
{
	Weapon::Weapon(XYZEngine::GameObject* owner, const std::string& textureMapName, int frameIndex)
	{
		if (owner == nullptr)
		{
			throw std::runtime_error("weapon needs an owner");
		}

		auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(textureMapName, frameIndex);
		if (texture == nullptr)
		{
			throw std::runtime_error("weapon texture is not loaded: " + textureMapName);
		}

		gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Weapon");

		transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
		transform->SetParent(owner->GetComponent<XYZEngine::TransformComponent>());
		transform->SetLocalPosition(WEAPON_OFFSET_X, WEAPON_OFFSET_Y);

		renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
		renderer->SetTexture(*texture);
		renderer->SetPixelSize(WEAPON_SPRITE_SIZE, WEAPON_SPRITE_SIZE);

		LOG_INFO("Weapon created for " + owner->GetName());
	}

	XYZEngine::GameObject* Weapon::GetGameObject()
	{
		return gameObject;
	}
	XYZEngine::TransformComponent* Weapon::GetTransform()
	{
		return transform;
	}
	XYZEngine::SpriteRendererComponent* Weapon::GetRenderer()
	{
		return renderer;
	}
}
