#pragma once

#include "LevelData.h"
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

	inline bool HasDoor(const RoguelikeGame::LevelData& level, const std::string& doorId)
	{
		return std::any_of(level.doors.begin(), level.doors.end(),
			[&doorId](const RoguelikeGame::DoorPlacement& door) { return door.doorId == doorId; });
	}
}
