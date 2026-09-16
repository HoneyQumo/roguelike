#include "pch.h"
#include "FuseComponent.h"
#include "GameWorld.h"
#include "ProjectFiles.h"
#include "GameSettings.h"
#include "PropCatalog.h"
#include <algorithm>
#include <sstream>

using RoguelikeGame::FuseComponent;
using RoguelikeGame::PropCatalog;
using RoguelikeGame::PropDefinition;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	class WreckOrderTest : public ProjectFiles::Test
	{
	};
}

namespace
{
	constexpr float STEP = 0.05f;

	class FuseTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* barrel = GameWorld::Instance()->CreateGameObject("Barrel");
			fuse = barrel->AddComponent<FuseComponent>();

			blasts = 0;
			fuse->SubscribeBurnedOut([this]() { blasts++; });

			GameWorld::Instance()->Update(STEP);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		void Run(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += STEP)
			{
				GameWorld::Instance()->Update(STEP);
			}
		}

		FuseComponent* fuse = nullptr;
		int blasts = 0;
	};

	PropCatalog CatalogOf(const std::string& text)
	{
		std::istringstream input(text);

		return PropCatalog::Parse(input, "blast");
	}

	class ShippedBlastsTest : public ProjectFiles::Test
	{
	};
}

TEST_F(FuseTest, AnUnlitFuseNeverGoesOff)
{
	Run(2.f);

	EXPECT_EQ(blasts, 0);
	EXPECT_FALSE(fuse->IsLit());
}

TEST_F(FuseTest, ALitFuseGoesOffOnceItBurnsDown)
{
	fuse->Light(0.3f);

	Run(0.2f);
	EXPECT_EQ(blasts, 0) << "went off early";
	EXPECT_TRUE(fuse->IsLit());

	Run(0.3f);

	EXPECT_EQ(blasts, 1);
	EXPECT_TRUE(fuse->HasBurnedOut());
}

TEST_F(FuseTest, HittingABurningBarrelDoesNotRestartItsFuse)
{
	fuse->Light(0.3f);
	Run(0.2f);

	fuse->Light(5.f);
	Run(0.2f);

	EXPECT_EQ(blasts, 1) << "the second light pushed the blast away";
}

TEST_F(FuseTest, AFuseGoesOffOnlyOnce)
{
	fuse->Light(0.1f);
	Run(1.f);

	fuse->Light(0.1f);
	Run(1.f);

	EXPECT_EQ(blasts, 1);
}

TEST_F(FuseTest, AFuseWithoutTimeStillWaitsAFrame)
{
	fuse->Light(0.f);

	EXPECT_EQ(blasts, 0) << "exploded inside the call that lit it";

	Run(STEP);

	EXPECT_EQ(blasts, 1);
}

TEST(PropBlastTests, OnlyABreakablePropCanExplode)
{
	PropCatalog catalog = CatalogOf(
		"[prop wall_barrel]\n"
		"name Barrel\n"
		"blast 150 55 0.25\n"
		"size 56\n");

	const PropDefinition* barrel = catalog.Find("wall_barrel");
	ASSERT_NE(barrel, nullptr);

	EXPECT_FALSE(barrel->IsExplosive()) << "an unbreakable prop cannot detonate";
}

TEST(PropBlastTests, ABreakablePropWithABlastIsExplosive)
{
	PropCatalog catalog = CatalogOf(
		"[prop barrel]\n"
		"name Barrel\n"
		"health 25\n"
		"blast 150 55 0.25\n"
		"size 56\n");

	const PropDefinition* barrel = catalog.Find("barrel");
	ASSERT_NE(barrel, nullptr);

	EXPECT_TRUE(barrel->IsExplosive());
	EXPECT_FLOAT_EQ(barrel->blastRadius, 150.f);
	EXPECT_FLOAT_EQ(barrel->blastDamage, 55.f);
	EXPECT_FLOAT_EQ(barrel->blastFuse, 0.25f);
}

