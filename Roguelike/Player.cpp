#include "Player.h"
#include "GameSettings.h"
#include "Projectile.h"
#include "PlayerAttackComponent.h"
#include <WeaponComponent.h>
#include <AudioComponent.h>
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <stdexcept>
#include <MovementComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <SpriteDirectionComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include <HealthComponent.h>
#include <HealthBarComponent.h>

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

		animation->SetHurtAnimation("player", PLAYER_HURT_FIRST_FRAME, PLAYER_HURT_FRAMES, HURT_FRAMERATE);
		animation->SetDeathAnimation("player", PLAYER_DEATH_FIRST_FRAME, PLAYER_DEATH_FRAMES, DEATH_FRAMERATE);

		auto health = gameObject->AddComponent<XYZEngine::HealthComponent>();
		health->SetMaxHealth(PLAYER_MAX_HEALTH);
		health->SetArmor(PLAYER_ARMOR);

		auto shotAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
		shotAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound("shot"));
		shotAudio->SetVolume(SHOT_VOLUME);

		auto hurtAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
		hurtAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound("hurt"));
		hurtAudio->SetVolume(HURT_VOLUME);

		auto weaponComponent = gameObject->AddComponent<XYZEngine::WeaponComponent>();
		weaponComponent->SetCooldown(PLAYER_ATTACK_COOLDOWN);
		weaponComponent->SetDamage(PLAYER_ATTACK_DAMAGE);
		weaponComponent->SetProjectileSpeed(PLAYER_PROJECTILE_SPEED);
		weaponComponent->SetShotOffset(SHOT_OFFSET);
		weaponComponent->SetShotAction([shotAudio](const XYZEngine::Vector2Df& shotPosition, const XYZEngine::Vector2Df& shotDirection, float damage, float speed)
			{
				Projectile::Spawn(shotPosition, shotDirection, damage, speed, "Player");
				shotAudio->Play();
			});

		auto attack = gameObject->AddComponent<PlayerAttackComponent>();

		try
		{
			weapon = std::make_unique<Weapon>(gameObject, "player_weapon");
			attack->SetWeapon(weapon->GetTransform(), weapon->GetRenderer());
		}
		catch (const std::exception& exception)
		{
			LOG_ERROR(std::string("Player weapon is not created: ") + exception.what());
		}

		auto weaponObject = weapon == nullptr ? nullptr : weapon->GetGameObject();
		health->SubscribeDamage([animation, hurtAudio](float damage)
			{
				animation->PlayHurt();
				hurtAudio->Play();
			});
		health->SubscribeDeath([animation, movement, collider, weaponObject]()
			{
				animation->PlayDeath();
				movement->SetSpeed(0.f);
				collider->SetTrigger(true);

				if (weaponObject != nullptr)
				{
					XYZEngine::GameWorld::Instance()->DestroyGameObject(weaponObject);
				}

				LOG_WARN("Player is dead, controls are disabled");
			});

		auto healthBar = gameObject->AddComponent<XYZEngine::HealthBarComponent>();
		healthBar->SetSize(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT);
		healthBar->SetOffset(0.f, HEALTH_BAR_OFFSET_Y);
		healthBar->SetColors({ 90, 200, 90 }, { 20, 20, 20, 200 });

		LOG_INFO("Player created at " + std::to_string((int)position.x) + ";" + std::to_string((int)position.y));
	}

	XYZEngine::GameObject* Player::GetGameObject()
	{
		return gameObject;
	}
}
