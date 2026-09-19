#include "pch.h"
#include "ActAssembler.h"
#include "ItemCatalogLoader.h"
#include "LevelIntegrity.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
#include "PropCatalog.h"
#include <algorithm>
#include <set>
#include <string>
#include <vector>

using RoguelikeGame::CheckLevel;
using RoguelikeGame::ItemCatalog;
using RoguelikeGame::ItemCatalogLoader;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelReport;
using RoguelikeGame::LoadAct;
using RoguelikeGame::PropCatalog;
using RoguelikeGame::TileType;

namespace
{
	const std::string PRISON = "Resources/Acts/act1_prison.config";

	// Линия ворот: ряд стен между средним и нижним этажом тюрьмы. Через него
	// и идут три пути вниз, а больше вниз идти неоткуда.
	constexpr int GATE_LINE = 22;

	class PrisonRoutesTest : public ProjectFiles::Test
	{
	protected:
		void SetUp() override
		{
			ProjectFiles::Test::SetUp();
			if (!isFound)
			{
				return;
			}

			items = ItemCatalogLoader::Load("Resources/Items/items.config");
			props = PropCatalog::Load("Resources/Props/props.config");
			prison = LoadAct(PRISON);
		}

		bool HasDoor(const std::string& id) const
		{
			return std::any_of(prison.doors.begin(), prison.doors.end(),
				[&id](const RoguelikeGame::DoorPlacement& door) { return door.doorId == id; });
		}

		bool HasFixture(const std::vector<RoguelikeGame::FixturePlacement>& list, const std::string& id) const
		{
			return std::any_of(list.begin(), list.end(),
				[&id](const RoguelikeGame::FixturePlacement& fixture) { return fixture.id == id; });
		}

		const RoguelikeGame::PropPlacement* FindProp(const std::string& id) const
		{
			auto found = std::find_if(prison.props.begin(), prison.props.end(),
				[&id](const RoguelikeGame::PropPlacement& prop) { return prop.propId == id; });

			return found == prison.props.end() ? nullptr : &*found;
		}

		void DropItem(const std::string& id)
		{
			prison.items.erase(std::remove_if(prison.items.begin(), prison.items.end(),
				[&id](const RoguelikeGame::ItemPlacement& placement) { return placement.itemId == id; }),
				prison.items.end());
		}

		void Wall(int column, int row)
	{
		prison.tiles[row][column] = TileType::Wall;
	}

	void DropDoors(const std::string& id)
	{
		prison.doors.erase(std::remove_if(prison.doors.begin(), prison.doors.end(),
			[&id](const RoguelikeGame::DoorPlacement& door) { return door.doorId == id; }),
			prison.doors.end());
	}

	void DropFixtures(std::vector<RoguelikeGame::FixturePlacement>& list, const std::string& id)
	{
		list.erase(std::remove_if(list.begin(), list.end(),
			[&id](const RoguelikeGame::FixturePlacement& fixture) { return fixture.id == id; }),
			list.end());
	}

	// Заложить путь наглухо: и проход, и то, чем он открывается.
	void CloseDoorWay(const std::string& id)
	{
		for (const auto& door : prison.doors)
		{
			if (door.doorId == id)
			{
				Wall(door.column, door.row);
			}
		}

		DropDoors(id);
	}

	void CloseQuietWay()
	{
		CloseDoorWay("door_drain");
		DropFixtures(prison.plates, "door_drain");
	}

	void CloseKeyWay()
	{
		CloseDoorWay("door_gate");
		DropFixtures(prison.levers, "door_gate");
		DropItem("key_gate");
	}

	void CloseLoudWay()
	{
		const RoguelikeGame::PropPlacement* wall = FindProp("wall_cracked");
		if (wall != nullptr)
		{
			Wall(wall->column, wall->row);
		}

		prison.props.erase(std::remove_if(prison.props.begin(), prison.props.end(),
			[](const RoguelikeGame::PropPlacement& prop) { return prop.propId == "wall_cracked"; }),
			prison.props.end());
	}

	ItemCatalog items;
		PropCatalog props;
		LevelData prison;
	};
}

TEST_F(PrisonRoutesTest, TheQuietWayIsAGrateBehindAHiddenPlate)
{
	ASSERT_TRUE(isFound) << previous.string();

	EXPECT_TRUE(HasDoor("door_drain")) << "в прачечной нет решётки вниз";
	EXPECT_TRUE(HasFixture(prison.plates, "door_drain")) << "решётку нечем открыть";

	// Ключа у решётки нет и быть не должно: её находят, а не отпирают.
	for (const auto& placement : prison.items)
	{
		const RoguelikeGame::ItemDefinition* item = items.Find(placement.itemId);
		ASSERT_NE(item, nullptr) << placement.itemId;
		EXPECT_NE(item->effect.target, "door_drain") << "потайная решётка перестала быть потайной";
	}
}