TEST_F(ShippedBlastsTest, WhatExplodesOnTheBridgeCanAlsoBeBroken)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : {"fuel_barrel", "car_sedan", "car_van", "car_small"})
	{
		const PropDefinition* prop = props.Find(id);
		ASSERT_NE(prop, nullptr) << id;

		EXPECT_TRUE(prop->IsExplosive()) << id << " does not go off";
		EXPECT_GT(prop->health, 0.f) << id << " cannot be broken";
		EXPECT_GT(prop->blastFuse, 0.f) << id << " detonates inside the same frame";
	}
}

TEST_F(ShippedBlastsTest, TheBarrelIsQuickerAndTheCarIsWider)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");
	const PropDefinition* barrel = props.Find("fuel_barrel");
	const PropDefinition* car = props.Find("car_van");
	ASSERT_NE(barrel, nullptr);
	ASSERT_NE(car, nullptr);

	EXPECT_LT(barrel->blastFuse, car->blastFuse);
	EXPECT_LT(barrel->blastRadius, car->blastRadius);
	EXPECT_LT(barrel->health, car->health);
}

TEST(PropBlastTests, AWreckKeepsItsColliderWhileARegularPropOpensUp)
{
	PropCatalog catalog = CatalogOf(
		"[prop car]\n"
		"name Car\n"
		"health 90\n"
		"size 120\n"
		"wreck true\n"
		"\n"
		"[prop crate]\n"
		"name Crate\n"
		"health 20\n"
		"size 48\n");

	ASSERT_NE(catalog.Find("car"), nullptr);
	ASSERT_NE(catalog.Find("crate"), nullptr);

	EXPECT_TRUE(catalog.Find("car")->leavesWreck);
	EXPECT_FALSE(catalog.Find("crate")->leavesWreck) << "a crate should open the way once broken";
}

TEST(PropBlastTests, AnOrdinaryPropTurnsIntoItsWreckRightAway)
{
	PropCatalog catalog = CatalogOf(
		"[prop crate]\n"
		"name Crate\n"
		"health 20\n"
		"size 48\n");

	const PropDefinition* crate = catalog.Find("crate");
	ASSERT_NE(crate, nullptr);

	EXPECT_TRUE(crate->ShowsWreckOnBreak());
}

TEST(PropBlastTests, ExplosivesKeepTheirLooksUntilTheyGoOff)
{
	PropCatalog catalog = CatalogOf(
		"[prop barrel]\n"
		"name Barrel\n"
		"health 25\n"
		"size 44\n"
		"blast 150 55 0.25\n");

	const PropDefinition* barrel = catalog.Find("barrel");
	ASSERT_NE(barrel, nullptr);
	ASSERT_TRUE(barrel->IsExplosive());

	EXPECT_FALSE(barrel->ShowsWreckOnBreak()) << "the barrel blackens before it blows up";
}

TEST_F(WreckOrderTest, NothingOnTheBridgeBlackensBeforeItsBlast)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : {"car_sedan", "car_van", "car_small", "fuel_barrel"})
	{
		const PropDefinition* prop = props.Find(id);
		ASSERT_NE(prop, nullptr) << id;
		ASSERT_TRUE(prop->IsExplosive()) << id;

		EXPECT_FALSE(prop->ShowsWreckOnBreak()) << id << " shows its wreck while the fuse is still burning";
	}
}

namespace
{
	class DeadlyBlastTest : public ProjectFiles::Test
	{
	protected:
		/**
		*	Сколько урона надо, чтобы наверняка убить игрока в лучшем снаряжении.
		*	Броня съедает свою долю, пока не кончится, остальное идёт в тело.
		*/
		static float EnoughToKill()
		{
			return RoguelikeGame::PLAYER_MAX_HEALTH + RoguelikeGame::PLAYER_ARMOR_CAP;
		}

