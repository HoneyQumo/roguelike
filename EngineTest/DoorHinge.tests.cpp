#include "pch.h"
#include "DoorHinge.h"
#include "GameSettings.h"
#include "LevelGrid.h"
#include "LevelLoader.h"
#include <SFML/Graphics/Transformable.hpp>
#include <sstream>

using RoguelikeGame::DoorHinge;
using RoguelikeGame::HingeFor;
using RoguelikeGame::LeafAngleFor;
using RoguelikeGame::LevelGrid;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::TILE_SIZE;
using XYZEngine::Vector2Df;

namespace
{
	const std::string LEGEND =
		"[legend]\n"
		"# Wall\n"
		". Floor\n"
		"+ Door:door_exit\n";

	LevelGrid GridOf(const std::string& map)
	{
		std::istringstream input(LEGEND + map);

		return LevelGrid::Build(LevelLoader::Parse(input, "hinge"));
	}

	sf::Vector2f LeafTip(const DoorHinge& hinge, float angle)
	{
		sf::Transformable leaf;
		leaf.setOrigin(0.5f * TILE_SIZE, TILE_SIZE);
		leaf.setScale(1.f, -1.f);
		leaf.setPosition(hinge.pivot.x, hinge.pivot.y);
		leaf.setRotation(angle);

		return leaf.getTransform().transformPoint(0.5f * TILE_SIZE, 0.f);
	}

	const std::string CORRIDOR =
		"[map]\n"
		"#####\n"
		"#.+.#\n"
		"#####\n";

	const std::string SHAFT =
		"[map]\n"
		"###\n"
		"#.#\n"
		"#+#\n"
		"#.#\n"
		"###\n";

	const std::string DOUBLE_DOOR =
		"[map]\n"
		"#####\n"
		"#.#.#\n"
		"#.+.#\n"
		"#.+.#\n"
		"#.#.#\n"
		"#####\n";

	const std::string DEAD_END =
		"[map]\n"
		"#####\n"
		"#.+##\n"
		"#####\n";
}

TEST(DoorHingeTest, LeafAngleFollowsTheDirectionItPointsAt)
{
	EXPECT_NEAR(LeafAngleFor({0.f, 1.f}), 0.f, 0.01f);
	EXPECT_NEAR(LeafAngleFor({1.f, 0.f}), -90.f, 0.01f);
	EXPECT_NEAR(LeafAngleFor({-1.f, 0.f}), 90.f, 0.01f);
}

TEST(DoorHingeTest, DoorInACorridorStandsAcrossIt)
{
	DoorHinge hinge = HingeFor(GridOf(CORRIDOR), 2, 1);

	EXPECT_NEAR(hinge.closedAngle, 0.f, 0.01f);
}

TEST(DoorHingeTest, DoorInAShaftLiesAcrossIt)
{
	DoorHinge hinge = HingeFor(GridOf(SHAFT), 1, 2);

	EXPECT_NEAR(hinge.closedAngle, -90.f, 0.01f);
}

TEST(DoorHingeTest, HingeSitsOnTheEdgeOfTheCell)
{
	LevelGrid grid = GridOf(CORRIDOR);
	DoorHinge hinge = HingeFor(grid, 2, 1);
	Vector2Df center = grid.ToWorld(2, 1);

	EXPECT_NEAR(hinge.pivot.x, center.x, 0.01f);
	EXPECT_NEAR(hinge.pivot.y, center.y - 0.5f * TILE_SIZE, 0.01f);
}

TEST(DoorHingeTest, OpenLeafTurnsAQuarter)
{
	DoorHinge corridor = HingeFor(GridOf(CORRIDOR), 2, 1);
	DoorHinge shaft = HingeFor(GridOf(SHAFT), 1, 2);

	EXPECT_NEAR(std::abs(corridor.openAngle - corridor.closedAngle), 90.f, 0.01f);
	EXPECT_NEAR(std::abs(shaft.openAngle - shaft.closedAngle), 90.f, 0.01f);
}

TEST(DoorHingeTest, LeafSwingsToTheFreeSide)
{
	DoorHinge open = HingeFor(GridOf(CORRIDOR), 2, 1);
	DoorHinge blocked = HingeFor(GridOf(DEAD_END), 2, 1);

	EXPECT_NEAR(open.openAngle, -90.f, 0.01f);
	EXPECT_NEAR(blocked.openAngle, 90.f, 0.01f);
}

