#pragma once

#include "LevelData.h"
#include "LevelZones.h"
#include "PropCatalog.h"
#include <algorithm>
#include <string>
#include <vector>

namespace ActRoutes
{
	/**
	*	Карта акта, которую тест может ломать. Путь проверяется так: два других
	*	закладываются наглухо, и акт обязан остаться проходимым. Заложить - это
	*	и проход стеной, и то, чем он открывался: ключ, рычаг, плитка, проп.
	*
	*	Помощники общие, потому что акты разные, а способ проверки один.
	*/
	class Closer
	{
	public:
		explicit Closer(RoguelikeGame::LevelData& level) : level(level) {}

		void Wall(int column, int row)
		{
			level.tiles[row][column] = RoguelikeGame::TileType::Wall;
		}

		// Заложить проход и убрать саму дверь: иначе проверка карты увидит
		// дверь в стене и скажет про тупик, а тест не про это.
		void CloseDoor(const std::string& doorId)
		{
			for (const auto& door : level.doors)
			{
				if (door.doorId == doorId)
				{
					Wall(door.column, door.row);
				}
			}

			level.doors.erase(std::remove_if(level.doors.begin(), level.doors.end(),
				[&doorId](const RoguelikeGame::DoorPlacement& door) { return door.doorId == doorId; }),
				level.doors.end());
		}

		void CloseProps(const std::string& propId)
		{
			for (const auto& prop : level.props)
			{
				if (prop.propId == propId)
				{
					Wall(prop.column, prop.row);
				}
			}

			DropProps(propId);
		}

		void DropProps(const std::string& propId)
		{
			level.props.erase(std::remove_if(level.props.begin(), level.props.end(),
				[&propId](const RoguelikeGame::PropPlacement& prop) { return prop.propId == propId; }),
				level.props.end());
		}

		void DropItem(const std::string& itemId)
		{
			level.items.erase(std::remove_if(level.items.begin(), level.items.end(),
				[&itemId](const RoguelikeGame::ItemPlacement& placement) { return placement.itemId == itemId; }),
				level.items.end());
		}

		void DropFixtures(std::vector<RoguelikeGame::FixturePlacement>& list, const std::string& id)
		{
			list.erase(std::remove_if(list.begin(), list.end(),
				[&id](const RoguelikeGame::FixturePlacement& fixture) { return fixture.id == id; }),
				list.end());
		}

		void DropLever(const std::string& id) { DropFixtures(level.levers, id); }
		void DropPlate(const std::string& id) { DropFixtures(level.plates, id); }

	private:
		RoguelikeGame::LevelData& level;
	};

	inline const RoguelikeGame::PropPlacement* FindProp(const RoguelikeGame::LevelData& level, const std::string& propId)
	{
		auto found = std::find_if(level.props.begin(), level.props.end(),
			[&propId](const RoguelikeGame::PropPlacement& prop) { return prop.propId == propId; });

		return found == level.props.end() ? nullptr : &*found;
	}

	inline const RoguelikeGame::FixturePlacement* FindFixture(
		const std::vector<RoguelikeGame::FixturePlacement>& list, const std::string& id)
	{
		auto found = std::find_if(list.begin(), list.end(),
			[&id](const RoguelikeGame::FixturePlacement& fixture) { return fixture.id == id; });

		return found == list.end() ? nullptr : &*found;
	}

	struct Cell
	{
		int column = -1;
		int row = -1;
	};

	inline Cell FindTile(const RoguelikeGame::LevelData& level, RoguelikeGame::TileType type)
	{
		for (int row = 0; row < static_cast<int>(level.tiles.size()); row++)
		{
			for (int column = 0; column < level.width; column++)
			{
				if (TileAt(level, column, row) == type)
				{
					return {column, row};
				}
			}
		}

		return {};
	}

	// Ломаемое не преграда: до него дошли и его разнесли. Преграда - то, что стоит всегда.
	inline bool IsWalkable(const RoguelikeGame::LevelData& level,
		const RoguelikeGame::PropCatalog& props, int column, int row)
	{
		RoguelikeGame::TileType tile = TileAt(level, column, row);
		if (tile == RoguelikeGame::TileType::Wall || tile == RoguelikeGame::TileType::Empty)
		{
			return false;
		}

		for (const RoguelikeGame::PropPlacement& placement : level.props)
		{
			if (placement.column != column || placement.row != row)
			{
				continue;
			}

			const RoguelikeGame::PropDefinition* prop = props.Find(placement.propId);
			if (prop != nullptr && prop->isSolid && !prop->IsDestructible())
			{
				return false;
			}
		}

		return true;
	}

