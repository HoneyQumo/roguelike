#include "pch.h"
#include "SpatialHashGrid.h"

using XYZEngine::GridRange;
using XYZEngine::SpatialHashGrid;

namespace
{
	using Grid = SpatialHashGrid<int>;

	sf::FloatRect Box(float left, float top, float size)
	{
		return sf::FloatRect(left, top, size, size);
	}

	std::vector<int> Found(const Grid& grid, const sf::FloatRect& area)
	{
		std::vector<int> found;
		grid.Query(area, found);

		return found;
	}
}

TEST(SpatialHashGridTest, NearItemIsFound)
{
	Grid grid;
	grid.Insert(7, Box(10.f, 10.f, 20.f));

	EXPECT_EQ(Found(grid, Box(15.f, 15.f, 5.f)), std::vector<int>({7}));
}

TEST(SpatialHashGridTest, FarItemIsNotFound)
{
	Grid grid;
	grid.Insert(7, Box(10.f, 10.f, 20.f));

	EXPECT_TRUE(Found(grid, Box(5000.f, 5000.f, 20.f)).empty());
}

TEST(SpatialHashGridTest, NegativeCoordinatesAreKeptApart)
{
	Grid grid;
	grid.Insert(1, Box(-500.f, -500.f, 20.f));
	grid.Insert(2, Box(500.f, 500.f, 20.f));

	EXPECT_EQ(Found(grid, Box(-495.f, -495.f, 5.f)), std::vector<int>({1}));
	EXPECT_EQ(Found(grid, Box(505.f, 505.f, 5.f)), std::vector<int>({2}));
}

TEST(SpatialHashGridTest, KeysDoNotMixUpMirroredCells)
{
	Grid grid;
	grid.SetCellSize(100.f);
	grid.Insert(1, Box(-250.f, 350.f, 10.f));
	grid.Insert(2, Box(350.f, -250.f, 10.f));

	EXPECT_EQ(Found(grid, Box(-245.f, 355.f, 1.f)), std::vector<int>({1}));
	EXPECT_EQ(Found(grid, Box(355.f, -245.f, 1.f)), std::vector<int>({2}));
}

TEST(SpatialHashGridTest, ItemOverManyCellsIsReportedOnce)
{
	Grid grid;
	grid.SetCellSize(32.f);
	grid.Insert(5, Box(0.f, 0.f, 200.f));

	EXPECT_GT(grid.GetEntryCount(), 1u);
	EXPECT_EQ(Found(grid, Box(0.f, 0.f, 200.f)), std::vector<int>({5}));
}

TEST(SpatialHashGridTest, RemovedItemIsGone)
{
	Grid grid;
	grid.Insert(1, Box(10.f, 10.f, 20.f));
	grid.Insert(2, Box(10.f, 10.f, 20.f));
	grid.Remove(1, Box(10.f, 10.f, 20.f));

	EXPECT_EQ(Found(grid, Box(10.f, 10.f, 20.f)), std::vector<int>({2}));
}

TEST(SpatialHashGridTest, EmptyCellsAreReleased)
{
	Grid grid;
	grid.Insert(1, Box(10.f, 10.f, 20.f));
	ASSERT_EQ(grid.GetCellCount(), 1u);

	grid.Remove(1, Box(10.f, 10.f, 20.f));

	EXPECT_EQ(grid.GetCellCount(), 0u);
	EXPECT_EQ(grid.GetEntryCount(), 0u);
}

TEST(SpatialHashGridTest, MovedItemIsFoundAtTheNewPlace)
{
	Grid grid;
	grid.Insert(1, Box(10.f, 10.f, 20.f));
	grid.Move(1, Box(10.f, 10.f, 20.f), Box(900.f, 900.f, 20.f));

	EXPECT_TRUE(Found(grid, Box(10.f, 10.f, 20.f)).empty());
	EXPECT_EQ(Found(grid, Box(900.f, 900.f, 20.f)), std::vector<int>({1}));
}

TEST(SpatialHashGridTest, MoveInsideOneCellKeepsTheEntry)
{
	Grid grid;
	grid.SetCellSize(100.f);
	grid.Insert(1, Box(10.f, 10.f, 20.f));
	grid.Move(1, Box(10.f, 10.f, 20.f), Box(12.f, 12.f, 20.f));

	EXPECT_EQ(grid.GetEntryCount(), 1u);
	EXPECT_EQ(Found(grid, Box(12.f, 12.f, 20.f)), std::vector<int>({1}));
}

TEST(SpatialHashGridTest, TouchingBoxesShareACell)
{
	Grid grid;
	grid.SetCellSize(64.f);
	grid.Insert(1, sf::FloatRect(0.f, 0.f, 64.f, 64.f));

	for (int filler = 0; filler < 50; filler++)
	{
		grid.Insert(100 + filler, Box(10000.f + static_cast<float>(filler) * 64.f, 10000.f, 10.f));
	}

	EXPECT_EQ(Found(grid, sf::FloatRect(64.f, 0.f, 64.f, 64.f)), std::vector<int>({1}));
}

TEST(SpatialHashGridTest, BoxWithNegativeSizeIsStillIndexed)
{
	Grid grid;
	grid.SetCellSize(64.f);
	grid.Insert(1, sf::FloatRect(0.f, 0.f, -40.f, -40.f));

	EXPECT_EQ(Found(grid, sf::FloatRect(-20.f, -20.f, 10.f, 10.f)), std::vector<int>({1}));
}

TEST(SpatialHashGridTest, QueryWithNegativeSizeFindsTheSameBox)
{
	Grid grid;
	grid.SetCellSize(64.f);
	grid.Insert(1, Box(-100.f, -100.f, 20.f));

	EXPECT_EQ(Found(grid, sf::FloatRect(0.f, 0.f, -200.f, -200.f)), std::vector<int>({1}));
}

TEST(SpatialHashGridTest, HugeQueryStillFindsEverything)
{
	Grid grid;
	grid.SetCellSize(16.f);
	grid.Insert(1, Box(-1000.f, -1000.f, 10.f));
	grid.Insert(2, Box(1000.f, 1000.f, 10.f));

	EXPECT_EQ(Found(grid, sf::FloatRect(-1e9f, -1e9f, 2e9f, 2e9f)), std::vector<int>({1, 2}));
}

TEST(SpatialHashGridTest, CellSizeChangeEmptiesTheGrid)
{
	Grid grid;
	grid.Insert(1, Box(10.f, 10.f, 20.f));
	grid.SetCellSize(32.f);

	EXPECT_EQ(grid.GetCellSize(), 32.f);
	EXPECT_EQ(grid.GetEntryCount(), 0u);
}

TEST(SpatialHashGridTest, BadCellSizeIsIgnored)
{
	Grid grid;
	float before = grid.GetCellSize();
	grid.SetCellSize(0.f);
	grid.SetCellSize(-10.f);

	EXPECT_EQ(grid.GetCellSize(), before);
}

TEST(SpatialHashGridTest, RangeGrowsWithTheBox)
{
	Grid grid;
	grid.SetCellSize(64.f);

	GridRange one = grid.RangeOf(sf::FloatRect(0.f, 0.f, 10.f, 10.f));
	GridRange many = grid.RangeOf(sf::FloatRect(0.f, 0.f, 200.f, 200.f));

	EXPECT_EQ(one.GetCellCount(), 1u);
	EXPECT_EQ(many.GetCellCount(), 16u);
	EXPECT_NE(one, many);
}
