#pragma once

#include <SFML/Graphics/Rect.hpp>
#include "LevelData.h"

namespace RoguelikeGame
{
    constexpr int TILE_FRAME_SIZE = 64;
    constexpr int TILE_FLOOR_ROW = 0;
    constexpr int TILE_WALL_ROW = 1;
    constexpr int TILE_FLOOR_FRAMES = 4;
    constexpr int TILE_WALL_FRAMES = 16;
    constexpr int TILE_ATLAS_COLUMNS = 16;

    constexpr int WALL_NEIGHBOUR_UP = 1;
    constexpr int WALL_NEIGHBOUR_RIGHT = 2;
    constexpr int WALL_NEIGHBOUR_DOWN = 4;
    constexpr int WALL_NEIGHBOUR_LEFT = 8;

    inline bool IsWallAt(const LevelData& level, int column, int row)
    {
        if (row < 0 || row >= static_cast<int>(level.tiles.size()))
        {
            return true;
        }

        if (column < 0 || column >= static_cast<int>(level.tiles[row].size()))
        {
            return true;
        }

        return level.tiles[row][column] == TileType::Wall;
    }

    inline int WallMask(const LevelData& level, int column, int row)
    {
        int mask = 0;

        mask |= IsWallAt(level, column, row - 1) ? WALL_NEIGHBOUR_UP : 0;
        mask |= IsWallAt(level, column + 1, row) ? WALL_NEIGHBOUR_RIGHT : 0;
        mask |= IsWallAt(level, column, row + 1) ? WALL_NEIGHBOUR_DOWN : 0;
        mask |= IsWallAt(level, column - 1, row) ? WALL_NEIGHBOUR_LEFT : 0;

        return mask;
    }

    inline int WallFrame(int mask)
    {
        return mask < 0 ? 0 : mask % TILE_WALL_FRAMES;
    }

    inline unsigned int TileHash(int column, int row)
    {
        unsigned int value = static_cast<unsigned int>(column) * 374761393u + static_cast<unsigned int>(row) * 668265263u;

        value ^= value >> 13;
        value *= 1274126177u;
        value ^= value >> 16;

        return value;
    }

    inline int FloorFrame(int column, int row)
    {
        return static_cast<int>(TileHash(column, row) % TILE_FLOOR_FRAMES);
    }

    inline sf::IntRect TileFrameRect(int row, int column)
    {
        int safeColumn = column < 0 ? 0 : column % TILE_ATLAS_COLUMNS;
        int safeRow = row < 0 ? 0 : row;

        return {safeColumn * TILE_FRAME_SIZE, safeRow * TILE_FRAME_SIZE, TILE_FRAME_SIZE, TILE_FRAME_SIZE};
    }

    inline sf::IntRect TileFrameFor(const LevelData& level, int column, int row)
    {
        if (level.tiles[row][column] == TileType::Wall)
        {
            return TileFrameRect(TILE_WALL_ROW, WallFrame(WallMask(level, column, row)));
        }

        return TileFrameRect(TILE_FLOOR_ROW, FloorFrame(column, row));
    }
}
