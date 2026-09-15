#include "pch.h"
#include "Noise.h"
#include "WeaponCatalog.h"

using RoguelikeGame::Faction;
using RoguelikeGame::IsHeard;
using RoguelikeGame::MuffledRadius;
using RoguelikeGame::Noise;
using XYZEngine::Vector2Df;

namespace
{
	Noise Shot(float radius, Faction from = Faction::Player)
	{
		Noise noise;
		noise.position = {100.f, 100.f};
		noise.radius = radius;
		noise.from = from;

		return noise;
	}
}

TEST(NoiseTest, CloseEnoughIsHeard)
{
	EXPECT_TRUE(IsHeard(Shot(200.f), {100.f, 250.f}, Faction::Enemy));
}

TEST(NoiseTest, TooFarIsNotHeard)
{
	EXPECT_FALSE(IsHeard(Shot(200.f), {100.f, 350.f}, Faction::Enemy));
}

TEST(NoiseTest, RightAtTheEdgeIsStillHeard)
{
	EXPECT_TRUE(IsHeard(Shot(200.f), {300.f, 100.f}, Faction::Enemy));
	EXPECT_FALSE(IsHeard(Shot(200.f), {300.1f, 100.f}, Faction::Enemy));
}

TEST(NoiseTest, SilentSourceIsNeverHeard)
{
	EXPECT_FALSE(IsHeard(Shot(0.f), {100.f, 100.f}, Faction::Enemy));
	EXPECT_FALSE(IsHeard(Shot(-10.f), {100.f, 100.f}, Faction::Enemy));
}

TEST(NoiseTest, OwnSideIsNotWorthListeningTo)
{
	EXPECT_FALSE(IsHeard(Shot(400.f, Faction::Enemy), {150.f, 100.f}, Faction::Enemy));
	EXPECT_TRUE(IsHeard(Shot(400.f, Faction::Player), {150.f, 100.f}, Faction::Enemy));
}

TEST(NoiseTest, NobodysNoiseIsHeardByEveryone)
{
	EXPECT_TRUE(IsHeard(Shot(400.f, Faction::Neutral), {150.f, 100.f}, Faction::Enemy));
	EXPECT_TRUE(IsHeard(Shot(400.f, Faction::Neutral), {150.f, 100.f}, Faction::Player));
}

TEST(NoiseTest, QuietWeaponIsHeardCloserThanALoudOne)
{
	Vector2Df listener = {400.f, 100.f};

	EXPECT_TRUE(IsHeard(Shot(RoguelikeGame::SHOT_NOISE_RADIUS), listener, Faction::Enemy));
	EXPECT_FALSE(IsHeard(Shot(RoguelikeGame::QUIET_NOISE_RADIUS), listener, Faction::Enemy));
}

TEST(NoiseTest, WallCutsTheReachDown)
{
	EXPECT_FLOAT_EQ(MuffledRadius(560.f, 0), 560.f);
	EXPECT_LT(MuffledRadius(560.f, 1), 560.f);
	EXPECT_LT(MuffledRadius(560.f, 2), MuffledRadius(560.f, 1));
}

TEST(NoiseTest, ShotBehindOneWallIsHeardOnlyFromClose)
{
	EXPECT_TRUE(IsHeard(Shot(400.f), {100.f, 300.f}, Faction::Enemy, 0));
	EXPECT_FALSE(IsHeard(Shot(400.f), {100.f, 300.f}, Faction::Enemy, 1));
	EXPECT_TRUE(IsHeard(Shot(400.f), {100.f, 250.f}, Faction::Enemy, 1));
}

TEST(NoiseTest, BehindManyWallsNothingIsHeard)
{
	EXPECT_FALSE(IsHeard(Shot(560.f), {100.f, 180.f}, Faction::Enemy, 4));
}
