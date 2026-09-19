#include "ActAssembler.h"
#include "RoomTransform.h"
#include "LevelLoader.h"
#include <map>
#include <LoggerRegistry.h>
#include <algorithm>
#include <stdexcept>

namespace RoguelikeGame
{
    namespace
    {
        struct PlacedRoom
        {
            std::string id;
            int column = 0;
            int row = 0;
            LevelData layout;

            int Right() const { return column + layout.width - 1; }
            int Bottom() const { return row + layout.height - 1; }
        };

        bool IsOpen(const LevelData& room, int column, int row)
        {
            if (row < 0 || row >= static_cast<int>(room.tiles.size()))
            {
                return false;
            }
            if (column < 0 || column >= static_cast<int>(room.tiles[row].size()))
            {
                return false;
            }

            TileType tile = room.tiles[row][column];

            return tile != TileType::Wall && tile != TileType::Empty;
        }

        std::string Where(const PlacedRoom& room, int column, int row)
        {
            return room.id + " at " + std::to_string(column) + ";" + std::to_string(row);
        }

        void CheckSeam(const PlacedRoom& first, const PlacedRoom& second)
        {
            bool isSideBySide = first.Right() + 1 == second.column;
            bool isStacked = first.Bottom() + 1 == second.row;

            if (!isSideBySide && !isStacked)
            {
                return;
            }

            int from = isSideBySide ? std::max(first.row, second.row) : std::max(first.column, second.column);
            int to = isSideBySide ? std::min(first.Bottom(), second.Bottom()) : std::min(first.Right(), second.Right());

            for (int along = from; along <= to; along++)
            {
                int firstColumn = isSideBySide ? first.layout.width - 1 : along - first.column;
                int firstRow = isSideBySide ? along - first.row : first.layout.height - 1;
                int secondColumn = isSideBySide ? 0 : along - second.column;
                int secondRow = isSideBySide ? along - second.row : 0;

                bool isFirstOpen = IsOpen(first.layout, firstColumn, firstRow);
                bool isSecondOpen = IsOpen(second.layout, secondColumn, secondRow);

                if (isFirstOpen == isSecondOpen)
                {
                    continue;
                }

                const PlacedRoom& open = isFirstOpen ? first : second;
                const PlacedRoom& closed = isFirstOpen ? second : first;
                int openColumn = isFirstOpen ? first.column + firstColumn : second.column + secondColumn;
                int openRow = isFirstOpen ? first.row + firstRow : second.row + secondRow;

                LOG_ERROR("Act seam does not match: " + Where(open, openColumn, openRow)
                    + " opens into a wall of " + closed.id);
                throw std::runtime_error("Act seam does not match: " + open.id + " opens into a wall of " + closed.id);
            }
        }
    }

