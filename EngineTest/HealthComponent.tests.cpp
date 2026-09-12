#include "pch.h"
#include "GameWorld.h"
#include "HealthComponent.h"

using RoguelikeGame::DamageInfo;
using RoguelikeGame::DamageKind;
using RoguelikeGame::DamageSource;
using RoguelikeGame::DeathInfo;
using RoguelikeGame::Faction;
using RoguelikeGame::HealthComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	class HealthComponentTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		HealthComponent* CreateHealth(const std::string& name, float maxHealth, float armor = 0.f)
		{
			GameObject* object = GameWorld::Instance()->CreateGameObject(name);
			auto health = object->AddComponent<HealthComponent>();
			health->SetMaxHealth(maxHealth);
			health->SetArmor(armor);
			return health;
		}

		DamageSource MakeSource(const std::string& attackerName)
		{
			DamageSource source;
			source.kind = DamageKind::Bullet;
			source.attackerId = 7;
			source.attackerName = attackerName;
			source.attackerFaction = Faction::Enemy;
			source.position = {10.f, 20.f};
			source.direction = {1.f, 0.f};
			return source;
		}
	};
}

TEST_F(HealthComponentTest, DamageEventCarriesAmountBeforeAndAfterArmor)
{
	HealthComponent* health = CreateHealth("Target", 100.f, 5.f);
	DamageInfo taken;
	health->SubscribeDamage([&taken](const DamageInfo& info) { taken = info; });

	health->TakeDamage(20.f);

	EXPECT_FLOAT_EQ(taken.amount, 15.f);
	EXPECT_FLOAT_EQ(taken.rawAmount, 20.f);
	EXPECT_FALSE(taken.isLethal);
}

TEST_F(HealthComponentTest, DamageEventCarriesSource)
{
	HealthComponent* health = CreateHealth("Target", 100.f);
	DamageInfo taken;
	health->SubscribeDamage([&taken](const DamageInfo& info) { taken = info; });

	health->TakeDamage(10.f, MakeSource("Raider"));

	EXPECT_EQ(taken.source.kind, DamageKind::Bullet);
	EXPECT_EQ(taken.source.attackerName, "Raider");
	EXPECT_EQ(taken.source.attackerFaction, Faction::Enemy);
	EXPECT_FLOAT_EQ(taken.source.position.x, 10.f);
	EXPECT_FLOAT_EQ(taken.source.position.y, 20.f);
}

TEST_F(HealthComponentTest, DamageWithoutSourceStaysUnknown)
{
	HealthComponent* health = CreateHealth("Target", 100.f);
	DamageInfo taken;
	health->SubscribeDamage([&taken](const DamageInfo& info) { taken = info; });

	health->TakeDamage(10.f);

	EXPECT_EQ(taken.source.kind, DamageKind::Unknown);
	EXPECT_EQ(taken.source.attackerId, XYZEngine::NO_GAME_OBJECT);
	EXPECT_TRUE(taken.source.attackerName.empty());
}

TEST_F(HealthComponentTest, DeathEventCarriesPositionAndKiller)
{
	HealthComponent* health = CreateHealth("Target", 30.f);
	health->GetGameObject()->GetTransform()->SetWorldPosition({64.f, 128.f});
	DeathInfo death;
	health->SubscribeDeath([&death](const DeathInfo& info) { death = info; });

	health->TakeDamage(1000.f, MakeSource("Raider"));

	EXPECT_FLOAT_EQ(death.position.x, 64.f);
	EXPECT_FLOAT_EQ(death.position.y, 128.f);
	EXPECT_EQ(death.source.attackerName, "Raider");
	EXPECT_EQ(death.source.kind, DamageKind::Bullet);
}

