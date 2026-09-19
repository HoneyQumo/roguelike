#include "pch.h"
#include "Noise.h"
#include "EnemyCatalog.h"
#include "WeaponCatalog.h"

using RoguelikeGame::Faction;
using RoguelikeGame::IsHeard;
using RoguelikeGame::LoudnessAt;
using RoguelikeGame::MuffledRadius;
using RoguelikeGame::Noise;
using RoguelikeGame::NoiseKind;
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

namespace
{
	Noise Call(float radius, Faction from)
	{
		Noise noise = Shot(radius, from);
		noise.kind = NoiseKind::Call;

		return noise;
	}
}

TEST(NoiseTest, ACallReachesOwnSideOnly)
{
	EXPECT_TRUE(IsHeard(Call(300.f, Faction::Enemy), {100.f, 250.f}, Faction::Enemy));
	EXPECT_FALSE(IsHeard(Call(300.f, Faction::Enemy), {100.f, 250.f}, Faction::Player));
}

TEST(NoiseTest, ADisturbanceReachesTheOtherSideOnly)
{
	EXPECT_TRUE(IsHeard(Shot(300.f, Faction::Enemy), {100.f, 250.f}, Faction::Player));
	EXPECT_FALSE(IsHeard(Shot(300.f, Faction::Enemy), {100.f, 250.f}, Faction::Enemy));
}

TEST(NoiseTest, ACallIsMuffledByWallsTheSameWay)
{
	EXPECT_TRUE(IsHeard(Call(300.f, Faction::Enemy), {100.f, 250.f}, Faction::Enemy, 0));
	EXPECT_FALSE(IsHeard(Call(300.f, Faction::Enemy), {100.f, 250.f}, Faction::Enemy, 1));
}

TEST(NoiseTest, EveryEnemyShoutsAndTheRadioShoutsFurther)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		EXPECT_GT(enemy.config.shoutRadius, 0.f) << enemy.config.objectName;
	}

	const RoguelikeGame::EnemyConfig* radio = RoguelikeGame::FindEnemyConfig(RoguelikeGame::TileType::RadioSpawn);
	const RoguelikeGame::EnemyConfig* grunt = RoguelikeGame::FindEnemyConfig(RoguelikeGame::TileType::GruntSpawn);

	ASSERT_NE(radio, nullptr);
	ASSERT_NE(grunt, nullptr);
	EXPECT_GT(radio->shoutRadius, grunt->shoutRadius);
}

// Раньше шум на краю радиуса бил ровно так же, как в упор: слышно или нет,
// середины не было.
TEST(NoiseLoudnessTest, TheFartherTheQuieter)
{
	Noise shot = Shot(200.f);

	float here = LoudnessAt(shot, {100.f, 100.f}, Faction::Enemy);
	float halfway = LoudnessAt(shot, {100.f, 200.f}, Faction::Enemy);
	float edge = LoudnessAt(shot, {100.f, 300.f}, Faction::Enemy);

	EXPECT_FLOAT_EQ(here, 1.f);
	EXPECT_GT(halfway, edge);
	EXPECT_LT(halfway, here);
	EXPECT_GT(edge, RoguelikeGame::NOISE_HEARD_AT);
}

TEST(NoiseLoudnessTest, BehindTheRadiusThereIsNothingToHear)
{
	EXPECT_FLOAT_EQ(LoudnessAt(Shot(200.f), {100.f, 301.f}, Faction::Enemy), 0.f);
}

// Сила отдельно от радиуса: шаг слышно недалеко и он тихий, у глушителя
// радиус мал, но это не шёпот.
TEST(NoiseLoudnessTest, AQuietSourceIsQuieterEverywhere)
{
	Noise loud = Shot(200.f);
	Noise quiet = Shot(200.f);
	quiet.loudness = 0.25f;

	EXPECT_LT(LoudnessAt(quiet, {100.f, 100.f}, Faction::Enemy), LoudnessAt(loud, {100.f, 100.f}, Faction::Enemy));
	EXPECT_LT(LoudnessAt(quiet, {100.f, 200.f}, Faction::Enemy), LoudnessAt(loud, {100.f, 200.f}, Faction::Enemy));
}

TEST(NoiseLoudnessTest, AWallTakesLoudnessAwayAndNotOnlyReach)
{
	Noise shot = Shot(400.f);

	float open = LoudnessAt(shot, {100.f, 200.f}, Faction::Enemy, 0);
	float behindWall = LoudnessAt(shot, {100.f, 200.f}, Faction::Enemy, 1);

	EXPECT_GT(open, behindWall);
	EXPECT_GT(behindWall, 0.f);
}

TEST(NoiseLoudnessTest, SilenceIsNotHeardAtAll)
{
	Noise mute = Shot(200.f);
	mute.loudness = 0.f;

	EXPECT_FLOAT_EQ(LoudnessAt(mute, {100.f, 100.f}, Faction::Enemy), 0.f);
	EXPECT_FALSE(IsHeard(mute, {100.f, 100.f}, Faction::Enemy));
}

TEST(NoiseLoudnessTest, TheOwnSideStaysDeafToItsOwnNoise)
{
	EXPECT_FLOAT_EQ(LoudnessAt(Shot(200.f), {100.f, 100.f}, Faction::Player), 0.f);
}
