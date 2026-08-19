#include "LevelLoader.h"
#include <fstream>
#include <iostream>

namespace RoguelikeGame
{
	bool LevelLoader::Load(const std::string& filePath, LevelData& levelData)
	{
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			std::cout << "Can't open level file: " << filePath << std::endl;
			return false;
		}

		levelData = LevelData();

		std::string line;
		while (std::getline(file, line))
		{
			if (!line.empty() && line.back() == '\r')
			{
				line.pop_back();
			}

			if (line.empty() || line.front() == ';')
			{
				continue;
			}

			std::vector<TileType> row;
			row.reserve(line.size());
			for (char symbol : line)
			{
				row.push_back(SymbolToTileType(symbol));
			}

			if ((int)row.size() > levelData.width)
			{
				levelData.width = (int)row.size();
			}
			levelData.tiles.push_back(row);
		}

		file.close();

		levelData.height = (int)levelData.tiles.size();
		if (levelData.height == 0)
		{
			std::cout << "Level file is empty: " << filePath << std::endl;
			return false;
		}

		return true;
	}

	TileType LevelLoader::SymbolToTileType(char symbol)
	{
		switch (symbol)
		{
		case '#':
			return TileType::Wall;
		case '.':
			return TileType::Floor;
		case '@':
			return TileType::PlayerSpawn;
		case 'E':
			return TileType::EnemySpawn;
		default:
			return TileType::Empty;
		}
	}
}
