#include "LevelLoader.h"
#include "EnemyCatalog.h"
#include <LoggerRegistry.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace RoguelikeGame
{
    const std::string LEGEND_SECTION = "legend";
    const std::string MAP_SECTION = "map";
    const std::string LEVEL_SECTION = "level";
    const std::string ITEM_PREFIX = "Item:";
    const std::string PROP_PREFIX = "Prop:";
    const std::string PATROL_PREFIX = "Patrol:";
    const std::string WATCH_PREFIX = "Watch:";
    constexpr char COMMENT_SYMBOL = ';';
    constexpr char EMPTY_SYMBOL = ' ';
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

        return Parse(file, filePath);
    }

    LevelData LevelLoader::Parse(std::istream& input, const std::string& sourceName)
    {
        LevelData levelData;
        Legend legend = GetDefaultLegend();
        bool isLegendSection = false;
        bool isLevelSection = false;

        std::string line;
        int lineNumber = 0;
        while (std::getline(input, line))
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

            if (IsSection(line, LEVEL_SECTION))
            {
                isLevelSection = true;
                isLegendSection = false;
                continue;
            }
            if (IsSection(line, LEGEND_SECTION))
            {
                isLegendSection = true;
                isLevelSection = false;
                legend.clear();
                continue;
            }
            if (IsSection(line, MAP_SECTION))
            {
                isLegendSection = false;
                isLevelSection = false;
                continue;
            }

            if (isLevelSection)
            {
                ReadInfoLine(line, lineNumber, levelData.info);
                continue;
            }

            if (isLegendSection)
            {
                ReadLegendLine(line, lineNumber, legend);
                continue;
            }

            ReadMapLine(line, legend, levelData);
        }

        levelData.height = static_cast<int>(levelData.tiles.size());
        Validate(levelData, sourceName);

        LOG_INFO("Level loaded: " + sourceName + ", size " + std::to_string(levelData.width) + "x" + std::to_string(levelData.height)
            + ", legend entries " + std::to_string(legend.size()));
        return levelData;
    }

    void LevelLoader::ReadInfoLine(const std::string& line, int lineNumber, LevelInfo& info)
    {
        std::istringstream stream(line);
        std::string key;
        stream >> key;

        if (key == "title")
        {
            std::string rest;
            std::getline(stream, rest);
            info.title = Trim(rest);
            return;
        }

        if (key == "next")
        {
            stream >> info.nextLevelId;
            return;
        }

        if (key == "boss")
        {
            if (!(stream >> info.boss.bossId))
            {
                LOG_ERROR("Level boss has no id, line " + std::to_string(lineNumber));
                throw std::runtime_error("Level boss has no id");
            }

            if (!(stream >> info.boss.healthScale))
            {
                info.boss.healthScale = 1.f;
            }

            if (!(stream >> info.boss.damageScale))
            {
                info.boss.damageScale = 1.f;
            }

            if (info.boss.healthScale <= 0.f || info.boss.damageScale <= 0.f)
            {
                LOG_ERROR("Level boss scales must be positive, line " + std::to_string(lineNumber));
                throw std::runtime_error("Level boss scales must be positive");
            }

            return;
        }

        LOG_WARN("Unknown level field at line " + std::to_string(lineNumber) + ": " + key);
    }

    void LevelLoader::Validate(const LevelData& levelData, const std::string& sourceName)
    {
        if (levelData.height == 0)
        {
            LOG_ERROR("Level file is empty: " + sourceName);
            throw std::runtime_error("Level file has no tiles: " + sourceName);
        }

        int exits = CountTiles(levelData, TileType::Exit);
        if (!levelData.info.nextLevelId.empty() && exits == 0)
        {
            LOG_ERROR("Level " + sourceName + " leads to " + levelData.info.nextLevelId + " but has no exit tile");
            throw std::runtime_error("Level has no exit: " + sourceName);
        }

        if (exits > 0 && levelData.info.nextLevelId.empty())
        {
            LOG_WARN("Level " + sourceName + " has an exit but no next level");
        }
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

        if (name.compare(0, ITEM_PREFIX.size(), ITEM_PREFIX) == 0)
        {
            std::string itemId = Trim(name.substr(ITEM_PREFIX.size()));
            if (itemId.empty())
            {
                LOG_ERROR("Level legend line " + std::to_string(lineNumber) + " has no item id");
                throw std::runtime_error("Level legend line has no item id");
            }

            legend[symbol] = {TileType::Floor, itemId, "", "", 0, false};
            return;
        }

        if (name.compare(0, PROP_PREFIX.size(), PROP_PREFIX) == 0)
        {
            std::string propId = Trim(name.substr(PROP_PREFIX.size()));
            if (propId.empty())
            {
                LOG_ERROR("Level legend line " + std::to_string(lineNumber) + " has no prop id");
                throw std::runtime_error("Level legend line has no prop id");
            }

            legend[symbol] = {TileType::Floor, "", propId, "", 0, false};
            return;
        }

        bool isWatch = name.compare(0, WATCH_PREFIX.size(), WATCH_PREFIX) == 0;
        if (isWatch || name.compare(0, PATROL_PREFIX.size(), PATROL_PREFIX) == 0)
        {
            std::string routeId = Trim(name.substr(isWatch ? WATCH_PREFIX.size() : PATROL_PREFIX.size()));
            if (routeId.empty())
            {
                LOG_ERROR("Level legend line " + std::to_string(lineNumber) + " has no patrol route id");
                throw std::runtime_error("Level legend line has no patrol route id");
            }

            int order = 0;
            for (const auto& entry : legend)
            {
                if (!entry.second.patrolId.empty())
                {
                    order++;
                }
            }

            legend[symbol] = {TileType::Floor, "", "", routeId, order, isWatch};
            return;
        }

        TileType tileType;
        if (!TryGetTileType(name, tileType))
        {
            LOG_ERROR("Unknown tile type in level legend, line " + std::to_string(lineNumber) + ": " + name);
            throw std::runtime_error("Unknown tile type in level legend: " + name);
        }

        legend[symbol] = {tileType, "", "", "", 0, false};
    }

    void LevelLoader::ReadMapLine(const std::string& line, const Legend& legend, LevelData& levelData)
    {
        int row = static_cast<int>(levelData.tiles.size());

        std::vector<TileType> tiles;
        tiles.reserve(line.size());

        for (int column = 0; column < static_cast<int>(line.size()); column++)
        {
            char symbol = line[column];

            auto tile = legend.find(symbol);
            if (tile != legend.end())
            {
                if (!tile->second.itemId.empty())
                {
                    levelData.items.push_back({column, row, tile->second.itemId});
                }

                if (!tile->second.propId.empty())
                {
                    levelData.props.push_back({column, row, tile->second.propId});
                }

                if (!tile->second.patrolId.empty())
                {
                    levelData.patrols.push_back({column, row, tile->second.patrolId, tile->second.patrolOrder, tile->second.patrolWatch});
                }

                tiles.push_back(tile->second.tile);
                continue;
            }

            if (symbol != EMPTY_SYMBOL)
            {
                LOG_WARN(std::string("Unknown level symbol '") + symbol + "' at "
                    + std::to_string(column) + ";" + std::to_string(row));
            }

            tiles.push_back(TileType::Empty);
        }

        if (static_cast<int>(tiles.size()) > levelData.width)
        {
            levelData.width = static_cast<int>(tiles.size());
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

        for (const EnemyDefinition& enemy : ENEMIES)
        {
            if (name == enemy.tileName)
            {
                tileType = enemy.tile;
                return true;
            }
        }

        return false;
    }

    const LevelLoader::Legend& LevelLoader::GetDefaultLegend()
    {
        static const Legend defaultLegend = []()
        {
            Legend legend = {
                {'#', {TileType::Wall, ""}},
                {'.', {TileType::Floor, ""}},
                {'@', {TileType::PlayerSpawn, ""}}
            };

            for (const EnemyDefinition& enemy : ENEMIES)
            {
                legend[enemy.levelSymbol] = {enemy.tile, ""};
            }

            return legend;
        }();

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
