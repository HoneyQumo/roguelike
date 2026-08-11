#include "Player.h"
#include "GameSettings.h"
#include <ResourceSystem.h>
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

		auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
		renderer->SetTexture(*XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared("player", WALK_FIRST_FRAME));
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
		collider->SetSize(CHARACTER_COLLIDER_SIZE, CHARACTER_COLLIDER_SIZE);

		auto direction = gameObject->AddComponent<XYZEngine::SpriteDirectionComponent>();

		auto animation = gameObject->AddComponent<XYZEngine::SpriteMovementAnimationComponent>();
		animation->SetFrames("player", WALK_FIRST_FRAME, WALK_FRAMES_COUNT);
		animation->SetFramerate(WALK_FRAMERATE);
	}

	XYZEngine::GameObject* Player::GetGameObject()
	{
		return gameObject;
	}
}
