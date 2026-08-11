#include "Enemy.h"
#include "GameSettings.h"
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <stdexcept>
#include <ChaseComponent.h>
#include <MovementComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <SpriteDirectionComponent.h>
#include <SpriteMovementAnimationComponent.h>

namespace RoguelikeGame
{
	Enemy::Enemy(const XYZEngine::Vector2Df& position)
	{
		gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Enemy");

		auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
		transform->SetWorldPosition(position);

		auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared("enemy", ENEMY_IDLE_FIRST_FRAME);
		if (texture == nullptr)
		{
			XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
			throw std::runtime_error("enemy texture map is not loaded");
		}

		auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
		renderer->SetTexture(*texture);
		renderer->SetPixelSize(CHARACTER_SPRITE_SIZE, CHARACTER_SPRITE_SIZE);

		// Chase sets the direction, movement applies it: keep chase updated first.
		auto chase = gameObject->AddComponent<XYZEngine::ChaseComponent>();
		chase->SetTargetName("Player");
		chase->SetDetectionRadius(ENEMY_DETECTION_RADIUS);

		auto movement = gameObject->AddComponent<XYZEngine::MovementComponent>();
		movement->SetSpeed(ENEMY_SPEED);

		auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
		body->SetKinematic(false);

		auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
		collider->SetSize(CHARACTER_COLLIDER_WIDTH, CHARACTER_COLLIDER_HEIGHT);
		collider->SetOffset(0.f, CHARACTER_COLLIDER_OFFSET_Y);

		auto direction = gameObject->AddComponent<XYZEngine::SpriteDirectionComponent>();

		auto animation = gameObject->AddComponent<XYZEngine::SpriteMovementAnimationComponent>();
		animation->SetWalkAnimation("enemy", ENEMY_WALK_FIRST_FRAME, ENEMY_WALK_FRAMES, WALK_FRAMERATE);
		animation->SetIdleAnimation("enemy", ENEMY_IDLE_FIRST_FRAME, ENEMY_IDLE_FRAMES, IDLE_FRAMERATE);

		LOG_INFO("Enemy created at " + std::to_string((int)position.x) + ";" + std::to_string((int)position.y));
	}

	XYZEngine::GameObject* Enemy::GetGameObject()
	{
		return gameObject;
	}
}
