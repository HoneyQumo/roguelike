#include "pch.h"
#include "ActAssembler.h"
#include "ItemCatalogLoader.h"
#include "LevelIntegrity.h"
#include "LevelLoader.h"
#include "ProjectFiles.h"
#include "EnemyCatalog.h"
#include "LevelZones.h"
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

	constexpr int ROOM_WIDTH = 16;
	constexpr int ROOM_HEIGHT = 12;

	// Зона после сборки зовётся room@столбец;строка[:имя] - угол комнаты в самом имени.
	bool RoomOriginOf(const std::string& id, int& column, int& row)
	{
		std::size_t at = id.find('@');
		std::size_t semicolon = id.find(';', at);
		if (at == std::string::npos || semicolon == std::string::npos)
		{
			return false;
		}

		column = std::stoi(id.substr(at + 1, semicolon - at - 1));
		row = std::stoi(id.substr(semicolon + 1));

		return true;
	}

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

TEST_F(PrisonRoutesTest, EveryRouteHasItsOwnTrouble)
{
	ASSERT_TRUE(isFound) << previous.string();

	int traps = 0;
	for (const auto& prop : prison.props)
	{
		const RoguelikeGame::PropDefinition* definition = props.Find(prop.propId);
		traps += definition != nullptr && definition->IsTrap() ? 1 : 0;
	}

	EXPECT_GE(traps, 3) << "по ловушке на путь - минимум";
	EXPECT_GE(prison.ambushes.size(), 3u) << "по засаде на путь - минимум";
}

// Враг вне зоны не спит вовсе и идёт по своим делам с первой секунды. Комната
// без зон становится одной зоной сама, а вот узкая зона легко оставляет кого-то снаружи.
TEST_F(PrisonRoutesTest, EveryGuardBelongsToSomeRoom)
{
	ASSERT_TRUE(isFound) << previous.string();

	std::vector<RoguelikeGame::LevelZone> zones = RoguelikeGame::BuildZones(prison);
	ASSERT_FALSE(zones.empty());

	for (int row = 0; row < static_cast<int>(prison.tiles.size()); row++)
	{
		for (int column = 0; column < static_cast<int>(prison.tiles[row].size()); column++)
		{
			if (RoguelikeGame::FindEnemyConfig(prison.tiles[row][column]) == nullptr)
			{
				continue;
			}

			EXPECT_NE(RoguelikeGame::FindZoneAt(zones, column, row), nullptr)
				<< "охранник в " << column << ";" << row << " ни в какой зоне и потому не спит";
		}
	}
}

// Края комнат акта - общие коридорные полосы. Зона, дотянувшаяся до края, будит
// свою комнату и соседние от прохода мимо - и босса в том числе.
TEST_F(PrisonRoutesTest, NoZoneReachesTheCorridorsBetweenRooms)
{
	ASSERT_TRUE(isFound) << previous.string();

	std::vector<RoguelikeGame::LevelZone> zones = RoguelikeGame::BuildZones(prison);
	ASSERT_FALSE(zones.empty());

	for (const RoguelikeGame::LevelZone& zone : zones)
	{
		int column = 0;
		int row = 0;
		ASSERT_TRUE(RoomOriginOf(zone.id, column, row)) << zone.id;

		// Имя зоны несёт угол своей комнаты: room@столбец;строка[:зона].
		EXPECT_GT(zone.minColumn, column) << zone.id << " достаёт до левого края комнаты";
		EXPECT_LT(zone.maxColumn, column + ROOM_WIDTH - 1) << zone.id << " достаёт до правого края";
		EXPECT_GT(zone.minRow, row) << zone.id << " достаёт до верхнего края";
		EXPECT_LT(zone.maxRow, row + ROOM_HEIGHT - 1) << zone.id << " достаёт до нижнего края";
	}
}

TEST_F(PrisonRoutesTest, ThePrisonIsSoundAsItShips)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelReport report = CheckLevel(prison, items, props);

	EXPECT_TRUE(report.IsClean()) << report.Describe();
}
