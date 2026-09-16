#include "pch.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "MovementComponent.h"
#include "BossBrainComponent.h"
#include "BossCatalog.h"
#include "ChaseComponent.h"
#include "FactionComponent.h"
#include "HealthComponent.h"
#include "DamageInfo.h"

using RoguelikeGame::BossAbility;
using RoguelikeGame::BossBrainComponent;
using RoguelikeGame::BossState;
using RoguelikeGame::ChaseComponent;
using RoguelikeGame::EnemyConfig;
using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::FindBoss;
using RoguelikeGame::HealthComponent;
using RoguelikeGame::TileType;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::MovementComponent;
using XYZEngine::Vector2Df;

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
			config.attackDamage = 20.f;
			config.projectileSpeed = 900.f;
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		EnemyConfig config{};
		GameObject* boss = nullptr;
		GameObject* player = nullptr;
		BossBrainComponent* brain = nullptr;
		ChaseComponent* chase = nullptr;
		HealthComponent* health = nullptr;

		void CreateBoss(const char* bossId = "puppeteer")
		{
			boss = GameWorld::Instance()->CreateGameObject("Boss");
			boss->GetTransform()->SetWorldPosition({0.f, 0.f});
			boss->AddComponent<FactionComponent>()->SetFaction(Faction::Enemy);
			boss->AddComponent<MovementComponent>()->SetSpeed(config.speed);

			health = boss->AddComponent<HealthComponent>();
			health->SetMaxHealth(config.maxHealth);

			chase = boss->AddComponent<ChaseComponent>();
			chase->SetTargetName("Player");
			chase->SetDetectionRadius(config.detectionRadius);
			chase->SetStopDistance(config.stopDistance);

			brain = boss->AddComponent<BossBrainComponent>();
			brain->SetDefinition(FindBoss(bossId));
			brain->SetConfig(config);
			brain->SetTargetName("Player");
		}

		void CreatePlayer(float x, float y)
		{
			player = GameWorld::Instance()->CreateGameObject("Player");
			player->GetTransform()->SetWorldPosition({x, y});
			player->AddComponent<FactionComponent>()->SetFaction(Faction::Player);
			player->AddComponent<HealthComponent>()->SetMaxHealth(100.f);

			auto collider = player->AddComponent<XYZEngine::BoxColliderComponent>();
			collider->SetSize(30.f, 30.f);
		}

		GameObject* CreateMinion(int index)
		{
			GameObject* minion = GameWorld::Instance()->CreateGameObject("Minion" + std::to_string(index));
			minion->AddComponent<HealthComponent>()->SetMaxHealth(50.f);

			return minion;
		}

		void MovePlayerTo(float x, float y) { player->GetTransform()->SetWorldPosition({x, y}); }

		float PlayerHealth() const { return player->GetComponent<HealthComponent>()->GetHealth(); }

		float BossX() const { return boss->GetTransform()->GetWorldPosition().x; }

		void Step(int times = 1)
		{
			Step(times, STEP);
		}

		void Step(int times, float delta)
		{
			for (int frame = 0; frame < times; frame++)
			{
				GameWorld::Instance()->Update(delta);
			}
		}

		void StepUntil(BossState wanted, int limit = 60)
		{
			for (int frame = 0; frame < limit && brain->GetState() != wanted; frame++)
			{
				Step();
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

TEST_F(BossBrainTest, BossUsesItsFirstAbility)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);

	Step(2);

	EXPECT_EQ(brain->GetState(), BossState::Attack);
	EXPECT_EQ(brain->GetCurrentAbility(), BossAbility::Summon);
	EXPECT_FALSE(chase->IsEnabled());
}

TEST_F(BossBrainTest, BossChasesAgainWhileAbilitiesRecharge)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);
	StepUntil(BossState::Cooldown);
	ASSERT_EQ(brain->GetState(), BossState::Cooldown);

	StepUntil(BossState::Chase);

	EXPECT_EQ(brain->GetState(), BossState::Chase);
	EXPECT_TRUE(chase->IsEnabled());
}

TEST_F(BossBrainTest, BasicAttackIsUsedWhenNoAbilityFits)
{
	CreateBoss("colossus");
	CreatePlayer(120.f, 0.f);

	Step(2);

	EXPECT_EQ(brain->GetState(), BossState::Attack);
	EXPECT_EQ(brain->GetCurrentAbility(), BossAbility::Basic);
}

