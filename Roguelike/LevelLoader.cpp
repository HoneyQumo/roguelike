#include "LevelLoader.h"
#include <LoggerRegistry.h>
#include <fstream>
#include <stdexcept>

namespace RoguelikeGame
{
    LevelData LevelLoader::Load(const std::string& filePath)
    {
        LevelData levelData;

        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_ERROR("Can't open level file: " + filePath);
            throw std::runtime_error("Level file is not available: " + filePath);
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            if (line.empty() || line.front() == ';')
            {
                continue;
            }

            std::vector<TileType> row;
            row.reserve(line.size());
            for (char symbol : line)
            {
                row.push_back(SymbolToTileType(symbol));
            }

            if ((int)row.size() > levelData.width)
            {
                levelData.width = (int)row.size();
            }
            levelData.tiles.push_back(row);
        }

        file.close();

        levelData.height = (int)levelData.tiles.size();
        if (levelData.height == 0)
        {
            LOG_ERROR("Level file is empty: " + filePath);
            throw std::runtime_error("Level file has no tiles: " + filePath);
        }

        LOG_INFO("Level loaded: " + filePath + ", size " + std::to_string(levelData.width) + "x" + std::to_string(levelData.height));
        return levelData;
    }

    TileType LevelLoader::SymbolToTileType(char symbol)
    {
        switch (symbol)
        {
        case '#':
            return TileType::Wall;
        case '.':
            return TileType::Floor;
        case '@':
            return TileType::PlayerSpawn;
        case 'E':
            return TileType::EnemySpawn;
        case 'R':
            return TileType::RiflemanSpawn;
        default:
            return TileType::Empty;
        }
    }
}
