#include "LevelLoader.h"
#include <LoggerRegistry.h>
#include <fstream>
#include <stdexcept>

namespace RoguelikeGame
{
    const std::string LEGEND_SECTION = "legend";
    const std::string MAP_SECTION = "map";
    const char COMMENT_SYMBOL = ';';
    const char EMPTY_SYMBOL = ' ';
    const std::string WHITESPACE = " \t";
    const std::string UTF8_BOM = "\xEF\xBB\xBF";

    LevelData LevelLoader::Load(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_ERROR("Can't open level file: " + filePath);
            throw std::runtime_error("Level file is not available: " + filePath);
        }

        LevelData levelData;
        Legend legend = GetDefaultLegend();
        bool isLegendSection = false;

        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line))
        {
            lineNumber++;

            if (lineNumber == 1 && line.compare(0, UTF8_BOM.size(), UTF8_BOM) == 0)
            {
                line.erase(0, UTF8_BOM.size());
            }

            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            if (line.empty() || line.front() == COMMENT_SYMBOL)
            {
                continue;
            }

            if (IsSection(line, LEGEND_SECTION))
            {
                isLegendSection = true;
                legend.clear();
                continue;
            }
            if (IsSection(line, MAP_SECTION))
            {
                isLegendSection = false;
                continue;
            }

            if (isLegendSection)
            {
                ReadLegendLine(line, lineNumber, legend);
                continue;
            }

            ReadMapLine(line, legend, levelData);
        }

        file.close();

        levelData.height = (int)levelData.tiles.size();
        if (levelData.height == 0)
        {
            LOG_ERROR("Level file is empty: " + filePath);
            throw std::runtime_error("Level file has no tiles: " + filePath);
        }

        LOG_INFO("Level loaded: " + filePath + ", size " + std::to_string(levelData.width) + "x" + std::to_string(levelData.height)
            + ", legend entries " + std::to_string(legend.size()));
        return levelData;
    }

    bool LevelLoader::IsSection(const std::string& line, const std::string& sectionName)
    {
        return Trim(line) == "[" + sectionName + "]";
    }

    void LevelLoader::ReadLegendLine(const std::string& line, int lineNumber, Legend& legend)
    {
        char symbol = line.front();

        size_t nameStart = line.find_first_not_of(WHITESPACE, 1);
        if (nameStart == std::string::npos)
        {
            LOG_ERROR("Level legend line " + std::to_string(lineNumber) + " has no tile type name");
            throw std::runtime_error("Level legend line has no tile type name");
        }

        std::string name = Trim(line.substr(nameStart));

        TileType tileType;
        if (!TryGetTileType(name, tileType))
        {
            LOG_ERROR("Unknown tile type in level legend, line " + std::to_string(lineNumber) + ": " + name);
            throw std::runtime_error("Unknown tile type in level legend: " + name);
        }

        legend[symbol] = tileType;
    }

    void LevelLoader::ReadMapLine(const std::string& line, const Legend& legend, LevelData& levelData)
    {
        int row = (int)levelData.tiles.size();

        std::vector<TileType> tiles;
        tiles.reserve(line.size());

        for (int column = 0; column < (int)line.size(); column++)
        {
            char symbol = line[column];

            auto tile = legend.find(symbol);
            if (tile != legend.end())
            {
                tiles.push_back(tile->second);
                continue;
            }

            if (symbol != EMPTY_SYMBOL)
            {
                LOG_WARN(std::string("Unknown level symbol '") + symbol + "' at "
                    + std::to_string(column) + ";" + std::to_string(row));
            }

            tiles.push_back(TileType::Empty);
        }

        if ((int)tiles.size() > levelData.width)
        {
            levelData.width = (int)tiles.size();
        }
        levelData.tiles.push_back(tiles);
    }

    bool LevelLoader::TryGetTileType(const std::string& name, TileType& tileType)
    {
        for (const auto& tileTypeName : TILE_TYPE_NAMES)
        {
            if (name == tileTypeName.name)
            {
                tileType = tileTypeName.type;
                return true;
            }
        }

        return false;
    }

    const LevelLoader::Legend& LevelLoader::GetDefaultLegend()
    {
        static const Legend defaultLegend = {
            {'#', TileType::Wall},
            {'.', TileType::Floor},
            {'@', TileType::PlayerSpawn},
            {'g', TileType::GruntSpawn},
            {'a', TileType::AssaultSpawn},
            {'s', TileType::ShieldSpawn},
            {'h', TileType::HeavySpawn},
            {'r', TileType::RadioSpawn},
            {'b', TileType::BossSpawn}
        };

        return defaultLegend;
    }

    std::string LevelLoader::Trim(const std::string& line)
    {
        size_t first = line.find_first_not_of(WHITESPACE);
        if (first == std::string::npos)
        {
            return "";
        }

        size_t last = line.find_last_not_of(WHITESPACE);
        return line.substr(first, last - first + 1);
    }
}