TEST_F(BossBrainTest, VolleyFiresDeterministicFan)
{
	CreateBoss("colossus");
	CreatePlayer(250.f, 0.f);

	std::vector<Vector2Df> shots;
	brain->SubscribeShot([&shots](const Vector2Df&, const Vector2Df& direction, float, float) { shots.push_back(direction); });

	Step(2);
	ASSERT_EQ(brain->GetCurrentAbility(), BossAbility::Volley);
	EXPECT_TRUE(shots.empty());

	Step(7);

	ASSERT_EQ(shots.size(), 7u);
	EXPECT_NEAR(shots[3].x, 1.f, 0.001f);
	EXPECT_NEAR(shots[3].y, 0.f, 0.001f);
	EXPECT_NEAR(shots[0].y, -shots[6].y, 0.001f);
	EXPECT_LT(shots[0].y, 0.f);
	EXPECT_GT(shots[6].y, 0.f);
}

TEST_F(BossBrainTest, VolleyDamageFollowsTheAbilityScale)
{
	CreateBoss("colossus");
	CreatePlayer(250.f, 0.f);

	float damage = 0.f;
	brain->SubscribeShot([&damage](const Vector2Df&, const Vector2Df&, float shotDamage, float) { damage = shotDamage; });

	Step(9);

	EXPECT_FLOAT_EQ(damage, config.attackDamage * 0.8f);
}

TEST_F(BossBrainTest, SummonAsksForMinionsAroundTheBoss)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);

	std::vector<Vector2Df> points;
	brain->SubscribeSummon([&points](const Vector2Df& point, TileType) { points.push_back(point); });

	Step(11);

	ASSERT_EQ(points.size(), 3u);
	for (const Vector2Df& point : points)
	{
		EXPECT_NEAR(point.GetLength(), 90.f, 0.5f);
	}
}

TEST_F(BossBrainTest, MinionLimitStopsTheSummon)
{
	CreateBoss("puppeteer");
	CreatePlayer(480.f, 0.f);

	for (int index = 0; index < RoguelikeGame::BOSS_MINION_LIMIT; index++)
	{
		brain->RegisterMinion(CreateMinion(index));
	}

	Step(4);

	EXPECT_EQ(brain->GetState(), BossState::Chase);
	EXPECT_EQ(brain->GetAbilityUses(BossAbility::Summon), 0);
}

TEST_F(BossBrainTest, SpawnedMinionsAreReported)
{
	CreateBoss("puppeteer");

	std::vector<GameObject*> reported;
	brain->SubscribeMinionSpawned([&reported](GameObject* minion) { reported.push_back(minion); });

	GameObject* minion = CreateMinion(0);
	brain->RegisterMinion(minion);
	brain->RegisterMinion(nullptr);

	ASSERT_EQ(reported.size(), 1u);
	EXPECT_EQ(reported[0], minion);
}

TEST_F(BossBrainTest, BlastThrowsAMarkFromTheBoss)
{
	CreateBoss("gravedigger");
	CreatePlayer(300.f, 0.f);

	std::vector<Vector2Df> marks;
	float fuse = 0.f;
	float markRadius = 0.f;
	brain->SubscribeCastMark([&](const Vector2Df& point, float radius, float fuseTime)
	{
		marks.push_back(point);
		markRadius = radius;
		fuse = fuseTime;
	});

	Step(2);
	ASSERT_EQ(brain->GetCurrentAbility(), BossAbility::Blast);
	EXPECT_TRUE(marks.empty());

	Step(7);

	ASSERT_EQ(marks.size(), 1u);
	EXPECT_FLOAT_EQ(marks[0].x, 0.f);
	EXPECT_FLOAT_EQ(markRadius, 190.f);
	EXPECT_GT(fuse, 0.f);
}

TEST_F(BossBrainTest, ThrowingAMarkDoesNotHurtAnybody)
{
	CreateBoss("gravedigger");
	CreatePlayer(150.f, 0.f);

	Step(12);

	EXPECT_FLOAT_EQ(PlayerHealth(), 100.f);
}

TEST_F(BossBrainTest, DetonationHurtsThePlayerUnderIt)
{
	CreateBoss("gravedigger");
	CreatePlayer(150.f, 0.f);
	Step(2);

	int blasts = 0;
	brain->SubscribeBlast([&blasts](const Vector2Df&, float) { blasts++; });

	brain->DetonateBlast({150.f, 0.f});

	EXPECT_EQ(blasts, 1);
	EXPECT_FLOAT_EQ(PlayerHealth(), 100.f - config.attackDamage * 1.5f);
}

