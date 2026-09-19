#include "pch.h"
#include "FactionComponent.h"
#include "HealthComponent.h"
#include "Noise.h"
#include "PropCatalog.h"
#include "TrapComponent.h"
#include "TreadComponent.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include <RigidbodyComponent.h>
#include <sstream>

using RoguelikeGame::ClearNoiseListeners;
using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::HealthComponent;
using RoguelikeGame::Noise;
using RoguelikeGame::PropCatalog;
using RoguelikeGame::SubscribeNoiseRaised;
using RoguelikeGame::TrapComponent;
using RoguelikeGame::TrapKind;
using RoguelikeGame::TreadComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	constexpr float TILE = 64.f;
	constexpr float AWAY = 10.f * TILE;
	constexpr float SPIKE_DAMAGE = 35.f;
	constexpr float WIRE_REACH = 640.f;

	class TrapTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			ClearNoiseListeners();
			heard = 0;
			SubscribeNoiseRaised([this](const Noise& noise) { heard++; lastNoise = noise; });
		}

		void TearDown() override
		{
			ClearNoiseListeners();
			GameWorld::Instance()->Clear();
		}

		TrapComponent* CreateTrap(TrapKind kind, float amount)
		{
			GameObject* object = GameWorld::Instance()->CreateGameObject("Trap");
			object->GetTransform()->SetWorldPosition({0.f, 0.f});

			auto pad = object->AddComponent<XYZEngine::BoxColliderComponent>();
			pad->SetSize(TILE, TILE);
			pad->SetTrigger(true);

			auto tread = object->AddComponent<TreadComponent>();
			tread->SetPad(pad);

			auto trap = object->AddComponent<TrapComponent>();
			trap->SetKind(kind);
			trap->SetAmount(amount);
			trap->SetTread(tread);

			return trap;
		}

		GameObject* CreateWalker(Faction faction)
		{
			GameObject* walker = GameWorld::Instance()->CreateGameObject("Walker");
			walker->GetTransform()->SetWorldPosition({AWAY, 0.f});
			walker->AddComponent<FactionComponent>()->SetFaction(faction);
			walker->AddComponent<XYZEngine::RigidbodyComponent>()->SetKinematic(false);
			walker->AddComponent<XYZEngine::BoxColliderComponent>()->SetSize(30.f, 30.f);

			auto health = walker->AddComponent<HealthComponent>();
			health->SetMaxHealth(100.f);

			Step();

			return walker;
		}

		void MoveTo(GameObject* walker, float x)
		{
			walker->GetTransform()->SetWorldPosition({x, 0.f});
			Step();
		}

		void Step()
		{
			GameWorld::Instance()->Update(0.016f);
			GameWorld::Instance()->UpdatePhysics();
		}

		int heard = 0;
		Noise lastNoise;
	};
}

TEST_F(TrapTest, SpikesHurtWhoeverStepsOnThem)
{
	TrapComponent* trap = CreateTrap(TrapKind::Hurt, SPIKE_DAMAGE);
	GameObject* player = CreateWalker(Faction::Player);

	MoveTo(player, 0.f);

	EXPECT_TRUE(trap->IsSprung());
	EXPECT_LT(player->GetComponent<HealthComponent>()->GetHealth(), 100.f);
}

// Ловушка не разбирает, кто наступил: охранника можно вывести на шипы,
// и это честно - иначе половина хитростей игрока не работала бы.
TEST_F(TrapTest, SpikesHurtGuardsToo)
{
	TrapComponent* trap = CreateTrap(TrapKind::Hurt, SPIKE_DAMAGE);
	GameObject* guard = CreateWalker(Faction::Enemy);

	MoveTo(guard, 0.f);

	EXPECT_TRUE(trap->IsSprung());
	EXPECT_LT(guard->GetComponent<HealthComponent>()->GetHealth(), 100.f);
}

// Растяжка не бьёт, а зовёт: для тихого пути это хуже удара.
TEST_F(TrapTest, TheWireCallsTheGuardInsteadOfHurting)
{
	TrapComponent* trap = CreateTrap(TrapKind::Alarm, WIRE_REACH);
	GameObject* player = CreateWalker(Faction::Player);

	MoveTo(player, 0.f);

	EXPECT_TRUE(trap->IsSprung());
	EXPECT_FLOAT_EQ(player->GetComponent<HealthComponent>()->GetHealth(), 100.f);
	EXPECT_EQ(heard, 1);
	EXPECT_FLOAT_EQ(lastNoise.radius, WIRE_REACH);
}

TEST_F(TrapTest, ATrapGoesOffOnce)
{
	TrapComponent* trap = CreateTrap(TrapKind::Hurt, SPIKE_DAMAGE);
	GameObject* player = CreateWalker(Faction::Player);

	MoveTo(player, 0.f);
	float afterFirst = player->GetComponent<HealthComponent>()->GetHealth();

	MoveTo(player, AWAY);
	MoveTo(player, 0.f);

	EXPECT_FLOAT_EQ(player->GetComponent<HealthComponent>()->GetHealth(), afterFirst);
	EXPECT_TRUE(trap->IsSprung());
}

TEST_F(TrapTest, ATrapWithoutAKindIsJustScenery)
{
	TrapComponent* trap = CreateTrap(TrapKind::None, SPIKE_DAMAGE);
	GameObject* player = CreateWalker(Faction::Player);

	MoveTo(player, 0.f);

	EXPECT_FALSE(trap->IsSprung());
	EXPECT_FLOAT_EQ(player->GetComponent<HealthComponent>()->GetHealth(), 100.f);
}

TEST(TrapCatalogTest, TheTrapLineReadsKindAndAmount)
{
	std::string text =
		"[prop spikes]\n"
		"name Шипы\n"
		"size 60\n"
		"trap hurt 35\n"
		"\n"
		"[prop wire]\n"
		"name Растяжка\n"
		"size 60\n"
		"trap alarm 640\n";
	std::istringstream input(text);

	PropCatalog catalog = PropCatalog::Parse(input, "traps");

	ASSERT_NE(catalog.Find("spikes"), nullptr);
	EXPECT_TRUE(catalog.Find("spikes")->IsTrap());
	EXPECT_EQ(catalog.Find("spikes")->trapKind, TrapKind::Hurt);
	EXPECT_FLOAT_EQ(catalog.Find("spikes")->trapAmount, 35.f);

	ASSERT_NE(catalog.Find("wire"), nullptr);
	EXPECT_EQ(catalog.Find("wire")->trapKind, TrapKind::Alarm);
	EXPECT_FLOAT_EQ(catalog.Find("wire")->trapAmount, 640.f);
}

TEST(TrapCatalogTest, APropWithoutATrapLineIsNotATrap)
{
	std::string text =
		"[prop crate]\n"
		"name Ящик\n"
		"size 48\n";
	std::istringstream input(text);

	PropCatalog catalog = PropCatalog::Parse(input, "traps");

	ASSERT_NE(catalog.Find("crate"), nullptr);
	EXPECT_FALSE(catalog.Find("crate")->IsTrap());
}
