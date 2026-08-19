#pragma once

#include <string>
#include "LevelData.h"

namespace RoguelikeGame
{
	class LevelLoader
	{
	public:
		static bool Load(const std::string& filePath, LevelData& levelData);
	private:
		static TileType SymbolToTileType(char symbol);
	};
}
