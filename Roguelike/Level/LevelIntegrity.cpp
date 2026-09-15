#include "LevelIntegrity.h"
#include "ItemCatalog.h"
#include <algorithm>
#include <map>
#include <set>
#include <vector>

namespace RoguelikeGame
{
    namespace
    {
        constexpr int DESCRIBE_LIMIT = 12;

        struct Cell
        {
            int column = 0;
            int row = 0;
        };

        bool IsSolid(TileType tile)
        {
            return tile == TileType::Wall || tile == TileType::Empty;
        }

        bool IsGate(TileType tile)
        {
            return tile == TileType::Door;
        }

        int HeightOf(const LevelData& levelData)
        {
            return static_cast<int>(levelData.tiles.size());
        }

        std::vector<Cell> CellsOf(const LevelData& levelData, TileType tile)
        {
            std::vector<Cell> found;
            for (int row = 0; row < HeightOf(levelData); row++)
            {
                const std::vector<TileType>& line = levelData.tiles[row];
                for (int column = 0; column < static_cast<int>(line.size()); column++)
                {
                    if (line[column] == tile)
                    {
                        found.push_back({column, row});
                    }
                }
            }

            return found;
        }

        std::map<long long, std::string> DoorsByCell(const LevelData& levelData)
        {
            std::map<long long, std::string> doors;
            for (const DoorPlacement& door : levelData.doors)
            {
                doors[static_cast<long long>(door.row) * levelData.width + door.column] = door.doorId;
            }

            return doors;
        }

        std::vector<char> Walk(const LevelData& levelData, const Cell& start,
            const std::map<long long, std::string>& doors, const std::set<std::string>& opened)
        {
            int width = levelData.width;
            int height = HeightOf(levelData);
            std::vector<char> reached(static_cast<std::size_t>(width) * height, 0);

            if (width <= 0 || height <= 0)
            {
                return reached;
            }

            std::vector<Cell> wave = {start};
            reached[static_cast<std::size_t>(start.row) * width + start.column] = 1;

            while (!wave.empty())
            {
                Cell cell = wave.back();
                wave.pop_back();

                const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
                for (const auto& step : steps)
                {
                    int column = cell.column + step[0];
                    int row = cell.row + step[1];
                    if (column < 0 || row < 0 || column >= width || row >= height)
                    {
                        continue;
                    }

                    std::size_t index = static_cast<std::size_t>(row) * width + column;
                    if (reached[index] != 0)
                    {
                        continue;
                    }

                    TileType tile = TileAt(levelData, column, row);
                    if (IsSolid(tile))
                    {
                        continue;
                    }

                    if (IsGate(tile))
                    {
                        auto door = doors.find(static_cast<long long>(index));
                        if (door == doors.end() || opened.count(door->second) == 0)
                        {
                            continue;
                        }
                    }

                    reached[index] = 1;
                    wave.push_back({column, row});
                }
            }

            return reached;
        }

        std::set<std::string> OpenedByReachedKeys(const LevelData& levelData, const ItemCatalog& items,
            const std::vector<char>& reached)
        {
            std::set<std::string> opened;
            for (const ItemPlacement& placement : levelData.items)
            {
                std::size_t index = static_cast<std::size_t>(placement.row) * levelData.width + placement.column;
                if (index >= reached.size() || reached[index] == 0)
                {
                    continue;
                }

                const ItemDefinition* item = items.Find(placement.itemId);
                if (item != nullptr && item->effect.kind == ItemEffectKind::Unlock)
                {
                    opened.insert(item->effect.target);
                }
            }

            return opened;
        }

