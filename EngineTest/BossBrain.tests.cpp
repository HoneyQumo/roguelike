#include "pch.h"
#include "GameWorld.h"
#include "MovementComponent.h"
#include "BossBrainComponent.h"
#include "BossCatalog.h"
#include "ChaseComponent.h"
#include "HealthComponent.h"

using RoguelikeGame::BossAbility;
using RoguelikeGame::BossBrainComponent;
using RoguelikeGame::BossState;
using RoguelikeGame::ChaseComponent;
using RoguelikeGame::EnemyConfig;
using RoguelikeGame::FindBoss;
using RoguelikeGame::HealthComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::MovementComponent;

namespace
{
	constexpr float STEP = 0.1f;

	class BossBrainTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			config = EnemyConfig{};
			config.objectName = "Boss";
			config.speed = 90.f;
			config.detectionRadius = 500.f;
			config.stopDistance = 170.f;
			config.maxHealth = 600.f;
			config.attackRange = 260.f;
			config.attackDamage = 24.f;
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		EnemyConfig config{};
		GameObject* boss = nullptr;
		GameObject* player = nullptr;
		BossBrainComponent* brain = nullptr;
		ChaseComponent* chase = nullptr;
		HealthComponent* health = nullptr;

		void CreateBoss()
		{
			boss = GameWorld::Instance()->CreateGameObject("Boss");
			boss->GetTransform()->SetWorldPosition({0.f, 0.f});
			boss->AddComponent<MovementComponent>()->SetSpeed(config.speed);

			health = boss->AddComponent<HealthComponent>();
			health->SetMaxHealth(config.maxHealth);

			chase = boss->AddComponent<ChaseComponent>();
			chase->SetTargetName("Player");
			chase->SetDetectionRadius(config.detectionRadius);
			chase->SetStopDistance(config.stopDistance);

			brain = boss->AddComponent<BossBrainComponent>();
			brain->SetDefinition(FindBoss("puppeteer"));
			brain->SetConfig(config);
			brain->SetTargetName("Player");
		}

		void CreatePlayer(float x, float y)
		{
			player = GameWorld::Instance()->CreateGameObject("Player");
			player->GetTransform()->SetWorldPosition({x, y});
			player->AddComponent<HealthComponent>()->SetMaxHealth(100.f);
		}

		void MovePlayerTo(float x, float y) { player->GetTransform()->SetWorldPosition({x, y}); }

		void Step(int times = 1)
		{
			for (int frame = 0; frame < times; frame++)
			{
				GameWorld::Instance()->Update(STEP);
			}
		}
	};
}

TEST_F(BossBrainTest, BossWaitsWithoutTarget)
{
	CreateBoss();

	Step(3);

	EXPECT_EQ(brain->GetState(), BossState::Idle);
	EXPECT_FALSE(chase->IsEnabled());
}

TEST_F(BossBrainTest, DistantPlayerIsNotDetected)
{
	CreateBoss();
	CreatePlayer(2000.f, 0.f);

	Step(2);

	EXPECT_EQ(brain->GetState(), BossState::Idle);
}

TEST_F(BossBrainTest, BossChasesTheDetectedPlayer)
{
	CreateBoss();
	CreatePlayer(400.f, 0.f);

	Step(2);

	EXPECT_EQ(brain->GetState(), BossState::Chase);
	EXPECT_TRUE(chase->IsEnabled());
}

TEST_F(BossBrainTest, BossAttacksInsideAttackRange)
{
	CreateBoss();
	CreatePlayer(200.f, 0.f);

	Step(3);

	EXPECT_EQ(brain->GetState(), BossState::Attack);
	EXPECT_EQ(brain->GetCurrentAbility(), BossAbility::Basic);
	EXPECT_FALSE(chase->IsEnabled());
}

TEST_F(BossBrainTest, AttackIsFollowedByCooldown)
{
	CreateBoss();
	CreatePlayer(200.f, 0.f);
	Step(3);
	ASSERT_EQ(brain->GetState(), BossState::Attack);

	Step(13);

	EXPECT_EQ(brain->GetState(), BossState::Cooldown);
	EXPECT_EQ(brain->GetCurrentAbility(), BossAbility::None);
}