TEST_F(PrisonRoutesTest, TheGateOpensByKeyOrByHoldingTheSwitch)
{
	ASSERT_TRUE(isFound) << previous.string();

	ASSERT_TRUE(HasDoor("door_gate"));

	auto lever = std::find_if(prison.levers.begin(), prison.levers.end(),
		[](const RoguelikeGame::FixturePlacement& fixture) { return fixture.id == "door_gate"; });

	ASSERT_NE(lever, prison.levers.end()) << "у ворот нет своего рубильника";
	EXPECT_GT(lever->holdTime, 0.f) << "рубильник ворот обязан стоить времени, иначе ключ не нужен вовсе";
}

TEST_F(PrisonRoutesTest, TheLoudWayIsAWallToBreakWithABarrelBesideIt)
{
	ASSERT_TRUE(isFound) << previous.string();

	const RoguelikeGame::PropPlacement* wall = FindProp("wall_cracked");
	ASSERT_NE(wall, nullptr) << "ломать нечего";
	EXPECT_EQ(wall->row, GATE_LINE) << "пролом должен резать линию ворот, а не стоять где попало";

	const RoguelikeGame::PropDefinition* definition = props.Find("wall_cracked");
	ASSERT_NE(definition, nullptr);
	EXPECT_TRUE(definition->IsDestructible());
	EXPECT_TRUE(definition->isSolid) << "целая стена обязана держать клетку закрытой";
	EXPECT_FALSE(definition->leavesWreck) << "остов не даст пройти там, где сломали";

	const RoguelikeGame::PropPlacement* barrel = FindProp("fuel_barrel");
	ASSERT_NE(barrel, nullptr) << "бочки рядом нет, ломать нечем";
	EXPECT_LE(std::abs(barrel->column - wall->column), 1);
	EXPECT_LE(std::abs(barrel->row - wall->row), 1);

	const RoguelikeGame::PropDefinition* fuel = props.Find("fuel_barrel");
	ASSERT_NE(fuel, nullptr);
	EXPECT_GT(fuel->blastDamage, definition->health) << "одной бочки должно хватать на стену";
}

// За проломом обязан быть пол, иначе сломанная стена ведёт в стену.
TEST_F(PrisonRoutesTest, BehindTheBreachThereIsFloor)
{
	ASSERT_TRUE(isFound) << previous.string();

	const RoguelikeGame::PropPlacement* wall = FindProp("wall_cracked");
	ASSERT_NE(wall, nullptr);

	EXPECT_EQ(TileAt(prison, wall->column, wall->row + 1), TileType::Floor);
	EXPECT_EQ(TileAt(prison, wall->column, wall->row - 1), TileType::Floor);
}

// Каждый путь обязан выводить сам по себе. Два других закладываем наглухо -
// если тюрьма всё ещё проходима, путь настоящий, а не украшение к единственному.
TEST_F(PrisonRoutesTest, TheQuietWayAloneIsEnough)
{
	ASSERT_TRUE(isFound) << previous.string();

	CloseKeyWay();
	CloseLoudWay();

	LevelReport report = CheckLevel(prison, items, props);

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}

TEST_F(PrisonRoutesTest, TheKeyWayAloneIsEnough)
{
	ASSERT_TRUE(isFound) << previous.string();

	CloseQuietWay();
	CloseLoudWay();

	LevelReport report = CheckLevel(prison, items, props);

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}

TEST_F(PrisonRoutesTest, TheLoudWayAloneIsEnough)
{
	ASSERT_TRUE(isFound) << previous.string();

	CloseQuietWay();
	CloseKeyWay();

	LevelReport report = CheckLevel(prison, items, props);

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}

// Проверка самих закладок: если заложить всё три, тюрьма обязана распасться.
// Иначе три теста выше проходят потому, что ничего не закрывают.
TEST_F(PrisonRoutesTest, WithAllThreeShutTheWayOutIsGone)
{
	ASSERT_TRUE(isFound) << previous.string();

	CloseQuietWay();
	CloseKeyWay();
	CloseLoudWay();

	LevelReport report = CheckLevel(prison, items, props);

	EXPECT_FALSE(report.IsClean()) << "закладки ничего не закрывают, значит три теста выше ничего не доказывают";
}

TEST_F(PrisonRoutesTest, ThePrisonIsSoundAsItShips)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelReport report = CheckLevel(prison, items, props);

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}
