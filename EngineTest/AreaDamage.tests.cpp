#include "pch.h"
#include "AreaDamage.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "ExplosiveComponent.h"

using namespace XYZEngine;
using namespace RoguelikeGame;

namespace
{
	class AreaDamageTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* CreateCollider(const std::string& name, float x, float y, float size, bool isTrigger = false)
		{
			GameObject* object = GameWorld::Instance()->CreateGameObject(name);
			object->GetTransform()->SetWorldPosition({x, y});

			auto collider = object->AddComponent<BoxColliderComponent>();
			collider->SetSize(size, size);
			collider->SetTrigger(isTrigger);

			return object;
		}

		GameObject* CreateCharacter(const std::string& name, float x, float y)
		{
			GameObject* character = CreateCollider(name, x, y, 30.f);
			character->AddComponent<HealthComponent>()->SetMaxHealth(100.f);

			return character;
		}

		void UpdateBounds() { GameWorld::Instance()->Update(0.016f); }
	};
}

TEST_F(AreaDamageTest, LivingTargetsAreFoundWithDistanceAndDirection)
{
	CreateCharacter("Enemy", 50.f, 0.f);
	UpdateBounds();

	AreaQuery query = QueryDamageArea({0.f, 0.f}, 100.f, nullptr);

	ASSERT_EQ(query.targets.size(), 1u);
	EXPECT_EQ(query.targets[0].gameObject->GetName(), "Enemy");
	EXPECT_NE(query.targets[0].health, nullptr);
	EXPECT_FLOAT_EQ(query.targets[0].distance, 50.f);
	EXPECT_FLOAT_EQ(query.targets[0].direction.x, 1.f);
	EXPECT_FLOAT_EQ(query.targets[0].direction.y, 0.f);
}

TEST_F(AreaDamageTest, IgnoredObjectIsNotATarget)
{
	GameObject* self = CreateCharacter("Self", 0.f, 0.f);
	CreateCharacter("Enemy", 40.f, 0.f);
	UpdateBounds();

	AreaQuery query = QueryDamageArea({0.f, 0.f}, 100.f, self);

	ASSERT_EQ(query.targets.size(), 1u);
	EXPECT_EQ(query.targets[0].gameObject->GetName(), "Enemy");
}

TEST_F(AreaDamageTest, TargetBeyondRadiusIsSkipped)
{
	CreateCharacter("Far", 90.f, 0.f);
	UpdateBounds();

	AreaQuery query = QueryDamageArea({0.f, 0.f}, 50.f, nullptr);

	EXPECT_TRUE(query.targets.empty());
}

TEST_F(AreaDamageTest, DeadAndInvulnerableTargetsAreSkipped)
{
	CreateCharacter("Dead", 20.f, 0.f)->GetComponent<HealthComponent>()->TakeDamage(1000.f);
	CreateCharacter("Invulnerable", 30.f, 0.f)->GetComponent<HealthComponent>()->SetInvulnerable(true);
	UpdateBounds();

	AreaQuery query = QueryDamageArea({0.f, 0.f}, 100.f, nullptr);

	EXPECT_TRUE(query.targets.empty());
}

TEST_F(AreaDamageTest, SolidColliderWithoutHealthBecomesAnObstacle)
{
	CreateCollider("Wall", 30.f, 0.f, 64.f);
	CreateCollider("Bullet", 40.f, 0.f, 8.f, true);
	UpdateBounds();

	AreaQuery query = QueryDamageArea({0.f, 0.f}, 100.f, nullptr);

	EXPECT_TRUE(query.targets.empty());
	EXPECT_EQ(query.obstacles.size(), 1u);
}

TEST_F(AreaDamageTest, TargetWithTwoCollidersIsReportedOnce)
{
	GameObject* enemy = CreateCharacter("Enemy", 40.f, 0.f);
	enemy->AddComponent<BoxColliderComponent>()->SetSize(40.f, 40.f);
	UpdateBounds();

	AreaQuery query = QueryDamageArea({0.f, 0.f}, 100.f, nullptr);

	EXPECT_EQ(query.targets.size(), 1u);
}

TEST_F(AreaDamageTest, ExplosionFillsDamageSourceWithOwner)
{
	GameObject* bomb = GameWorld::Instance()->CreateGameObject("Rocket");
	bomb->GetTransform()->SetWorldPosition({0.f, 0.f});
	auto explosive = bomb->AddComponent<ExplosiveComponent>();
	explosive->SetRadius(100.f);
	explosive->SetCenterDamage(40.f);
	explosive->SetOwner(11, "Player", Faction::Player);

	GameObject* victim = CreateCharacter("Enemy", 50.f, 0.f);
	DamageInfo taken;
	victim->GetComponent<HealthComponent>()->SubscribeDamage([&taken](const DamageInfo& info) { taken = info; });
	UpdateBounds();

	int hits = explosive->Explode({0.f, 0.f});

	EXPECT_EQ(hits, 1);
	EXPECT_EQ(taken.source.kind, DamageKind::Explosion);
	EXPECT_EQ(taken.source.attackerId, 11u);
	EXPECT_EQ(taken.source.attackerName, "Player");
	EXPECT_EQ(taken.source.attackerFaction, Faction::Player);
	EXPECT_FLOAT_EQ(taken.source.position.x, 50.f);
}