TEST_F(BossBrainTest, CycleRepeatsWhileThePlayerStaysClose)
{
	CreateBoss();
	CreatePlayer(200.f, 0.f);

	std::vector<BossState> states;
	brain->SubscribeStateChanged([&states](BossState, BossState next) { states.push_back(next); });

	Step(30);

	ASSERT_GE(states.size(), 5u);
	EXPECT_EQ(states[0], BossState::Chase);
	EXPECT_EQ(states[1], BossState::Attack);
	EXPECT_EQ(states[2], BossState::Cooldown);
	EXPECT_EQ(states[3], BossState::Chase);
	EXPECT_EQ(states[4], BossState::Attack);
}

TEST_F(BossBrainTest, BossRestsBetweenAttacks)
{
	CreateBoss();
	CreatePlayer(200.f, 0.f);
	Step(3);
	ASSERT_EQ(brain->GetState(), BossState::Attack);

	int attacks = 0;
	brain->SubscribeAbilityUsed([&attacks](BossAbility) { attacks++; });

	Step(21);

	EXPECT_EQ(attacks, 1);
}

TEST_F(BossBrainTest, LostPlayerReturnsBossToIdle)
{
	CreateBoss();
	CreatePlayer(400.f, 0.f);
	Step(2);
	ASSERT_EQ(brain->GetState(), BossState::Chase);

	MovePlayerTo(3000.f, 0.f);
	Step(2);

	EXPECT_EQ(brain->GetState(), BossState::Idle);
}

TEST_F(BossBrainTest, DeadPlayerIsNotChased)
{
	CreateBoss();
	CreatePlayer(200.f, 0.f);
	Step(2);

	player->GetComponent<HealthComponent>()->TakeDamage(999.f);
	Step(30);

	EXPECT_EQ(brain->GetState(), BossState::Idle);
}

TEST_F(BossBrainTest, DeathWinsOverAnyState)
{
	CreateBoss();
	CreatePlayer(200.f, 0.f);
	Step(3);
	ASSERT_EQ(brain->GetState(), BossState::Attack);

	health->TakeDamage(config.maxHealth);
	Step();

	EXPECT_EQ(brain->GetState(), BossState::Death);
	EXPECT_FALSE(chase->IsEnabled());
}

TEST_F(BossBrainTest, DeadBossStaysDead)
{
	CreateBoss();
	CreatePlayer(200.f, 0.f);
	Step(2);
	health->TakeDamage(config.maxHealth);
	Step();
	ASSERT_EQ(brain->GetState(), BossState::Death);

	health->Heal(config.maxHealth);
	Step(5);

	EXPECT_EQ(brain->GetState(), BossState::Death);
}

TEST_F(BossBrainTest, StateChangesAreReported)
{
	CreateBoss();
	CreatePlayer(200.f, 0.f);

	std::vector<BossState> states;
	brain->SubscribeStateChanged([&states](BossState, BossState next) { states.push_back(next); });

	Step(25);

	ASSERT_GE(states.size(), 3u);
	EXPECT_EQ(states[0], BossState::Chase);
	EXPECT_EQ(states[1], BossState::Attack);
	EXPECT_EQ(states[2], BossState::Cooldown);
}

TEST_F(BossBrainTest, BossIsNotEnragedInTheFirstStage)
{
	CreateBoss();
	CreatePlayer(200.f, 0.f);
	Step(2);

	health->TakeDamage(config.maxHealth * 0.9f);
	Step(5);

	EXPECT_FALSE(brain->IsEnraged());
	EXPECT_NE(brain->GetState(), BossState::Enraged);
}

TEST_F(BossBrainTest, StoppedBossDoesNotKeepMoving)
{
	CreateBoss();
	CreatePlayer(400.f, 0.f);
	Step(2);
	ASSERT_EQ(brain->GetState(), BossState::Chase);

	MovePlayerTo(200.f, 0.f);
	Step(2);
	ASSERT_EQ(brain->GetState(), BossState::Attack);

	XYZEngine::Vector2Df before = boss->GetTransform()->GetWorldPosition();
	Step(5);

	EXPECT_FLOAT_EQ(boss->GetTransform()->GetWorldPosition().x, before.x);
	EXPECT_FLOAT_EQ(boss->GetTransform()->GetWorldPosition().y, before.y);
}
