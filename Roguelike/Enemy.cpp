#include "Enemy.h"
#include "GameSettings.h"
#include "Projectile.h"
#include "EnemyAttackComponent.h"
#include <LoggerRegistry.h>
#include <ResourceSystem.h>
#include <ChaseComponent.h>
#include <MovementComponent.h>
#include <RigidbodyComponent.h>
#include <BoxColliderComponent.h>
#include <SpriteDirectionComponent.h>
#include <SpriteMovementAnimationComponent.h>
#include <HealthComponent.h>
#include <HealthBarComponent.h>
#include <WeaponComponent.h>
#include <AudioComponent.h>
#include <stdexcept>

namespace RoguelikeGame
{
	Enemy::Enemy(const EnemyConfig& config, const XYZEngine::Vector2Df& position)
	{
		gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(config.objectName);

		auto transform = gameObject->GetComponent<XYZEngine::TransformComponent>();
		transform->SetWorldPosition(position);

		auto texture = XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(config.textureMapName, config.idleFirstFrame);
		if (texture == nullptr)
		{
			XYZEngine::GameWorld::Instance()->DestroyGameObject(gameObject);
			throw std::runtime_error("enemy texture map is not loaded: " + config.textureMapName);
		}

		auto renderer = gameObject->AddComponent<XYZEngine::SpriteRendererComponent>();
		renderer->SetTexture(*texture);
		renderer->SetPixelSize(CHARACTER_SPRITE_SIZE, CHARACTER_SPRITE_SIZE);

		// Chase sets the direction, movement applies it: keep chase updated first.
		auto chase = gameObject->AddComponent<XYZEngine::ChaseComponent>();
		chase->SetTargetName("Player");
		chase->SetDetectionRadius(config.detectionRadius);
		chase->SetStopDistance(config.stopDistance);

		auto movement = gameObject->AddComponent<XYZEngine::MovementComponent>();
		movement->SetSpeed(config.speed);

		auto body = gameObject->AddComponent<XYZEngine::RigidbodyComponent>();
		body->SetKinematic(false);

		auto collider = gameObject->AddComponent<XYZEngine::BoxColliderComponent>();
		collider->SetSize(CHARACTER_COLLIDER_WIDTH, CHARACTER_COLLIDER_HEIGHT);
		collider->SetOffset(0.f, CHARACTER_COLLIDER_OFFSET_Y);

		auto direction = gameObject->AddComponent<XYZEngine::SpriteDirectionComponent>();

		auto animation = gameObject->AddComponent<XYZEngine::SpriteMovementAnimationComponent>();
		animation->SetWalkAnimation(config.textureMapName, config.walkFirstFrame, config.walkFrames, WALK_FRAMERATE);
		animation->SetIdleAnimation(config.textureMapName, config.idleFirstFrame, config.idleFrames, IDLE_FRAMERATE);
		animation->SetDeathAnimation(config.textureMapName, config.deathFirstFrame, config.deathFrames, DEATH_FRAMERATE);
		if (config.hurtFrames > 0)
		{
			animation->SetHurtAnimation(config.textureMapName, config.hurtFirstFrame, config.hurtFrames, HURT_FRAMERATE);
		}

		auto health = gameObject->AddComponent<XYZEngine::HealthComponent>();
		health->SetMaxHealth(config.maxHealth);
		health->SetArmor(config.armor);

		auto healthBar = gameObject->AddComponent<XYZEngine::HealthBarComponent>();
		healthBar->SetSize(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT);
		healthBar->SetOffset(0.f, HEALTH_BAR_OFFSET_Y);
		healthBar->SetColors({ 200, 60, 60 }, { 20, 20, 20, 200 });

		auto shotAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
		shotAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound("shot"));
		shotAudio->SetVolume(SHOT_VOLUME);

		auto hurtAudio = gameObject->AddComponent<XYZEngine::AudioComponent>();
		hurtAudio->SetSound(XYZEngine::ResourceSystem::Instance()->GetSound("hurt"));
		hurtAudio->SetVolume(HURT_VOLUME);

		auto weaponComponent = gameObject->AddComponent<XYZEngine::WeaponComponent>();
		weaponComponent->SetCooldown(config.attackCooldown);
		weaponComponent->SetDamage(config.attackDamage);
		weaponComponent->SetProjectileSpeed(config.projectileSpeed);
		weaponComponent->SetShotOffset(SHOT_OFFSET);

		std::string shooterName = config.objectName;
		weaponComponent->SetShotAction([shooterName, shotAudio](const XYZEngine::Vector2Df& shotPosition, const XYZEngine::Vector2Df& shotDirection, float damage, float speed)
			{
				Projectile::Spawn(shotPosition, shotDirection, damage, speed, shooterName);
				shotAudio->Play();
			});

		auto attack = gameObject->AddComponent<EnemyAttackComponent>();
		attack->SetTargetName("Player");
		attack->SetAttackRange(config.attackRange);

		if (!config.weaponTextureName.empty())
		{
			try
			{
				weapon = std::make_unique<Weapon>(gameObject, config.weaponTextureName);
				attack->SetWeapon(weapon->GetTransform(), weapon->GetRenderer());
			}
			catch (const std::exception& exception)
			{
				LOG_ERROR(std::string("Enemy weapon is not created: ") + exception.what());
			}
		}

		auto weaponObject = weapon == nullptr ? nullptr : weapon->GetGameObject();
		health->SubscribeDamage([animation, hurtAudio](float damage)
			{
				animation->PlayHurt();
				hurtAudio->Play();
			});
		health->SubscribeDeath([animation, movement, chase, collider, weaponObject]()
			{
				animation->PlayDeath();
				movement->SetSpeed(0.f);
				chase->SetDetectionRadius(0.f);
				collider->SetTrigger(true);

				if (weaponObject != nullptr)
				{
					XYZEngine::GameWorld::Instance()->DestroyGameObject(weaponObject);
				}
			});

		LOG_INFO(config.objectName + " created at " + std::to_string((int)position.x) + ";" + std::to_string((int)position.y));
	}

	XYZEngine::GameObject* Enemy::GetGameObject()
	{
		return gameObject;
	}
}
