#include "LevelLoader.h"
#include "FogFormat.h"
#include "PursuitFormat.h"
#include "EnemyCatalog.h"
#include <LoggerRegistry.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace RoguelikeGame
{
    const std::string LEGEND_SECTION = "legend";
    const std::string MAP_SECTION = "map";
    const std::string WAVES_SECTION = "waves";
    const std::string PURSUIT_SECTION = "pursuit";
    const std::string OVERLAY_SECTION = "overlay";
    const std::string AMBUSH_SECTION = "ambush";
    const std::string WAVE_KEYWORD = "wave";
    const std::string STYLE_KEYWORD = "style";
    const std::string LEVEL_SECTION = "level";
    const std::string ITEM_PREFIX = "Item:";
    const std::string PROP_PREFIX = "Prop:";
    const std::string PATROL_PREFIX = "Patrol:";
    const std::string WATCH_PREFIX = "Watch:";
    const std::string DOOR_PREFIX = "Door:";
    const std::string ZONE_PREFIX = "Zone:";
    const std::string SWITCH_PREFIX = "Switch:";
    const std::string PLATE_PREFIX = "Plate:";
    const std::string HATCH_PREFIX = "Hatch:";
    const std::string ESCAPE_PREFIX = "Escape:";
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
        Section section = Section::Map;

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

            // Своя легенда дополняет стандартную, а не затирает: забытый символ
            // проваливался в Gap, который даже обзор не закрывает.
            // Объявленный символ по-прежнему перекрывает стандартный - запись идёт позже.
            const std::pair<const std::string*, Section> sections[] = {
                {&LEVEL_SECTION, Section::Level},
                {&LEGEND_SECTION, Section::Legend},
                {&WAVES_SECTION, Section::Waves},
                {&AMBUSH_SECTION, Section::Ambush},
                {&PURSUIT_SECTION, Section::Pursuit},
                {&OVERLAY_SECTION, Section::Overlay},
                {&MAP_SECTION, Section::Map},
            };

            bool isHeader = false;
            for (const auto& known : sections)
            {
                if (IsSection(line, *known.first))
                {
                    section = known.second;
                    isHeader = true;
                    break;
                }
            }

            if (isHeader)
            {
                continue;
            }

            switch (section)
            {
            case Section::Level:
                ReadInfoLine(line, lineNumber, levelData.info);
                break;
            case Section::Legend:
                ReadLegendLine(line, lineNumber, legend);
                break;
            case Section::Waves:
                ReadWaveLine(line, lineNumber, legend, levelData);
                break;
            case Section::Ambush:
                ReadAmbushLine(line, lineNumber, legend, levelData);
                break;
            case Section::Pursuit:
                ReadPursuitLine(line, lineNumber, levelData.pursuit);
                break;
            case Section::Overlay:
                ReadOverlayLine(line, legend, levelData);
                break;
            case Section::Map:
                ReadMapLine(line, legend, levelData);
                break;
            }
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

        if (key == STYLE_KEYWORD)
        {
            ReadFightStyle(stream, lineNumber, info.style);
            return;
        }

        if (key == FOG_KEYWORD)
        {
            ReadFogRadius(stream, lineNumber, info.fogRadius);
            return;
        }

        if (key == "kind")
        {
            stream >> info.kind;
            return;
        }

        if (key == "tileset")
        {
            stream >> info.tileset;
            return;
        }

        if (key == "music")
        {
            stream >> info.music;
            return;
        }

        if (key == "ambient")
        {
            stream >> info.ambient;
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

            stream >> info.boss.drop;

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


    /**
    *	Строка волны: wave <пауза> <символ><сколько> ...
    *	Символы берутся из легенды уровня и обязаны быть вражьими.
    */
    // Группы «символ и сколько» одинаковы у волны и у засады: разбор общий.
    std::vector<WaveEntry> LevelLoader::ReadWaveGroups(std::istringstream& stream, int lineNumber, const Legend& legend)
    {
        std::vector<WaveEntry> entries;
        std::string group;

        while (stream >> group)
        {
            if (group.size() < 2)
            {
                LOG_ERROR("Wave line " + std::to_string(lineNumber) + " has a group without a count: " + group);
                throw std::runtime_error("Wave group has no count");
            }

            auto found = legend.find(group.front());
            if (found == legend.end())
            {
                LOG_ERROR("Wave line " + std::to_string(lineNumber) + " uses a symbol that is not in the legend: " + group);
                throw std::runtime_error("Unknown wave symbol");
            }

            if (FindEnemyConfig(found->second.tile) == nullptr)
            {
                LOG_ERROR("Wave line " + std::to_string(lineNumber) + " asks for something that is not an enemy: " + group);
                throw std::runtime_error("Wave symbol is not an enemy");
            }

            int count = 0;
            try
            {
                count = std::stoi(group.substr(1));
            }
            catch (const std::exception&)
            {
                count = 0;
            }

            if (count <= 0)
            {
                LOG_ERROR("Wave line " + std::to_string(lineNumber) + " asks for a non-positive count: " + group);
                throw std::runtime_error("Wave group count must be positive");
            }

            entries.push_back({found->second.tile, count});
        }

        if (entries.empty())
        {
            LOG_ERROR("Wave line " + std::to_string(lineNumber) + " is empty");
            throw std::runtime_error("Wave has no enemies");
        }

        return entries;
    }

    void LevelLoader::ReadAmbushLine(const std::string& line, int lineNumber, const Legend& legend, LevelData& levelData)
    {
        std::istringstream stream(Trim(line));

        AmbushSpec ambush;
        if (!(stream >> ambush.zoneId))
        {
            LOG_ERROR("Ambush line " + std::to_string(lineNumber) + " has no zone");
            throw std::runtime_error("Ambush line has no zone");
        }

        if (!(stream >> ambush.delay) || ambush.delay < 0.f)
        {
            LOG_ERROR("Ambush line " + std::to_string(lineNumber) + " has no delay");
            throw std::runtime_error("Ambush line has no delay");
        }

        ambush.entries = ReadWaveGroups(stream, lineNumber, legend);
        levelData.ambushes.push_back(std::move(ambush));
    }

    void LevelLoader::ReadWaveLine(const std::string& line, int lineNumber, const Legend& legend, LevelData& levelData)
    {
        std::istringstream stream(Trim(line));
        std::string keyword;
        stream >> keyword;

        if (keyword == STYLE_KEYWORD)
        {
            ReadFightStyle(stream, lineNumber, levelData.wavesStyle);
            return;
        }

        if (keyword != WAVE_KEYWORD)
        {
            LOG_ERROR("Wave line " + std::to_string(lineNumber) + " does not start with " + WAVE_KEYWORD);
            throw std::runtime_error("Unknown wave line");
        }

        WaveSpec wave;
        if (!(stream >> wave.delay) || wave.delay < 0.f)
        {
            LOG_ERROR("Wave line " + std::to_string(lineNumber) + " has no delay");
            throw std::runtime_error("Wave line has no delay");
        }

        wave.entries = ReadWaveGroups(stream, lineNumber, legend);

        levelData.waves.push_back(std::move(wave));
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

            legend[symbol] = {TileType::Floor, itemId, "", "", 0, false, "", ""};
            return;
        }

        if (name.compare(0, PROP_PREFIX.size(), PROP_PREFIX) == 0)
        {
            std::string propId = Trim(name.substr(PROP_PREFIX.size()));
            float propAngle = 0.f;

            std::size_t turn = propId.find('@');
            if (turn != std::string::npos)
            {
                try
                {
                    propAngle = std::stof(propId.substr(turn + 1));
                }
                catch (const std::exception&)
                {
                    LOG_ERROR("Level legend line " + std::to_string(lineNumber) + " has a bad prop angle");
                    throw std::runtime_error("Level legend line has a bad prop angle");
                }

                propId = Trim(propId.substr(0, turn));
            }

            if (propId.empty())
            {
                LOG_ERROR("Level legend line " + std::to_string(lineNumber) + " has no prop id");
                throw std::runtime_error("Level legend line has no prop id");
            }

            LegendEntry entry;
            entry.tile = TileType::Floor;
            entry.propId = propId;
            entry.propAngle = propAngle;

            legend[symbol] = entry;
            return;
        }

        if (name.compare(0, DOOR_PREFIX.size(), DOOR_PREFIX) == 0)
        {
            std::string doorId = Trim(name.substr(DOOR_PREFIX.size()));
            if (doorId.empty())
            {
                LOG_ERROR("Level legend line " + std::to_string(lineNumber) + " has no door id");
                throw std::runtime_error("Level legend line has no door id");
            }

            legend[symbol] = {TileType::Door, "", "", "", 0, false, doorId, ""};
            return;
        }

        // Фикстуры разбираются одинаково и отличаются только тем, в какое поле лягут.
        const std::pair<const std::string*, std::string LegendEntry::*> fixtures[] = {
            {&HATCH_PREFIX, &LegendEntry::hatchId},
            {&ESCAPE_PREFIX, &LegendEntry::escapeId},
            {&SWITCH_PREFIX, &LegendEntry::leverId},
            {&PLATE_PREFIX, &LegendEntry::plateId},
        };

        for (const auto& kind : fixtures)
        {
            const std::string& prefix = *kind.first;
            if (name.compare(0, prefix.size(), prefix) != 0)
            {
                continue;
            }

            std::string fixtureId = Trim(name.substr(prefix.size()));
            if (fixtureId.empty())
            {
                LOG_ERROR("Level legend line " + std::to_string(lineNumber) + " has no fixture id");
                throw std::runtime_error("Level legend line has no fixture id");
            }

            LegendEntry entry;
            entry.tile = TileType::Floor;
            entry.*kind.second = fixtureId;

            legend[symbol] = entry;
            return;
        }

        if (name.compare(0, ZONE_PREFIX.size(), ZONE_PREFIX) == 0)
        {
            std::string zoneId = Trim(name.substr(ZONE_PREFIX.size()));
            if (zoneId.empty())
            {
                LOG_ERROR("Level legend line " + std::to_string(lineNumber) + " has no zone id");
                throw std::runtime_error("Level legend line has no zone id");
            }

            legend[symbol] = {TileType::Floor, "", "", "", 0, false, "", zoneId};
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

            legend[symbol] = {TileType::Floor, "", "", routeId, order, isWatch, "", ""};
            return;
        }

        TileType tileType;
        if (!TryGetTileType(name, tileType))
        {
            LOG_ERROR("Unknown tile type in level legend, line " + std::to_string(lineNumber) + ": " + name);
            throw std::runtime_error("Unknown tile type in level legend: " + name);
        }

        legend[symbol] = {tileType, "", "", "", 0, false, "", ""};
    }

    /**
    *	Строка верхнего слоя. Символы те же, что и на карте, но кладётся только
    *	рисунок: предметы, двери и прочее из легенды здесь игнорируются - иначе
    *	один символ означал бы разное в зависимости от того, в какую карту попал.
    */
    void LevelLoader::ReadOverlayLine(const std::string& line, const Legend& legend, LevelData& levelData)
    {
        std::vector<TileType> tiles;
        tiles.reserve(line.size());

        for (char symbol : line)
        {
            auto found = legend.find(symbol);
            tiles.push_back(found == legend.end() ? TileType::Empty : found->second.tile);
        }

        levelData.overlay.push_back(std::move(tiles));
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
                    levelData.props.push_back({column, row, tile->second.propId, tile->second.propAngle});
                }

                if (!tile->second.patrolId.empty())
                {
                    levelData.patrols.push_back({column, row, tile->second.patrolId, tile->second.patrolOrder, tile->second.patrolWatch});
                }

                if (!tile->second.doorId.empty())
                {
                    levelData.doors.push_back({column, row, tile->second.doorId});
                }

                if (!tile->second.leverId.empty())
                {
                    levelData.levers.push_back({column, row, tile->second.leverId});
                }

                if (!tile->second.plateId.empty())
                {
                    levelData.plates.push_back({column, row, tile->second.plateId});
                }

                if (!tile->second.escapeId.empty())
                {
                    levelData.escapes.push_back({column, row, tile->second.escapeId});
                }

                if (!tile->second.hatchId.empty())
                {
                    levelData.hatches.push_back({column, row, tile->second.hatchId});
                }

                if (!tile->second.zoneId.empty())
                {
                    levelData.zones.push_back({column, row, tile->second.zoneId});
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
