#include "RoomTransform.h"

namespace RoguelikeGame
{
    namespace
    {
        struct Placement
        {
            int column = 0;
            int row = 0;
        };

        Placement Mirror(Placement cell, int width)
        {
            return {width - 1 - cell.column, cell.row};
        }

        Placement Turn(Placement cell, int width, int height, int quarters)
        {
            for (int turn = 0; turn < quarters; turn++)
            {
                Placement turned = {height - 1 - cell.row, cell.column};
                cell = turned;

                int swapped = width;
                width = height;
                height = swapped;
            }

            return cell;
        }

        Placement Move(Placement cell, int width, int height, int quarters, bool isMirrored)
        {
            if (isMirrored)
            {
                cell = Mirror(cell, width);
            }

            return Turn(cell, width, height, quarters);
        }
    }

    LevelData TransformRoom(const LevelData& room, int quarters, bool isMirrored)
    {
        int turns = ((quarters % ROOM_QUARTERS) + ROOM_QUARTERS) % ROOM_QUARTERS;

        int width = room.width;
        int height = room.height > 0 ? room.height : static_cast<int>(room.tiles.size());
        bool isSideways = turns % 2 != 0;

        LevelData moved;
        moved.info = room.info;
        moved.width = isSideways ? height : width;
        moved.height = isSideways ? width : height;
        moved.tiles.assign(moved.height, std::vector<TileType>(moved.width, TileType::Empty));

        for (int row = 0; row < height; row++)
        {
            int columns = row < static_cast<int>(room.tiles.size()) ? static_cast<int>(room.tiles[row].size()) : 0;
            for (int column = 0; column < columns; column++)
            {
                Placement to = Move({column, row}, width, height, turns, isMirrored);
                moved.tiles[to.row][to.column] = room.tiles[row][column];
            }
        }

        if (!room.overlay.empty())
        {
            moved.overlay.assign(moved.height, std::vector<TileType>(moved.width, TileType::Empty));

            for (int row = 0; row < height; row++)
            {
                int columns = row < static_cast<int>(room.overlay.size()) ? static_cast<int>(room.overlay[row].size()) : 0;
                for (int column = 0; column < columns; column++)
                {
                    Placement to = Move({column, row}, width, height, turns, isMirrored);
                    moved.overlay[to.row][to.column] = room.overlay[row][column];
                }
            }
        }

        for (const ItemPlacement& item : room.items)
        {
            Placement to = Move({item.column, item.row}, width, height, turns, isMirrored);
            moved.items.push_back({to.column, to.row, item.itemId});
        }

        for (const PropPlacement& prop : room.props)
        {
            Placement to = Move({prop.column, prop.row}, width, height, turns, isMirrored);
            moved.props.push_back({to.column, to.row, prop.propId, prop.angle + turns * 90.f});
        }

        for (const PatrolPoint& point : room.patrols)
        {
            Placement to = Move({point.column, point.row}, width, height, turns, isMirrored);
            moved.patrols.push_back({to.column, to.row, point.routeId, point.order, point.isWatch});
        }

        for (const DoorPlacement& door : room.doors)
        {
            Placement to = Move({door.column, door.row}, width, height, turns, isMirrored);
            moved.doors.push_back({to.column, to.row, door.doorId});
        }

        for (const FixturePlacement& lever : room.levers)
        {
            Placement to = Move({lever.column, lever.row}, width, height, turns, isMirrored);
            moved.levers.push_back({to.column, to.row, lever.id});
        }

        for (const FixturePlacement& hatch : room.hatches)
        {
            Placement to = Move({hatch.column, hatch.row}, width, height, turns, isMirrored);
            moved.hatches.push_back({to.column, to.row, hatch.id});
        }

        for (const FixturePlacement& escape : room.escapes)
        {
            Placement to = Move({escape.column, escape.row}, width, height, turns, isMirrored);
            moved.escapes.push_back({to.column, to.row, escape.id});
        }

        for (const ZonePlacement& zone : room.zones)
        {
            Placement to = Move({zone.column, zone.row}, width, height, turns, isMirrored);
            moved.zones.push_back({to.column, to.row, zone.zoneId});
        }

        return moved;
    }
}