		// Тот же расчёт, что в ExplosiveComponent::DamageAt.
		static float DamageAt(const RoguelikeGame::PropDefinition& prop, float distance)
		{
			float core = RoguelikeGame::EXPLOSION_CORE_RADIUS;
			if (distance <= core)
			{
				return prop.blastDamage;
			}

			float span = std::max(prop.blastRadius - core, 1.f);
			float part = std::min((distance - core) / span, 1.f);

			return prop.blastDamage * (1.f - part * (1.f - RoguelikeGame::PROP_BLAST_EDGE_PART));
		}
	};

	const char* BLASTING[] = {"car_sedan", "car_van", "car_small", "fuel_barrel"};
}

TEST_F(DeadlyBlastTest, StandingNextToItIsCertainDeath)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : BLASTING)
	{
		const PropDefinition* prop = props.Find(id);
		ASSERT_NE(prop, nullptr) << id;

		// Вплотную - это борт машины плюс полшага игрока, а не математический центр.
		float touching = 0.5f * prop->size + 0.5f * RoguelikeGame::CHARACTER_COLLIDER_SIZE;

		EXPECT_GE(DamageAt(*prop, touching), EnoughToKill())
			<< id << " leaves a fully armoured player alive at point blank";
	}
}

TEST_F(DeadlyBlastTest, AtTheEdgeItHurtsButLetsYouLive)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : BLASTING)
	{
		const PropDefinition* prop = props.Find(id);
		ASSERT_NE(prop, nullptr) << id;

		float atEdge = DamageAt(*prop, prop->blastRadius);

		EXPECT_LT(atEdge, RoguelikeGame::PLAYER_MAX_HEALTH)
			<< id << " kills outright even at the rim of its radius";
		EXPECT_GT(atEdge, 0.1f * RoguelikeGame::PLAYER_MAX_HEALTH)
			<< id << " is harmless at the rim, so the radius is a lie";
	}
}

TEST_F(DeadlyBlastTest, TheDamageOnlyFallsOffAsYouBackAway)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : BLASTING)
	{
		const PropDefinition* prop = props.Find(id);
		ASSERT_NE(prop, nullptr) << id;

		float previousDamage = DamageAt(*prop, 0.f);
		for (float distance = 0.f; distance <= prop->blastRadius; distance += 8.f)
		{
			float damage = DamageAt(*prop, distance);

			EXPECT_LE(damage, previousDamage + 0.01f) << id << " hits harder further away, at " << distance;
			previousDamage = damage;
		}
	}
}

TEST_F(DeadlyBlastTest, InsideTheCoreThereIsNoSafeSpot)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");
	const PropDefinition* car = props.Find("car_sedan");
	ASSERT_NE(car, nullptr);

	// В эпицентре всё равно, где именно стоишь.
	EXPECT_FLOAT_EQ(DamageAt(*car, 0.f), DamageAt(*car, RoguelikeGame::EXPLOSION_CORE_RADIUS));
	EXPECT_LT(DamageAt(*car, RoguelikeGame::EXPLOSION_CORE_RADIUS + 40.f), car->blastDamage);
}

TEST_F(DeadlyBlastTest, ACarIsWorseThanABarrelWhereverYouStand)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");
	const PropDefinition* car = props.Find("car_van");
	const PropDefinition* barrel = props.Find("fuel_barrel");
	ASSERT_NE(car, nullptr);
	ASSERT_NE(barrel, nullptr);

	EXPECT_GT(car->blastRadius, barrel->blastRadius);

	for (float distance = 0.f; distance <= barrel->blastRadius; distance += 16.f)
	{
		EXPECT_GE(DamageAt(*car, distance), DamageAt(*barrel, distance))
			<< "a barrel out-blasts a van at " << distance;
	}
}

TEST_F(DeadlyBlastTest, TheBlastReachesWellPastTheThingThatBlew)
{
	ASSERT_TRUE(isFound) << previous.string();

	PropCatalog props = PropCatalog::Load("Resources/Props/props.config");

	for (const char* id : BLASTING)
	{
		const PropDefinition* prop = props.Find(id);
		ASSERT_NE(prop, nullptr) << id;

		EXPECT_GT(prop->blastRadius, 3.f * RoguelikeGame::TILE_SIZE) << id << " barely reaches past its own bumper";
	}
}