	// Вход рисуют по-разному: в акте это Entrance, в одиночной карте - PlayerSpawn.
	inline Cell StartOf(const RoguelikeGame::LevelData& level)
	{
		Cell cell = FindTile(level, RoguelikeGame::TileType::Entrance);

		return cell.row < 0 ? FindTile(level, RoguelikeGame::TileType::PlayerSpawn) : cell;
	}

	// Выход - тайл, люк или машина: уводит с локации любой из трёх.
	inline Cell ExitOf(const RoguelikeGame::LevelData& level)
	{
		Cell cell = FindTile(level, RoguelikeGame::TileType::Exit);
		if (cell.row >= 0)
		{
			return cell;
		}

		if (!level.hatches.empty())
		{
			return {level.hatches.front().column, level.hatches.front().row};
		}

		if (!level.escapes.empty())
		{
			return {level.escapes.front().column, level.escapes.front().row};
		}

		return {};
	}

	/**
	*	Зона входа и зона выхода не в счёт, но вход стоит в дверях - на клетке,
	*	которая сама в зону не попала. Поэтому берём и соседнюю через клетку.
	*/
	inline const RoguelikeGame::LevelZone* ZoneNear(
		const std::vector<RoguelikeGame::LevelZone>& zones, Cell cell)
	{
		for (const RoguelikeGame::LevelZone& zone : zones)
		{
			if (cell.column >= zone.minColumn - 1 && cell.column <= zone.maxColumn + 1
				&& cell.row >= zone.minRow - 1 && cell.row <= zone.maxRow + 1)
			{
				return &zone;
			}
		}

		return nullptr;
	}

	/**
	*	Пробежка мимо комнат: путь от входа к выходу, не задевший ни одной зоны.
	*	Комнаты старта и выхода не в счёт - их не обойти по построению.
	*
	*	Двери и ломаемые пропы считаются открытыми: проверяется планировка акта,
	*	а не то, чем её заперли. Если и так пути нет - комнаты обойти нельзя.
	*/
	inline bool CanSlipPastTheRooms(const RoguelikeGame::LevelData& level,
		const RoguelikeGame::PropCatalog& props)
	{
		Cell start = StartOf(level);
		Cell finish = ExitOf(level);
		if (start.row < 0 || finish.row < 0)
		{
			return false;
		}

		std::vector<RoguelikeGame::LevelZone> zones = RoguelikeGame::BuildZones(level);
		const RoguelikeGame::LevelZone* home = ZoneNear(zones, start);
		const RoguelikeGame::LevelZone* last = ZoneNear(zones, finish);

		int height = static_cast<int>(level.tiles.size());
		std::vector<char> seen(static_cast<std::size_t>(height) * level.width, 0);
		std::vector<Cell> queue = {start};
		seen[static_cast<std::size_t>(start.row) * level.width + start.column] = 1;

		const int STEPS[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
		for (std::size_t head = 0; head < queue.size(); head++)
		{
			Cell cell = queue[head];
			if (cell.column == finish.column && cell.row == finish.row)
			{
				return true;
			}

			for (const int* step : STEPS)
			{
				Cell next = {cell.column + step[0], cell.row + step[1]};
				if (next.column < 0 || next.column >= level.width || next.row < 0 || next.row >= height)
				{
					continue;
				}

				std::size_t index = static_cast<std::size_t>(next.row) * level.width + next.column;
				if (seen[index] != 0 || !IsWalkable(level, props, next.column, next.row))
				{
					continue;
				}

				const RoguelikeGame::LevelZone* zone = FindZoneAt(zones, next.column, next.row);
				if (zone != nullptr && zone != home && zone != last)
				{
					continue;
				}

				seen[index] = 1;
				queue.push_back(next);
			}
		}

		return false;
	}

	inline bool HasDoor(const RoguelikeGame::LevelData& level, const std::string& doorId)
	{
		return std::any_of(level.doors.begin(), level.doors.end(),
			[&doorId](const RoguelikeGame::DoorPlacement& door) { return door.doorId == doorId; });
	}
}