TEST(DoorHingeTest, DoubleDoorHangsOnBothWalls)
{
	LevelGrid grid = GridOf(DOUBLE_DOOR);
	DoorHinge upper = HingeFor(grid, 2, 2);
	DoorHinge lower = HingeFor(grid, 2, 3);

	EXPECT_NEAR(upper.pivot.y, grid.ToWorld(2, 2).y + 0.5f * TILE_SIZE, 0.01f);
	EXPECT_NEAR(lower.pivot.y, grid.ToWorld(2, 3).y - 0.5f * TILE_SIZE, 0.01f);
	EXPECT_NEAR(upper.pivot.x, lower.pivot.x, 0.01f);
}

TEST(DoorHingeTest, BothLeavesOfADoubleDoorSwingTheSameWay)
{
	LevelGrid grid = GridOf(DOUBLE_DOOR);

	EXPECT_NEAR(HingeFor(grid, 2, 2).openAngle, -90.f, 0.01f);
	EXPECT_NEAR(HingeFor(grid, 2, 3).openAngle, -90.f, 0.01f);
}

TEST(DoorHingeTest, LeavesOfADoubleDoorStandAlongTheSameLine)
{
	LevelGrid grid = GridOf(DOUBLE_DOOR);
	DoorHinge upper = HingeFor(grid, 2, 2);
	DoorHinge lower = HingeFor(grid, 2, 3);

	EXPECT_NEAR(std::abs(upper.closedAngle - lower.closedAngle), 180.f, 0.01f);
}

TEST(DoorHingeTest, ClosedLeafCoversItsCell)
{
	LevelGrid grid = GridOf(CORRIDOR);
	DoorHinge hinge = HingeFor(grid, 2, 1);
	sf::Vector2f tip = LeafTip(hinge, hinge.closedAngle);
	Vector2Df center = grid.ToWorld(2, 1);

	EXPECT_NEAR(tip.x, center.x, 0.01f);
	EXPECT_NEAR(tip.y, center.y + 0.5f * TILE_SIZE, 0.01f);
}

TEST(DoorHingeTest, ClosedLeafInAShaftCoversItsCell)
{
	LevelGrid grid = GridOf(SHAFT);
	DoorHinge hinge = HingeFor(grid, 1, 2);
	sf::Vector2f tip = LeafTip(hinge, hinge.closedAngle);
	Vector2Df center = grid.ToWorld(1, 2);

	EXPECT_NEAR(tip.x, center.x + 0.5f * TILE_SIZE, 0.01f);
	EXPECT_NEAR(tip.y, center.y, 0.01f);
}

TEST(DoorHingeTest, OpenLeafLeavesTheCellAndLiesOnTheFreeSide)
{
	LevelGrid grid = GridOf(CORRIDOR);
	DoorHinge hinge = HingeFor(grid, 2, 1);
	sf::Vector2f tip = LeafTip(hinge, hinge.openAngle);

	EXPECT_NEAR(tip.x, hinge.pivot.x + TILE_SIZE, 0.01f);
	EXPECT_NEAR(tip.y, hinge.pivot.y, 0.01f);
}

TEST(DoorHingeTest, OpenLeafAvoidsTheWallInADeadEnd)
{
	LevelGrid grid = GridOf(DEAD_END);
	DoorHinge hinge = HingeFor(grid, 2, 1);
	sf::Vector2f tip = LeafTip(hinge, hinge.openAngle);

	EXPECT_NEAR(tip.x, hinge.pivot.x - TILE_SIZE, 0.01f);
	EXPECT_NEAR(tip.y, hinge.pivot.y, 0.01f);
}

TEST(DoorHingeTest, LeavesOfADoubleDoorCoverBothCells)
{
	LevelGrid grid = GridOf(DOUBLE_DOOR);
	DoorHinge upper = HingeFor(grid, 2, 2);
	DoorHinge lower = HingeFor(grid, 2, 3);

	EXPECT_NEAR(LeafTip(upper, upper.closedAngle).y, grid.ToWorld(2, 2).y - 0.5f * TILE_SIZE, 0.01f);
	EXPECT_NEAR(LeafTip(lower, lower.closedAngle).y, grid.ToWorld(2, 3).y + 0.5f * TILE_SIZE, 0.01f);
}