TEST_F(BossBrainTest, DetonationSparesThePlayerWhoLeft)
{
	CreateBoss("gravedigger");
	CreatePlayer(150.f, 0.f);
	Step(2);

	brain->DetonateBlast({1500.f, 0.f});

	EXPECT_FLOAT_EQ(PlayerHealth(), 100.f);
}

TEST_F(BossBrainTest, DeadBossDoesNotDetonate)
{
	CreateBoss("gravedigger");
	CreatePlayer(150.f, 0.f);
	Step(2);
	health->TakeDamage(config.maxHealth);
	Step();

	brain->DetonateBlast({150.f, 0.f});

	EXPECT_FLOAT_EQ(PlayerHealth(), 100.f);
}

TEST_F(BossBrainTest, SelfCenteredAbilitiesDoNotShowAMark)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);

	int marks = 0;
	brain->SubscribeCastMark([&marks](const Vector2Df&, float, float) { marks++; });

	Step(12);

	ASSERT_GE(brain->GetAbilityUses(BossAbility::Summon), 1);
	EXPECT_EQ(marks, 0);
}

TEST_F(BossBrainTest, DashCarriesTheBossTowardsThePlayer)
{
	CreateBoss("colossus");
	CreatePlayer(400.f, 0.f);

	Step(2);
	ASSERT_EQ(brain->GetCurrentAbility(), BossAbility::Dash);
	float start = BossX();

	Step(10);

	EXPECT_GT(BossX(), start + 80.f);
}

TEST_F(BossBrainTest, DashEndsWithASlamAndRestoresSpeed)
{
	CreateBoss("colossus");
	CreatePlayer(265.f, 0.f);

	int blasts = 0;
	brain->SubscribeBlast([&blasts](const Vector2Df&, float) { blasts++; });

	Step(2);
	ASSERT_EQ(brain->GetCurrentAbility(), BossAbility::Dash);

	Step(20, 0.05f);

	EXPECT_EQ(brain->GetState(), BossState::Cooldown);
	EXPECT_EQ(blasts, 1);
	EXPECT_LT(PlayerHealth(), 100.f);
	EXPECT_FLOAT_EQ(boss->GetComponent<MovementComponent>()->GetSpeed(), config.speed);
}

TEST_F(BossBrainTest, EnrageStartsBelowTheThreshold)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);

	int rages = 0;
	brain->SubscribeEnraged([&rages]() { rages++; });

	Step(2);
	health->TakeDamage(config.maxHealth * 0.7f);
	Step();

	EXPECT_EQ(brain->GetState(), BossState::Enraged);
	EXPECT_TRUE(brain->IsEnraged());
	EXPECT_EQ(rages, 1);
	EXPECT_FLOAT_EQ(boss->GetComponent<MovementComponent>()->GetSpeed(), config.speed * 1.35f);
}

TEST_F(BossBrainTest, EnrageHappensOnlyOnce)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);

	int rages = 0;
	brain->SubscribeEnraged([&rages]() { rages++; });

	Step(2);
	health->TakeDamage(config.maxHealth * 0.7f);
	Step(20);
	health->TakeDamage(config.maxHealth * 0.1f);
	Step(20);

	EXPECT_EQ(rages, 1);
}

TEST_F(BossBrainTest, EnrageEndsInChase)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);
	Step(2);
	health->TakeDamage(config.maxHealth * 0.7f);
	Step();
	ASSERT_EQ(brain->GetState(), BossState::Enraged);

	Step(11);

	EXPECT_NE(brain->GetState(), BossState::Enraged);
}

TEST_F(BossBrainTest, EnragedBossHitsHarder)
{
	CreateBoss("gravedigger");
	CreatePlayer(2000.f, 0.f);
	Step(2);
	health->TakeDamage(config.maxHealth * 0.7f);
	Step();
	ASSERT_TRUE(brain->IsEnraged());

	MovePlayerTo(150.f, 0.f);
	Step(2);
	brain->DetonateBlast({150.f, 0.f});

	EXPECT_FLOAT_EQ(PlayerHealth(), 100.f - config.attackDamage * 1.5f * 1.25f);
}

TEST_F(BossBrainTest, RageDoesNotShortenTheWindup)
{
	CreateBoss("gravedigger");
	CreatePlayer(2000.f, 0.f);

	int steps = 0;
	bool isThrown = false;
	brain->SubscribeCastMark([&isThrown](const Vector2Df&, float, float) { isThrown = true; });

	Step(2);
	health->TakeDamage(config.maxHealth * 0.7f);
	Step();
	ASSERT_TRUE(brain->IsEnraged());

	MovePlayerTo(300.f, 0.f);
	StepUntil(BossState::Attack);
	ASSERT_EQ(brain->GetCurrentAbility(), BossAbility::Blast);

	while (!isThrown && steps < 20)
	{
		Step();
		steps++;
	}

	EXPECT_TRUE(isThrown);
	EXPECT_GE(steps, 6);
}

