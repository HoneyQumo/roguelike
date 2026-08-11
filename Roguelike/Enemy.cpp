#include "Enemy.h"
#include "GameSettings.h"
#include <ResourceSystem.h>
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

		auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
		renderer->SetTexture(*XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared("enemy", WALK_FIRST_FRAME));
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
		collider->SetSize(CHARACTER_COLLIDER_SIZE, CHARACTER_COLLIDER_SIZE);

		auto direction = gameObject->AddComponent<XYZEngine::SpriteDirectionComponent>();

		auto animation = gameObject->AddComponent<XYZEngine::SpriteMovementAnimationComponent>();
		animation->SetFrames("enemy", WALK_FIRST_FRAME, WALK_FRAMES_COUNT);
		animation->SetFramerate(WALK_FRAMERATE);
	}

	XYZEngine::GameObject* Enemy::GetGameObject()
	{
		return gameObject;
	}
}
