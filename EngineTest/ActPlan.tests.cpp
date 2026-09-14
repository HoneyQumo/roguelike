#include "pch.h"
#include "ActPlan.h"
#include <sstream>

using RoguelikeGame::ActLoader;
using RoguelikeGame::ActPlan;

namespace
{
	ActPlan PlanOf(const std::string& text)
	{
		std::istringstream input(text);

		return ActLoader::Parse(input, "act");
	}

	const std::string ACT =
		"[act]\n"
		"title Первый акт\n"
		"next catacombs\n"
		"tileset ruins\n"
		"music march\n"
		"boss puppeteer 2.5 1.5\n"
		"[rooms]\n"
		"entry 0 0\n"
		"hall 6 0 turn 1\n"
		"vault 12 0 turn 2 mirror\n";
}

TEST(ActPlanTest, ActKeepsItsOwnSettings)
{
	ActPlan plan = PlanOf(ACT);

	EXPECT_EQ(plan.info.title, "Первый акт");
	EXPECT_EQ(plan.info.nextLevelId, "catacombs");
	EXPECT_EQ(plan.info.tileset, "ruins");
	EXPECT_EQ(plan.info.music, "march");
	EXPECT_EQ(plan.info.boss.bossId, "puppeteer");
	EXPECT_FLOAT_EQ(plan.info.boss.healthScale, 2.5f);
}

TEST(ActPlanTest, RoomsAreReadInOrder)
{
	ActPlan plan = PlanOf(ACT);

	ASSERT_EQ(plan.rooms.size(), 3u);
	EXPECT_EQ(plan.rooms[0].roomId, "entry");
	EXPECT_EQ(plan.rooms[1].roomId, "hall");
	EXPECT_EQ(plan.rooms[2].roomId, "vault");
}

TEST(ActPlanTest, RoomPlaceIsRead)
{
	ActPlan plan = PlanOf(ACT);

	EXPECT_EQ(plan.rooms[1].column, 6);
	EXPECT_EQ(plan.rooms[1].row, 0);
}

TEST(ActPlanTest, TurnAndMirrorAreRead)
{
	ActPlan plan = PlanOf(ACT);

	EXPECT_EQ(plan.rooms[0].quarters, 0);
	EXPECT_FALSE(plan.rooms[0].isMirrored);

	EXPECT_EQ(plan.rooms[1].quarters, 1);
	EXPECT_FALSE(plan.rooms[1].isMirrored);

	EXPECT_EQ(plan.rooms[2].quarters, 2);
	EXPECT_TRUE(plan.rooms[2].isMirrored);
}

TEST(ActPlanTest, ActWithoutRoomsIsRefused)
{
	EXPECT_THROW(PlanOf("[act]\nPusto\n"), std::runtime_error);
}

TEST(ActPlanTest, RoomWithoutAPlaceIsRefused)
{
	EXPECT_THROW(PlanOf("[act]\ntitle Akt\n[rooms]\nentry\n"), std::runtime_error);
}

TEST(ActPlanTest, CommentsAndBlankLinesAreSkipped)
{
	ActPlan plan = PlanOf("; note\n\n[act]\ntitle Akt\n\n[rooms]\n; more\nentry 0 0\n");

	EXPECT_EQ(plan.info.title, "Akt");
	ASSERT_EQ(plan.rooms.size(), 1u);
}