        bool TouchesReached(const LevelData& levelData, const std::vector<char>& reached, int column, int row)
        {
            const int steps[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
            for (const auto& step : steps)
            {
                int nextColumn = column + step[0];
                int nextRow = row + step[1];
                if (nextColumn < 0 || nextRow < 0 || nextColumn >= levelData.width || nextRow >= HeightOf(levelData))
                {
                    continue;
                }

                std::size_t index = static_cast<std::size_t>(nextRow) * levelData.width + nextColumn;
                if (index < reached.size() && reached[index] != 0)
                {
                    return true;
                }
            }

            return false;
        }

        void CheckBorder(const LevelData& levelData, LevelReport& report)
        {
            int width = levelData.width;
            int height = HeightOf(levelData);

            for (int row = 0; row < height; row++)
            {
                for (int column = 0; column < width; column++)
                {
                    bool isBorder = row == 0 || row == height - 1 || column == 0 || column == width - 1;
                    if (!isBorder || IsSolid(TileAt(levelData, column, row)))
                    {
                        continue;
                    }

                    report.issues.push_back({LevelFault::OpenBorder, column, row, ""});
                }
            }
        }

        bool FindStart(const LevelData& levelData, LevelReport& report, Cell& start)
        {
            std::vector<Cell> entrances = CellsOf(levelData, TileType::Entrance);
            std::vector<Cell> spawns = CellsOf(levelData, TileType::PlayerSpawn);
            const std::vector<Cell>& starts = entrances.empty() ? spawns : entrances;

            if (starts.empty())
            {
                report.issues.push_back({LevelFault::NoStart, 0, 0, ""});
                return false;
            }

            if (starts.size() > 1u)
            {
                report.issues.push_back({LevelFault::ManyStarts, starts[1].column, starts[1].row, ""});
            }

            start = starts.front();
            return true;
        }

        void CheckDoors(const LevelData& levelData, const std::set<std::string>& opened,
            const std::vector<char>& reached, LevelReport& report)
        {
            std::set<std::string> reported;
            for (const DoorPlacement& door : levelData.doors)
            {
                if (opened.count(door.doorId) != 0 || reported.count(door.doorId) != 0)
                {
                    continue;
                }

                if (!TouchesReached(levelData, reached, door.column, door.row))
                {
                    continue;
                }

                reported.insert(door.doorId);
                report.issues.push_back({LevelFault::LockedOut, door.column, door.row, door.doorId});
            }
        }

        void CheckReached(const LevelData& levelData, const std::vector<char>& reached, LevelReport& report)
        {
            for (int row = 0; row < HeightOf(levelData); row++)
            {
                for (int column = 0; column < levelData.width; column++)
                {
                    TileType tile = TileAt(levelData, column, row);
                    if (IsSolid(tile) || IsGate(tile))
                    {
                        continue;
                    }

                    std::size_t index = static_cast<std::size_t>(row) * levelData.width + column;
                    if (index < reached.size() && reached[index] == 0)
                    {
                        report.issues.push_back({LevelFault::Unreachable, column, row, ""});
                    }
                }
            }
        }

        void CheckExit(const LevelData& levelData, const std::vector<char>& reached, LevelReport& report)
        {
            std::vector<Cell> exits = CellsOf(levelData, TileType::Exit);
            if (exits.empty())
            {
                if (!levelData.info.nextLevelId.empty())
                {
                    report.issues.push_back({LevelFault::NoExit, 0, 0, levelData.info.nextLevelId});
                }

                return;
            }

            for (const Cell& exit : exits)
            {
                std::size_t index = static_cast<std::size_t>(exit.row) * levelData.width + exit.column;
                if (index >= reached.size() || reached[index] == 0)
                {
                    report.issues.push_back({LevelFault::ExitUnreachable, exit.column, exit.row, ""});
                }
            }
        }
    }

    const char* NameOf(LevelFault fault)
    {
        for (const FaultName& entry : FAULT_NAMES)
        {
            if (entry.fault == fault)
            {
                return entry.name;
            }
        }

        return "Unknown";
    }

    bool LevelReport::IsClean() const
    {
        return issues.empty();
    }

    int LevelReport::Count(LevelFault fault) const
    {
        int count = 0;
        for (const LevelIssue& issue : issues)
        {
            if (issue.fault == fault)
            {
                count++;
            }
        }

        return count;
    }

    std::string LevelReport::Describe() const
    {
        if (issues.empty())
        {
            return "clean";
        }

        std::string text;
        int shown = 0;
        for (const LevelIssue& issue : issues)
        {
            if (shown == DESCRIBE_LIMIT)
            {
                text += "and " + std::to_string(static_cast<int>(issues.size()) - shown) + " more\n";
                break;
            }

            text += std::string(NameOf(issue.fault)) + " at " + std::to_string(issue.column)
                + ";" + std::to_string(issue.row);
            if (!issue.detail.empty())
            {
                text += " (" + issue.detail + ")";
            }

            text += "\n";
            shown++;
        }

        return text;
    }

    LevelReport CheckLevel(const LevelData& levelData, const ItemCatalog& items)
    {
        LevelReport report;
        if (levelData.width <= 0 || HeightOf(levelData) == 0)
        {
            report.issues.push_back({LevelFault::NoStart, 0, 0, ""});
            return report;
        }

        CheckBorder(levelData, report);

        Cell start;
        if (!FindStart(levelData, report, start))
        {
            return report;
        }

        std::map<long long, std::string> doors = DoorsByCell(levelData);
        std::set<std::string> opened;
        std::vector<char> reached = Walk(levelData, start, doors, opened);

        while (true)
        {
            std::set<std::string> next = OpenedByReachedKeys(levelData, items, reached);
            if (next.size() == opened.size())
            {
                break;
            }

            opened = next;
            reached = Walk(levelData, start, doors, opened);
        }

        CheckDoors(levelData, opened, reached, report);
        CheckReached(levelData, reached, report);
        CheckExit(levelData, reached, report);

        return report;
    }

    LevelReport CheckLevel(const LevelData& levelData)
    {
        return CheckLevel(levelData, ItemCatalog::Empty());
    }
}
