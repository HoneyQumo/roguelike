#include "LevelBuilder.h"
#include "GameSettings.h"
#include "Enemy.h"
#include "EnemyCatalog.h"
#include "Item.h"
#include "Wall.h"
#include <GameWorld.h>
#include <VertexArrayRendererComponent.h>
#include <LoggerRegistry.h>
#include <cassert>

namespace RoguelikeGame
{
    Level LevelBuilder::Build(const LevelData& levelData, const ItemCatalog& items)
    {
        Level level;

        if (CountTiles(levelData, TileType::PlayerSpawn) == 0)
        {
            LOG_ERROR("Level has no player spawn point, nothing is built");
            return level;
        }

        int tilesCount = BuildTiles(levelData, level);
        int wallsCount = 0;
        int enemiesCount = 0;

        for (int row = 0; row < levelData.height; row++)
        {
            for (int column = 0; column < (int)levelData.tiles[row].size(); column++)
            {
                auto position = TileToWorldPosition(column, row, levelData.height);

                try
                {
                    TileType tile = levelData.tiles[row][column];
                    switch (tile)
                    {
                    case TileType::Wall:
                        level.Add(CreateWall(position));
                        wallsCount++;
                        break;
                    case TileType::PlayerSpawn:
                        if (level.GetPlayerSpawn().has_value())
                        {
                            LOG_WARN("Level has more than one player spawn point, extra one at "
                                + std::to_string(column) + ";" + std::to_string(row) + " is ignored");
                        }
                        else
                        {
                            level.SetPlayerSpawn(position);
                        }
                        break;
                    default:
                        if (const EnemyConfig* config = FindEnemyConfig(tile))
                        {
                            level.Add(CreateEnemy(*config, position));
                            enemiesCount++;
                        }
                        break;
                    }
                }
                catch (const std::exception& exception)
                {
                    LOG_ERROR("Can't spawn tile at " + std::to_string(column) + ";" + std::to_string(row) + ": " + exception.what());
                }
            }
        }

        int itemsCount = BuildItems(levelData, items, level);

        LOG_INFO("Level built: tiles " + std::to_string(tilesCount)
            + ", walls " + std::to_string(wallsCount)
            + ", enemies " + std::to_string(enemiesCount)
            + ", items " + std::to_string(itemsCount));

        return level;
    }

    int LevelBuilder::BuildItems(const LevelData& levelData, const ItemCatalog& items, Level& level)
    {
        int itemsCount = 0;

        for (const ItemPlacement& placement : levelData.items)
        {
            const ItemDefinition* definition = items.Find(placement.itemId);
            if (definition == nullptr)
            {
                LOG_ERROR("Unknown item id on level: " + placement.itemId);
                continue;
            }

            auto position = TileToWorldPosition(placement.column, placement.row, levelData.height);

            try
            {
                if (level.Add(CreateItem(*definition, position)))
                {
                    itemsCount++;
                }
            }
            catch (const std::exception& exception)
            {
                LOG_ERROR("Can't spawn item " + placement.itemId + ": " + exception.what());
            }
        }

        return itemsCount;
    }

    int LevelBuilder::BuildTiles(const LevelData& levelData, Level& level)
    {
        auto tilesObject = XYZEngine::GameWorld::Instance()->CreateGameObject("LevelTiles");
        tilesObject->SetRenderLayer(GROUND_RENDER_LAYER);
        level.Add(tilesObject);

        auto renderer = tilesObject->AddComponent<XYZEngine::VertexArrayRendererComponent>();
        const XYZEngine::Vector2Df tileSize = {TILE_SIZE, TILE_SIZE};

        for (int row = 0; row < levelData.height; row++)
        {
            for (int column = 0; column < (int)levelData.tiles[row].size(); column++)
            {
                TileType tile = levelData.tiles[row][column];
                if (tile == TileType::Empty)
                {
                    continue;
                }

                const sf::Color& color = tile == TileType::Wall ? WALL_COLOR : FLOOR_COLOR;
                renderer->AddQuad(TileToWorldPosition(column, row, levelData.height), tileSize, color);
            }
        }

        return (int)renderer->GetQuadsCount();
    }

    // The level file is read top to bottom, while the world axis Y points up.
    XYZEngine::Vector2Df LevelBuilder::TileToWorldPosition(int column, int row, int levelHeight)
    {
        assert(column >= 0 && row >= 0 && row < levelHeight);
        return {column * TILE_SIZE, (levelHeight - 1 - row) * TILE_SIZE};
    }
}
