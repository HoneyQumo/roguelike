#include "pch.h"
#include "FuseComponent.h"
#include "GameWorld.h"
#include "ProjectFiles.h"
#include "PropCatalog.h"
#include <sstream>

using RoguelikeGame::FuseComponent;
using RoguelikeGame::PropCatalog;
using RoguelikeGame::PropDefinition;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

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
