#include "LevelBuilder.h"
#include "BossCatalog.h"
#include "GameSettings.h"
#include "Enemy.h"
#include "EnemyCatalog.h"
#include "Item.h"
#include "LevelExit.h"
#include "LevelExitComponent.h"
#include "Wall.h"
#include <GameWorld.h>
#include <RectangleRendererComponent.h>
#include <VertexArrayRendererComponent.h>
#include <LoggerRegistry.h>
#include <cassert>

namespace RoguelikeGame
{
    Level LevelBuilder::Build(const LevelData& levelData, const ItemCatalog& items)
    {
        Level level;

        if (CountTiles(levelData, TileType::PlayerSpawn) == 0 && CountTiles(levelData, TileType::Entrance) == 0)
        {
            LOG_ERROR("Level has no player spawn point and no entrance, nothing is built");
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
                    case TileType::Entrance:
                        if (level.GetEntrance().has_value())
                        {
                            LOG_WARN("Level has more than one entrance, extra one at "
                                + std::to_string(column) + ";" + std::to_string(row) + " is ignored");
                        }
                        else
                        {
                            level.SetEntrance(position);
                        }
                        break;
                    case TileType::Exit:
                        if (level.GetExit() != nullptr)
                        {
                            LOG_WARN("Level has more than one exit, extra one at "
                                + std::to_string(column) + ";" + std::to_string(row) + " is ignored");
                        }
                        else
                        {
                            XYZEngine::GameObject* exitObject = CreateLevelExit(position);
                            level.Add(exitObject);
                            level.SetExit(exitObject);
                        }
                        break;
                    case TileType::BossSpawn:
                        if (level.GetBoss() != nullptr)
                        {
                            LOG_WARN("Level has more than one boss, extra one at "
                                + std::to_string(column) + ";" + std::to_string(row) + " is ignored");
                        }
                        else
                        {
                            level.Add(CreateBossObject(levelData, position, level));
                            enemiesCount++;
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

        level.SetInfo(levelData.info);

        if (!levelData.info.boss.IsEmpty() && level.GetBoss() == nullptr)
        {
            LOG_ERROR("Level declares boss " + levelData.info.boss.bossId + " but has no boss spawn tile");
        }

        LockExit(level);

        int itemsCount = BuildItems(levelData, items, level);

        LOG_INFO("Level built: tiles " + std::to_string(tilesCount)
            + ", walls " + std::to_string(wallsCount)
            + ", enemies " + std::to_string(enemiesCount)
            + ", items " + std::to_string(itemsCount));

        return level;
    }

    XYZEngine::GameObject* LevelBuilder::CreateBossObject(const LevelData& levelData, const XYZEngine::Vector2Df& position, Level& level)
    {
        EnemyConfig config = ApplyBossScales(*FindEnemyConfig(TileType::BossSpawn),
            levelData.info.boss.healthScale, levelData.info.boss.damageScale);

        const BossDefinition* definition = levelData.info.boss.IsEmpty() ? nullptr : FindBoss(levelData.info.boss.bossId);
        if (definition == nullptr)
        {
            if (!levelData.info.boss.IsEmpty())
            {
                LOG_ERROR("Unknown boss id in level: " + levelData.info.boss.bossId);
            }

            return CreateEnemy(config, position);
        }

        XYZEngine::GameObject* bossObject = CreateBoss(config, *definition, position);
        level.SetBoss(bossObject);

        return bossObject;
    }

    void LevelBuilder::LockExit(Level& level)
    {
        if (level.GetBoss() == nullptr || level.GetExit() == nullptr)
        {
            return;
        }

        auto exitComponent = level.GetExit()->GetComponent<LevelExitComponent>();
        if (exitComponent != nullptr)
        {
            exitComponent->SetLocked(true);
        }

        auto renderer = level.GetExit()->GetComponent<XYZEngine::RectangleRendererComponent>();
        if (renderer != nullptr)
        {
            renderer->SetColor(LEVEL_EXIT_LOCKED_COLOR);
        }

        LOG_INFO("Level exit is locked until the boss is defeated");
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
