#include "pch.h"
#include "BoxColliderComponent.h"
#include "FactionComponent.h"
#include "FireComponent.h"
#include "GameSettings.h"
#include "GameWorld.h"
#include "HealthComponent.h"
#include "ProjectFiles.h"
#include "PropCatalog.h"
#include <sstream>

using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::FireComponent;
using RoguelikeGame::HealthComponent;
using RoguelikeGame::PropCatalog;
using RoguelikeGame::PropDefinition;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	constexpr float STEP = 0.05f;

	PropCatalog CatalogOf(const std::string& text)
	{
		std::istringstream input(text);

		return PropCatalog::Parse(input, "fire");
	}

	class FireTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* flame = GameWorld::Instance()->CreateGameObject("Fire");
			flame->GetTransform()->SetWorldPosition({0.f, 0.f});
			fire = flame->AddComponent<FireComponent>();
			fire->SetRadius(50.f);
			fire->SetDamage(5.f);
			fire->SetBeatTime(0.2f);

			burnedOut = 0;
			fire->SubscribeBurnedOut([this]() { burnedOut++; });

			GameWorld::Instance()->Update(STEP);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		HealthComponent* CreateVictim(Faction side, float x, float y)
		{
			GameObject* body = GameWorld::Instance()->CreateGameObject("Victim");
			body->GetTransform()->SetWorldPosition({x, y});
			body->AddComponent<FactionComponent>()->SetFaction(side);
			body->AddComponent<XYZEngine::BoxColliderComponent>()->SetSize(30.f, 30.f);

			auto health = body->AddComponent<HealthComponent>();
			health->SetMaxHealth(200.f);

			GameWorld::Instance()->Update(STEP);
			GameWorld::Instance()->UpdatePhysics();

			return health;
		}

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += STEP)
			{
				GameWorld::Instance()->Update(STEP);
				GameWorld::Instance()->UpdatePhysics();
			}
		}

		FireComponent* fire = nullptr;
		int burnedOut = 0;
	};

	class ShippedFireTest : public ProjectFiles::Test
	{
	};
}

TEST_F(FireTest, AnUnlitFireDoesNothing)
{
	HealthComponent* victim = CreateVictim(Faction::Player, 0.f, 0.f);

	Run(1.f);

	EXPECT_FLOAT_EQ(victim->GetHealth(), 200.f);
	EXPECT_FALSE(fire->IsBurning());
}

TEST_F(FireTest, FireBurnsWhoeverStandsInIt)
{
	HealthComponent* victim = CreateVictim(Faction::Player, 10.f, 0.f);
	fire->Light(1.f);

	Run(0.7f);

	EXPECT_LT(victim->GetHealth(), 200.f) << "the player walked through fire unharmed";
}

TEST_F(FireTest, FireBurnsEnemiesToo)
{
	HealthComponent* enemy = CreateVictim(Faction::Enemy, 10.f, 0.f);
	fire->Light(1.f);

	Run(0.7f);

	EXPECT_LT(enemy->GetHealth(), 200.f) << "fire takes sides";
}

TEST_F(FireTest, WhatStandsAwayFromTheFlameIsSafe)
{
	HealthComponent* far = CreateVictim(Faction::Player, 400.f, 0.f);
	fire->Light(1.f);

	Run(1.f);

	EXPECT_FLOAT_EQ(far->GetHealth(), 200.f);
}

TEST_F(FireTest, FireGoesOutWhenItsTimeIsUp)
{
	fire->Light(0.4f);

	Run(0.2f);
	EXPECT_TRUE(fire->IsBurning());

	Run(0.5f);

	EXPECT_FALSE(fire->IsBurning());
	EXPECT_EQ(burnedOut, 1);
}

TEST_F(FireTest, ABurntOutFireStopsHurting)
{
	HealthComponent* victim = CreateVictim(Faction::Player, 10.f, 0.f);
	fire->Light(0.3f);

	Run(0.6f);
	float afterFire = victim->GetHealth();

	Run(2.f);

	EXPECT_FLOAT_EQ(victim->GetHealth(), afterFire);
}

TEST_F(FireTest, AThrownEmberFliesToItsPlaceAndOnlyThenSettles)
{
	GameObject* flame = fire->GetGameObject();
	fire->Light(3.f);
	fire->Throw({0.f, 0.f}, {200.f, 0.f}, 0.4f);

	EXPECT_TRUE(fire->IsFlying());
	EXPECT_FLOAT_EQ(flame->GetTransform()->GetWorldPosition().x, 0.f);

	Run(0.2f);

	float midway = flame->GetTransform()->GetWorldPosition().x;
	EXPECT_GT(midway, 0.f) << "the ember did not leave the blast";
	EXPECT_LT(midway, 200.f) << "the ember teleported";

	Run(0.4f);

	EXPECT_FALSE(fire->IsFlying());
	EXPECT_NEAR(flame->GetTransform()->GetWorldPosition().x, 200.f, 1.f);
}

TEST_F(FireTest, AFlyingEmberDoesNotBurnWhatItPassesOver)
{
	HealthComponent* onTheWay = CreateVictim(Faction::Player, 100.f, 0.f);
	fire->Light(3.f);
	fire->Throw({0.f, 0.f}, {200.f, 0.f}, 0.4f);

	Run(0.4f);

	EXPECT_FLOAT_EQ(onTheWay->GetHealth(), 200.f) << "the ember scorched everything it flew over";
}

TEST_F(ShippedFireTest, WhatExplodesOnTheBridgeAlsoBurns)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : {"car_sedan", "car_van", "car_small", "fuel_barrel"})
	{
		const PropDefinition* prop = props.Find(id);
		ASSERT_NE(prop, nullptr) << id;

		EXPECT_GT(prop->burnTime, 0.f) << id << " goes out the moment it blows up";
		EXPECT_GT(prop->burnSpread, 0.f) << id << " always burns for exactly the same time";
		EXPECT_LT(prop->burnSpread, prop->burnTime) << id << " could burn for no time at all";
	}
}

TEST_F(ShippedFireTest, ACarBurnsMuchLongerThanABarrel)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");
	const PropDefinition* car = props.Find("car_sedan");
	const PropDefinition* barrel = props.Find("fuel_barrel");
	ASSERT_NE(car, nullptr);
	ASSERT_NE(barrel, nullptr);

	EXPECT_GT(car->burnTime, 3.f * barrel->burnTime);
	EXPECT_NEAR(car->burnTime, 60.f, 15.f) << "a car should burn for about a minute";
	EXPECT_NEAR(barrel->burnTime, 15.f, 5.f) << "a barrel should burn for about fifteen seconds";
}

TEST(PropBurnTests, APropWithoutBurnTimeLeavesNoFire)
{
	PropCatalog catalog = CatalogOf(
		"[prop crate]\n"
		"name Crate\n"
		"health 20\n"
		"size 48\n");

	const PropDefinition* crate = catalog.Find("crate");
	ASSERT_NE(crate, nullptr);

	EXPECT_FLOAT_EQ(crate->burnTime, 0.f);
}
