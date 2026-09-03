#pragma once

#include <map>
#include <string>
#include "LevelData.h"

namespace RoguelikeGame
{
    class LevelLoader
    {
    public:
        static LevelData Load(const std::string& filePath);

    private:
        using Legend = std::map<char, TileType>;

        static bool IsSection(const std::string& line, const std::string& sectionName);
        static void ReadLegendLine(const std::string& line, int lineNumber, Legend& legend);
        static void ReadMapLine(const std::string& line, const Legend& legend, LevelData& levelData);
        static bool TryGetTileType(const std::string& name, TileType& tileType);
        static const Legend& GetDefaultLegend();
        static std::string Trim(const std::string& line);
    };
}
