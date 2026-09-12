#include "pch.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "ProjectileComponent.h"
#include "FactionComponent.h"

using namespace XYZEngine;
using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::ProjectileComponent;

namespace
{
	class ProjectileTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* CreateObstacle(const std::string& name, float x, float size)
		{
			GameObject* obstacle = GameWorld::Instance()->CreateGameObject(name);
			obstacle->GetTransform()->SetWorldPosition({x, 0.f});

			auto collider = obstacle->AddComponent<BoxColliderComponent>();
			collider->SetSize(size, size);

			return obstacle;
		}

		GameObject* CreateCharacter(const std::string& name, float x, float size, Faction faction)
		{
			GameObject* character = CreateObstacle(name, x, size);
			character->AddComponent<FactionComponent>()->SetFaction(faction);

			return character;
		}

		GameObject* CreateProjectile(float speed, float colliderSize, int& hits, GameObjectId shooterId,
									 Faction shooterFaction = Faction::Player)
		{
			GameObject* projectile = GameWorld::Instance()->CreateGameObject("Projectile");

			auto collider = projectile->AddComponent<BoxColliderComponent>();
			collider->SetSize(colliderSize, colliderSize);
			collider->SetTrigger(true);

			auto component = projectile->AddComponent<ProjectileComponent>();
			component->SetDirection({1.f, 0.f});
			component->SetSpeed(speed);
			component->SetShooter(shooterId, shooterFaction);
			component->SubscribeHit([&hits](const Vector2Df&, const Vector2Df&, bool) { hits++; });

			return projectile;
		}
	};
}

TEST_F(ProjectileTest, FastProjectileDoesNotSkipThinWall)
{
	CreateObstacle("Wall", 100.f, 16.f);
	int hits = 0;
	CreateProjectile(10000.f, 8.f, hits, NO_GAME_OBJECT);

	GameWorld::Instance()->Update(0.05f);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(hits, 1);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Projectile"), nullptr);
}

TEST_F(ProjectileTest, ProjectileFliesThroughItsShooter)
{
	GameObject* shooter = CreateObstacle("Shooter", 100.f, 16.f);
	int hits = 0;
	GameObject* projectile = CreateProjectile(10000.f, 8.f, hits, shooter->GetId());

	GameWorld::Instance()->Update(0.05f);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(hits, 0);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Projectile"), projectile);
	EXPECT_GT(projectile->GetTransform()->GetWorldPosition().x, 100.f);
}

TEST_F(ProjectileTest, ProjectileFliesThroughItsOwnFaction)
{
	CreateCharacter("Ally", 100.f, 16.f, Faction::Enemy);
	int hits = 0;
	GameObject* projectile = CreateProjectile(10000.f, 8.f, hits, NO_GAME_OBJECT, Faction::Enemy);

	GameWorld::Instance()->Update(0.05f);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(hits, 0);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Projectile"), projectile);
}

TEST_F(ProjectileTest, ProjectileHitsAnotherFaction)
{
	CreateCharacter("Enemy", 100.f, 16.f, Faction::Enemy);
	int hits = 0;
	CreateProjectile(10000.f, 8.f, hits, NO_GAME_OBJECT, Faction::Player);

	GameWorld::Instance()->Update(0.05f);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(hits, 1);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Projectile"), nullptr);
}

TEST_F(ProjectileTest, TwoEnemiesOfTheSameKindShareOneFactionButNotOneId)
{
	GameObject* first = CreateCharacter("Assault", 0.f, 16.f, Faction::Enemy);
	GameObject* second = CreateCharacter("Assault", 100.f, 16.f, Faction::Enemy);

	EXPECT_EQ(first->GetName(), second->GetName());
	EXPECT_NE(first->GetId(), second->GetId());
	EXPECT_NE(first->GetId(), NO_GAME_OBJECT);
}