TEST_F(HealthComponentTest, LethalHitIsMarkedAndDeathFiresOnce)
{
	HealthComponent* health = CreateHealth("Target", 30.f);
	int damageCalls = 0;
	int deathCalls = 0;
	bool lastWasLethal = false;
	health->SubscribeDamage([&damageCalls, &lastWasLethal](const DamageInfo& info)
	{
		damageCalls++;
		lastWasLethal = info.isLethal;
	});
	health->SubscribeDeath([&deathCalls](const DeathInfo&) { deathCalls++; });

	health->TakeDamage(10.f);
	EXPECT_FALSE(lastWasLethal);

	health->TakeDamage(100.f);
	EXPECT_TRUE(lastWasLethal);

	health->TakeDamage(100.f);

	EXPECT_EQ(damageCalls, 2);
	EXPECT_EQ(deathCalls, 1);
}

TEST_F(HealthComponentTest, DamageEventComesBeforeDeathEvent)
{
	HealthComponent* health = CreateHealth("Target", 10.f);
	std::vector<std::string> order;
	health->SubscribeDamage([&order](const DamageInfo&) { order.push_back("damage"); });
	health->SubscribeDeath([&order](const DeathInfo&) { order.push_back("death"); });

	health->TakeDamage(50.f);

	ASSERT_EQ(order.size(), 2u);
	EXPECT_EQ(order[0], "damage");
	EXPECT_EQ(order[1], "death");
}

TEST_F(HealthComponentTest, InvulnerableTargetPublishesNothing)
{
	HealthComponent* health = CreateHealth("Target", 100.f);
	health->SetInvulnerable(true);
	int calls = 0;
	health->SubscribeDamage([&calls](const DamageInfo&) { calls++; });

	health->TakeDamage(50.f);

	EXPECT_EQ(calls, 0);
	EXPECT_FLOAT_EQ(health->GetHealth(), 100.f);
}

TEST_F(HealthComponentTest, HealPublishesRestoredAmountNotRequested)
{
	HealthComponent* health = CreateHealth("Target", 100.f);
	health->TakeDamage(30.f);
	float restored = 0.f;
	int calls = 0;
	health->SubscribeHeal([&restored, &calls](float amount)
	{
		restored = amount;
		calls++;
	});

	float returned = health->Heal(50.f);

	EXPECT_EQ(calls, 1);
	EXPECT_FLOAT_EQ(restored, 30.f);
	EXPECT_FLOAT_EQ(returned, 30.f);
	EXPECT_FLOAT_EQ(health->GetHealth(), 100.f);
}

TEST_F(HealthComponentTest, HealAtFullHealthPublishesNothing)
{
	HealthComponent* health = CreateHealth("Target", 100.f);
	int calls = 0;
	health->SubscribeHeal([&calls](float) { calls++; });

	float returned = health->Heal(25.f);

	EXPECT_EQ(calls, 0);
	EXPECT_FLOAT_EQ(returned, 0.f);
}

TEST_F(HealthComponentTest, HealDoesNotReviveTheDead)
{
	HealthComponent* health = CreateHealth("Target", 20.f);
	health->TakeDamage(1000.f);
	int calls = 0;
	health->SubscribeHeal([&calls](float) { calls++; });

	float returned = health->Heal(50.f);

	EXPECT_EQ(calls, 0);
	EXPECT_FLOAT_EQ(returned, 0.f);
	EXPECT_FALSE(health->IsAlive());
}

TEST_F(HealthComponentTest, SetMaxHealthDoesNotLookLikeHealing)
{
	HealthComponent* health = CreateHealth("Target", 100.f);
	health->TakeDamage(40.f);
	int calls = 0;
	health->SubscribeHeal([&calls](float) { calls++; });

	health->SetMaxHealth(200.f);

	EXPECT_EQ(calls, 0);
}

TEST_F(HealthComponentTest, UnsubscribeStopsEvents)
{
	HealthComponent* health = CreateHealth("Target", 100.f);
	int damageCalls = 0;
	int healCalls = 0;
	XYZEngine::SubscriptionId damageId = health->SubscribeDamage([&damageCalls](const DamageInfo&) { damageCalls++; });
	XYZEngine::SubscriptionId healId = health->SubscribeHeal([&healCalls](float) { healCalls++; });

	health->UnsubscribeDamage(damageId);
	health->UnsubscribeHeal(healId);
	health->TakeDamage(10.f);
	health->Heal(10.f);

	EXPECT_EQ(damageCalls, 0);
	EXPECT_EQ(healCalls, 0);
}
