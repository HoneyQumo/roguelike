#pragma once

#include <string>
#include "LevelData.h"

namespace RoguelikeGame
{
	class LevelLoader
	{
	public:
		static LevelData Load(const std::string& filePath);
	private:
		static TileType SymbolToTileType(char symbol);
	};
}
