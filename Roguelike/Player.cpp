#include "Player.h"
#include "GameSettings.h"
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <stdexcept>
#include <MovementComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <SpriteDirectionComponent.h>
#include <SpriteMovementAnimationComponent.h>

namespace RoguelikeGame
{
	Player::Player(const XYZEngine::Vector2Df& position)
	{
		gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Player");

		auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
		transform->SetWorldPosition(position);

		auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared("player", PLAYER_IDLE_FIRST_FRAME);
		if (texture == nullptr)
		{
			XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
			throw std::runtime_error("player texture map is not loaded");
		}

		auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
		renderer->SetTexture(*texture);
		renderer->SetPixelSize(CHARACTER_SPRITE_SIZE, CHARACTER_SPRITE_SIZE);

		auto camera = gameObject->AddComponent<XYZEngine::CameraComponent>();
		camera->SetWindow(&XYZEngine::RenderSystem::Instance()->GetMainWindow());
		camera->SetBaseResolution(SCREEN_WIDTH, SCREEN_HEIGHT);

		auto input = gameObject->AddComponent<XYZEngine::InputComponent>();

		auto movement = gameObject->AddComponent<XYZEngine::MovementComponent>();
		movement->SetSpeed(PLAYER_SPEED);

		auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
		body->SetKinematic(false);

		auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
		collider->SetSize(CHARACTER_COLLIDER_WIDTH, CHARACTER_COLLIDER_HEIGHT);
		collider->SetOffset(0.f, CHARACTER_COLLIDER_OFFSET_Y);

		auto direction = gameObject->AddComponent<XYZEngine::SpriteDirectionComponent>();

		auto animation = gameObject->AddComponent<XYZEngine::SpriteMovementAnimationComponent>();
		animation->SetWalkAnimation("player", PLAYER_WALK_FIRST_FRAME, PLAYER_WALK_FRAMES, WALK_FRAMERATE);
		animation->SetIdleAnimation("player", PLAYER_IDLE_FIRST_FRAME, PLAYER_IDLE_FRAMES, IDLE_FRAMERATE);

		LOG_INFO("Player created at " + std::to_string((int)position.x) + ";" + std::to_string((int)position.y));
	}

	XYZEngine::GameObject* Player::GetGameObject()
	{
		return gameObject;
	}
}