    LevelData AssembleAct(const ActPlan& plan, const RoomSource& rooms)
    {
        std::vector<PlacedRoom> placed;
        placed.reserve(plan.rooms.size());

        for (const RoomPlacement& wanted : plan.rooms)
        {
            const LevelData* room = rooms(wanted.roomId);
            if (room == nullptr)
            {
                LOG_ERROR("Act needs a room that is not in the library: " + wanted.roomId);
                throw std::runtime_error("Unknown room in act: " + wanted.roomId);
            }

            PlacedRoom entry;
            entry.id = wanted.roomId;
            entry.column = wanted.column;
            entry.row = wanted.row;
            entry.layout = TransformRoom(*room, wanted.quarters, wanted.isMirrored);
            placed.push_back(std::move(entry));
        }

        for (std::size_t first = 0u; first < placed.size(); first++)
        {
            for (std::size_t second = 0u; second < placed.size(); second++)
            {
                if (first != second)
                {
                    CheckSeam(placed[first], placed[second]);
                }
            }
        }

        LevelData act;
        act.info = plan.info;
        act.waves = plan.waves;
        act.pursuit = plan.pursuit;

        for (const PlacedRoom& room : placed)
        {
            act.width = std::max(act.width, room.Right() + 1);
            act.height = std::max(act.height, room.Bottom() + 1);
        }

        act.tiles.assign(act.height, std::vector<TileType>(act.width, TileType::Empty));

        // Верхний слой заводим, только если хоть одна комната его несёт:
        // пустая сетка на всю карту заняла бы память впустую.
        bool hasOverlay = false;
        for (const PlacedRoom& room : placed)
        {
            hasOverlay = hasOverlay || !room.layout.overlay.empty();
        }

        if (hasOverlay)
        {
            act.overlay.assign(act.height, std::vector<TileType>(act.width, TileType::Empty));
        }

        for (const PlacedRoom& room : placed)
        {
            for (int row = 0; row < room.layout.height; row++)
            {
                for (int column = 0; column < room.layout.width; column++)
                {
                    act.tiles[room.row + row][room.column + column] = room.layout.tiles[row][column];
                }
            }

            if (hasOverlay)
            {
                for (int row = 0; row < room.layout.height; row++)
                {
                    for (int column = 0; column < room.layout.width; column++)
                    {
                        act.overlay[room.row + row][room.column + column] = GridAt(room.layout.overlay, column, row);
                    }
                }
            }

            for (const ItemPlacement& item : room.layout.items)
            {
                act.items.push_back({room.column + item.column, room.row + item.row, item.itemId});
            }

            for (const PropPlacement& prop : room.layout.props)
            {
                act.props.push_back({room.column + prop.column, room.row + prop.row, prop.propId, prop.angle});
            }

            for (const DoorPlacement& door : room.layout.doors)
            {
                act.doors.push_back({room.column + door.column, room.row + door.row, door.doorId});
            }

            for (auto list : FIXTURE_LISTS)
            {
                for (const FixturePlacement& fixture : room.layout.*list)
                {
                    FixturePlacement moved = fixture;
                    moved.column = room.column + fixture.column;
                    moved.row = room.row + fixture.row;
                    (act.*list).push_back(std::move(moved));
                }
            }

            for (const PatrolPoint& point : room.layout.patrols)
            {
                act.patrols.push_back({room.column + point.column, room.row + point.row,
                    room.id + ":" + point.routeId, point.order, point.isWatch});
            }

            std::string place = room.id + "@" + std::to_string(room.column) + ";" + std::to_string(room.row);

            // Засада ссылается на зону своей комнаты, а зоны при сборке
            // переименовываются - значит, и ссылка переименовывается с ними.
            for (const AmbushSpec& ambush : room.layout.ambushes)
            {
                AmbushSpec moved = ambush;
                moved.zoneId = ambush.zoneId == WHOLE_ROOM ? place : place + ":" + ambush.zoneId;
                act.ambushes.push_back(std::move(moved));
            }

            if (room.layout.zones.empty())
            {
                act.zones.push_back({room.column, room.row, place});
                act.zones.push_back({room.Right(), room.Bottom(), place});
                continue;
            }

            for (const ZonePlacement& zone : room.layout.zones)
            {
                act.zones.push_back({room.column + zone.column, room.row + zone.row, place + ":" + zone.zoneId});
            }
        }

        LOG_INFO("Act assembled: " + std::to_string(placed.size()) + " rooms, size "
            + std::to_string(act.width) + "x" + std::to_string(act.height));

        return act;
    }
}

namespace RoguelikeGame
{
    LevelData LoadAct(const std::string& filePath)
    {
        ActPlan plan = ActLoader::Load(filePath);

        std::map<std::string, LevelData> library;
        for (const auto& entry : plan.library)
        {
            library[entry.first] = LevelLoader::Load(entry.second);
        }

        return AssembleAct(plan, [&library](const std::string& id) -> const LevelData*
        {
            auto found = library.find(id);

            return found == library.end() ? nullptr : &found->second;
        });
    }
}
