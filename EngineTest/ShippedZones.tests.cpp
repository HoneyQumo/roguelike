#include "pch.h"
#include "ActAssembler.h"
#include "ActPlan.h"
#include "EnemyCatalog.h"
#include "LevelCatalog.h"
#include "LevelLoader.h"
#include "LevelZones.h"
#include "ProjectFiles.h"
#include <map>
#include <string>
#include <vector>

using RoguelikeGame::ActLoader;
using RoguelikeGame::ActPlan;
using RoguelikeGame::BuildZones;
using RoguelikeGame::FindEnemyConfig;
using RoguelikeGame::FindZoneAt;
using RoguelikeGame::LevelCatalog;
using RoguelikeGame::LevelData;
using RoguelikeGame::LevelEntry;
using RoguelikeGame::LevelLoader;
using RoguelikeGame::LevelZone;
using RoguelikeGame::LoadAct;
using RoguelikeGame::RoomPlacement;

namespace
{
	struct Room
	{
		int column = 0;
		int row = 0;
		int width = 0;
		int height = 0;
	};

	// Комнаты у актов разного размера: мост собран из полос 32x12, тюрьма
	// и улица - из квадратов 16x12. Границы берём из плана, а не на глаз.
	std::map<std::string, Room> RoomsOf(const std::string& actPath)
	{
		ActPlan plan = ActLoader::Load(actPath);
		std::map<std::string, Room> rooms;

		for (const RoomPlacement& placement : plan.rooms)
		{
			auto found = plan.library.find(placement.roomId);
			if (found == plan.library.end())
			{
				continue;
			}

			LevelData layout = LevelLoader::Load(found->second);
			bool isTurned = placement.quarters % 2 != 0;

			Room room;
			room.column = placement.column;
			room.row = placement.row;
			room.width = isTurned ? static_cast<int>(layout.tiles.size()) : layout.width;
			room.height = isTurned ? layout.width : static_cast<int>(layout.tiles.size());

			// Имя зоны собирается из id комнаты и её угла - так же и ищем.
			rooms[placement.roomId + "@" + std::to_string(placement.column) + ";" + std::to_string(placement.row)] = room;
		}

		return rooms;
	}

	// Имя зоны после сборки - room@столбец;строка[:имя].
	std::string PlaceOf(const std::string& id)
	{
		std::size_t colon = id.find(':');

		return colon == std::string::npos ? id : id.substr(0, colon);
	}

	/**
	*	Акт в один ряд комнат - это забег: игрок и так проходит каждую комнату
	*	насквозь, и «проснуться на комнату вперёд» там не беда, а норма. Правило
	*	про коридор - про сетку, где мимо комнаты можно пройти, не заходя в неё.
	*/
	bool IsSingleRow(const std::map<std::string, Room>& rooms)
	{
		if (rooms.empty())
		{
			return true;
		}

		int first = rooms.begin()->second.row;
		for (const auto& room : rooms)
		{
			if (room.second.row != first)
			{
				return false;
			}
		}

		return true;
	}

	class ShippedZonesTest : public ProjectFiles::Test
	{
	};
}

/**
*	Края комнат акта - общие коридорные полосы. Зона, дотянувшаяся до края,
*	будит свою комнату и соседние от прохода мимо, а засада в такой зоне
*	срабатывает, когда игрок ещё в коридоре.
*/
TEST_F(ShippedZonesTest, NoZoneInAnyActReachesTheCorridors)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelCatalog catalog = LevelCatalog::Load("Resources/Levels/levels.config");
	ASSERT_FALSE(catalog.IsEmpty());

	int checked = 0;
	for (const LevelEntry& entry : catalog)
	{
		if (!entry.isAct)
		{
			continue;
		}

		std::map<std::string, Room> rooms = RoomsOf(entry.filePath);
		if (IsSingleRow(rooms))
		{
			continue;
		}

		LevelData act = LoadAct(entry.filePath);

		for (const LevelZone& zone : BuildZones(act))
		{
			auto found = rooms.find(PlaceOf(zone.id));
			if (found == rooms.end())
			{
				continue;
			}

			const Room& room = found->second;
			checked++;

			EXPECT_GT(zone.minColumn, room.column) << entry.id << ": " << zone.id << " достаёт до левого края комнаты";
			EXPECT_LT(zone.maxColumn, room.column + room.width - 1) << entry.id << ": " << zone.id << " достаёт до правого края";
			EXPECT_GT(zone.minRow, room.row) << entry.id << ": " << zone.id << " достаёт до верхнего края";
			EXPECT_LT(zone.maxRow, room.row + room.height - 1) << entry.id << ": " << zone.id << " достаёт до нижнего края";
		}
	}

	EXPECT_GT(checked, 0) << "ни одной зоны не проверено - тест ничего не стережёт";
}

// Враг вне зоны не спит вовсе и идёт по своим делам с первой секунды уровня.
TEST_F(ShippedZonesTest, EveryGuardInAnyActBelongsToSomeZone)
{
	ASSERT_TRUE(isFound) << previous.string();

	LevelCatalog catalog = LevelCatalog::Load("Resources/Levels/levels.config");
	ASSERT_FALSE(catalog.IsEmpty());

	for (const LevelEntry& entry : catalog)
	{
		if (!entry.isAct)
		{
			continue;
		}

		LevelData act = LoadAct(entry.filePath);
		std::vector<LevelZone> zones = BuildZones(act);

		for (int row = 0; row < static_cast<int>(act.tiles.size()); row++)
		{
			for (int column = 0; column < static_cast<int>(act.tiles[row].size()); column++)
			{
				if (FindEnemyConfig(act.tiles[row][column]) == nullptr)
				{
					continue;
				}

				EXPECT_NE(FindZoneAt(zones, column, row), nullptr)
					<< entry.id << ": охранник в " << column << ";" << row << " ни в какой зоне и потому не спит";
			}
		}
	}
}
