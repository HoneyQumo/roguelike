#include "pch.h"
#include "FightStyle.h"
#include "LevelLoader.h"
#include <sstream>
#include <stdexcept>

using RoguelikeGame::FightStyle;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::ParseFightStyle;
using RoguelikeGame::RELENTLESS_FIGHT;
using RoguelikeGame::TACTICAL_FIGHT;

namespace
{
	LevelData ParseLevel(const std::string& text)
	{
		std::istringstream input(text);

		return LevelLoader::Parse(input, "style");
	}

	const std::string MAP = "[map]\n#####\n#@..#\n#####\n";
}

TEST(FightStyleTest, ByDefaultAnEnemyFightsWithItsHeadOn)
{
	FightStyle style;

	EXPECT_TRUE(style.keepsDistance);
	EXPECT_TRUE(style.takesCover);
}

TEST(FightStyleTest, RelentlessTurnsEverythingOff)
{
	EXPECT_FALSE(RELENTLESS_FIGHT.keepsDistance);
	EXPECT_FALSE(RELENTLESS_FIGHT.takesCover);
}

TEST(FightStyleTest, BothNamesAreRead)
{
	FightStyle style = RELENTLESS_FIGHT;
	ASSERT_TRUE(ParseFightStyle("tactical", style));
	EXPECT_TRUE(style.takesCover);

	ASSERT_TRUE(ParseFightStyle("relentless", style));
	EXPECT_FALSE(style.takesCover);
}

TEST(FightStyleTest, AnUnknownNameChangesNothing)
{
	FightStyle style = TACTICAL_FIGHT;

	EXPECT_FALSE(ParseFightStyle("careful", style));
	EXPECT_TRUE(style.takesCover) << "a typo silently rewrote the style";
}

TEST(FightStyleTest, WithoutAKeyTheMapIsTacticalAndThePursuitIsNot)
{
	LevelData level = ParseLevel(MAP);

	EXPECT_TRUE(level.info.style.takesCover);
	EXPECT_TRUE(level.wavesStyle.takesCover);
	EXPECT_FALSE(level.pursuit.style.takesCover) << "the pursuit stops to reload instead of pushing";
}

TEST(FightStyleTest, TheLevelSectionSwitchesThePlacedEnemies)
{
	LevelData level = ParseLevel("[level]\nstyle relentless\n" + MAP);

	EXPECT_FALSE(level.info.style.takesCover);
	EXPECT_FALSE(level.info.style.keepsDistance);
}

TEST(FightStyleTest, TheWavesSectionSwitchesTheWaves)
{
	LevelData level = ParseLevel("[legend]\ng GruntSpawn\n[waves]\nstyle relentless\nwave 1 g2\n" + MAP);

	EXPECT_FALSE(level.wavesStyle.takesCover);
	EXPECT_EQ(level.waves.size(), 1u) << "the style key ate the wave line";
}

TEST(FightStyleTest, ThePursuitSectionCanBeSwitchedBack)
{
	LevelData level = ParseLevel("[pursuit]\nkeep 2\nstyle tactical\nfrom 0 g1\n" + MAP);

	EXPECT_TRUE(level.pursuit.style.takesCover);
	EXPECT_EQ(level.pursuit.keep, 2);
}

TEST(FightStyleTest, AMisspelledStyleStopsTheLevel)
{
	EXPECT_THROW(ParseLevel("[level]\nstyle carefull\n" + MAP), std::runtime_error);
}
