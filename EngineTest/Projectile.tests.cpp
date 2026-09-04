#include "pch.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "ProjectileComponent.h"

using namespace XYZEngine;
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

		GameObject* CreateProjectile(float speed, float colliderSize, int& hits)
		{
			GameObject* projectile = GameWorld::Instance()->CreateGameObject("Projectile");

			auto collider = projectile->AddComponent<BoxColliderComponent>();
			collider->SetSize(colliderSize, colliderSize);
			collider->SetTrigger(true);

			auto component = projectile->AddComponent<ProjectileComponent>();
			component->SetDirection({1.f, 0.f});
			component->SetSpeed(speed);
			component->SetShooterName("Player");
			component->SetHitAction([&hits](const Vector2Df&, const Vector2Df&, bool) { hits++; });

			return projectile;
		}
	};
}

TEST_F(ProjectileTest, FastProjectileDoesNotSkipThinWall)
{
	CreateObstacle("Wall", 100.f, 16.f);
	int hits = 0;
	CreateProjectile(10000.f, 8.f, hits);

	GameWorld::Instance()->Update(0.05f);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(hits, 1);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Projectile"), nullptr);
}

TEST_F(ProjectileTest, ProjectileFliesThroughItsShooter)
{
	CreateObstacle("Player", 100.f, 16.f);
	int hits = 0;
	GameObject* projectile = CreateProjectile(10000.f, 8.f, hits);

	GameWorld::Instance()->Update(0.05f);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(hits, 0);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Projectile"), projectile);
	EXPECT_GT(projectile->GetTransform()->GetWorldPosition().x, 100.f);
}