TEST_F(BossBrainTest, RageShortensTheRecovery)
{
	CreateBoss("gravedigger");
	CreatePlayer(2000.f, 0.f);

	Step(2);
	health->TakeDamage(config.maxHealth * 0.7f);
	Step();
	ASSERT_TRUE(brain->IsEnraged());

	MovePlayerTo(150.f, 0.f);
	StepUntil(BossState::Cooldown);
	ASSERT_EQ(brain->GetState(), BossState::Cooldown);

	Step(7);

	EXPECT_NE(brain->GetState(), BossState::Cooldown);
}

TEST_F(BossBrainTest, BothAbilitiesAreUsedInOneFight)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);

	Step(30);
	MovePlayerTo(150.f, 0.f);
	Step(60);

	EXPECT_GE(brain->GetAbilityUses(BossAbility::Summon), 1);
	EXPECT_GE(brain->GetAbilityUses(BossAbility::Blast), 1);
}

TEST_F(BossBrainTest, AbilityWaitsForItsCooldown)
{
	CreateBoss("gravedigger");
	CreatePlayer(150.f, 0.f);

	Step(40);

	EXPECT_EQ(brain->GetAbilityUses(BossAbility::Blast), 1);
}

TEST_F(BossBrainTest, LostPlayerDoesNotEndTheFight)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);
	Step(2);

	MovePlayerTo(3000.f, 0.f);
	Step(30);

	EXPECT_NE(brain->GetState(), BossState::Idle);
	EXPECT_GT(boss->GetComponent<MovementComponent>()->GetDirection().GetLengthSquared(), 0.f);
}

TEST_F(BossBrainTest, BossThatNeverSawThePlayerStaysIdle)
{
	CreateBoss("puppeteer");
	CreatePlayer(3000.f, 0.f);
	Step(10);

	EXPECT_EQ(brain->GetState(), BossState::Idle);
}

TEST_F(BossBrainTest, DamageFromOutsideTheRadiusStartsTheFight)
{
	CreateBoss("puppeteer");
	CreatePlayer(3000.f, 0.f);
	Step(2);
	ASSERT_EQ(brain->GetState(), BossState::Idle);

	RoguelikeGame::DamageSource source;
	source.attackerName = "Player";
	source.attackerFaction = Faction::Player;
	boss->GetComponent<HealthComponent>()->TakeDamage(10.f, source);
	Step(2);

	EXPECT_NE(brain->GetState(), BossState::Idle);
}

TEST_F(BossBrainTest, DeadPlayerIsNotChased)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);
	Step(2);

	player->GetComponent<HealthComponent>()->TakeDamage(999.f);
	StepUntil(BossState::Idle);

	EXPECT_EQ(brain->GetState(), BossState::Idle);
}

TEST_F(BossBrainTest, DeathWinsOverAnyState)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);
	Step(2);
	ASSERT_EQ(brain->GetState(), BossState::Attack);

	health->TakeDamage(config.maxHealth);
	Step();

	EXPECT_EQ(brain->GetState(), BossState::Death);
	EXPECT_FALSE(chase->IsEnabled());
}

TEST_F(BossBrainTest, DeadBossStaysDead)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);
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
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);

	std::vector<BossState> states;
	brain->SubscribeStateChanged([&states](BossState, BossState next) { states.push_back(next); });

	Step(30);

	ASSERT_GE(states.size(), 4u);
	EXPECT_EQ(states[0], BossState::Chase);
	EXPECT_EQ(states[1], BossState::Attack);
	EXPECT_EQ(states[2], BossState::Cooldown);
	EXPECT_EQ(states[3], BossState::Chase);
}

TEST_F(BossBrainTest, AttackKeepsTheBossInPlace)
{
	CreateBoss("puppeteer");
	CreatePlayer(400.f, 0.f);
	Step(2);
	ASSERT_EQ(brain->GetCurrentAbility(), BossAbility::Summon);

	Vector2Df before = boss->GetTransform()->GetWorldPosition();
	Step(8);

	EXPECT_FLOAT_EQ(boss->GetTransform()->GetWorldPosition().x, before.x);
	EXPECT_FLOAT_EQ(boss->GetTransform()->GetWorldPosition().y, before.y);
}
