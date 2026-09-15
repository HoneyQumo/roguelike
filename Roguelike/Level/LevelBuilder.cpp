#include "LevelBuilder.h"
#include "TileAtlas.h"
#include "LevelGrid.h"
#include "PathService.h"
#include "PatrolRoutes.h"
#include <ResourceSystem.h>
#include "BossCatalog.h"
#include "GameSettings.h"
#include "Enemy.h"
#include "EnemyCatalog.h"
#include "Item.h"
#include "Prop.h"
#include "LevelExit.h"
#include "LevelExitComponent.h"
#include "Wall.h"
#include "Door.h"
#include "DoorComponent.h"
#include "RoomWakeComponent.h"
#include <GameWorld.h>
#include <RectangleRendererComponent.h>
#include <VertexArrayRendererComponent.h>
#include <LoggerRegistry.h>
#include <cassert>

namespace RoguelikeGame
{
    Level LevelBuilder::Build(const LevelData& levelData, const ItemCatalog& items, const PropCatalog& props)
    {
        LevelGrid::SetCurrent(LevelGrid::Build(levelData));
        PathService::Reset();
        PatrolRoutes::SetCurrent(PatrolRoutes::Build(levelData, LevelGrid::Current()));

        Level level;

        if (CountTiles(levelData, TileType::PlayerSpawn) == 0 && CountTiles(levelData, TileType::Entrance) == 0)
        {
            LOG_ERROR("Level has no player spawn point and no entrance, nothing is built");
            return level;
        }

        int tilesCount = BuildTiles(levelData, level);
        int wallsCount = 0;
        int enemiesCount = 0;

        std::vector<LevelZone> zones = BuildZones(levelData);
        RoomWakeComponent* rooms = CreateRoomWake(zones, level);

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
                            XYZEngine::GameObject* bossObject = CreateBossObject(levelData, position, level);
                            level.Add(bossObject);
                            PutToSleep(rooms, zones, column, row, bossObject);
                            enemiesCount++;
                        }
                        break;
                    default:
                        if (const EnemyConfig* config = FindEnemyConfig(tile))
                        {
                            XYZEngine::GameObject* enemyObject = CreateEnemy(*config, position);
                            level.Add(enemyObject);
                            PutToSleep(rooms, zones, column, row, enemyObject);
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
        int propsCount = BuildProps(levelData, props, items, level);
        int doorsCount = BuildDoors(levelData, items, level);

        LOG_INFO("Level built: tiles " + std::to_string(tilesCount)
            + ", walls " + std::to_string(wallsCount)
            + ", enemies " + std::to_string(enemiesCount)
            + ", items " + std::to_string(itemsCount)
            + ", props " + std::to_string(propsCount)
            + ", doors " + std::to_string(doorsCount)
            + ", asleep " + std::to_string(rooms != nullptr ? rooms->GetSleepingCount() : 0));

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

    RoomWakeComponent* LevelBuilder::CreateRoomWake(const std::vector<LevelZone>& zones, Level& level)
    {
        if (zones.empty())
        {
            return nullptr;
        }

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(ROOMS_OBJECT_NAME);
        level.Add(gameObject);

        auto rooms = gameObject->AddComponent<RoomWakeComponent>();
        rooms->SetTargetName(PLAYER_OBJECT_NAME);
        rooms->SetZones(zones);

        return rooms;
    }

    void LevelBuilder::PutToSleep(RoomWakeComponent* rooms, const std::vector<LevelZone>& zones, int column, int row,
        XYZEngine::GameObject* enemy)
    {
        if (rooms == nullptr)
        {
            return;
        }

        const LevelZone* zone = FindZoneAt(zones, column, row);
        if (zone != nullptr)
        {
            rooms->AddSleeper(zone->id, enemy);
        }
    }

    int LevelBuilder::BuildDoors(const LevelData& levelData, const ItemCatalog& items, Level& level)
    {
        std::vector<DoorComponent*> doors;

        for (const DoorPlacement& placement : levelData.doors)
        {
            auto position = TileToWorldPosition(placement.column, placement.row, levelData.height);

            XYZEngine::GameObject* gameObject = CreateDoor(placement.doorId, position, items);
            if (level.Add(gameObject))
            {
                doors.push_back(gameObject->GetComponent<DoorComponent>());
            }
        }

        LinkDoors(doors);

        return static_cast<int>(doors.size());
    }

    int LevelBuilder::BuildProps(const LevelData& levelData, const PropCatalog& props, const ItemCatalog& items, Level& level)
    {
        int propsCount = 0;

        for (const PropPlacement& placement : levelData.props)
        {
            const PropDefinition* definition = props.Find(placement.propId);
            if (definition == nullptr)
            {
                LOG_ERROR("Unknown prop id on level: " + placement.propId);
                continue;
            }

            auto position = TileToWorldPosition(placement.column, placement.row, levelData.height);

            try
            {
                if (level.Add(CreateProp(*definition, position, items)))
                {
                    propsCount++;
                }
            }
            catch (const std::exception& exception)
            {
                LOG_ERROR("Can't spawn prop " + placement.propId + ": " + exception.what());
            }
        }

        return propsCount;
    }

    int LevelBuilder::BuildTiles(const LevelData& levelData, Level& level)
    {
        auto tilesObject = XYZEngine::GameWorld::Instance()->CreateGameObject("LevelTiles");
        tilesObject->SetRenderLayer(GROUND_RENDER_LAYER);
        level.Add(tilesObject);

        auto renderer = tilesObject->AddComponent<XYZEngine::VertexArrayRendererComponent>();
        const XYZEngine::Vector2Df tileSize = {TILE_SIZE, TILE_SIZE};

        const sf::Texture* tiles = XYZEngine::ResourceSystem::Instance()->GetTextureShared(TILES_TEXTURE);
        renderer->SetTexture(tiles);

        for (int row = 0; row < levelData.height; row++)
        {
            for (int column = 0; column < (int)levelData.tiles[row].size(); column++)
            {
                TileType tile = levelData.tiles[row][column];
                if (tile == TileType::Empty)
                {
                    continue;
                }

                auto position = TileToWorldPosition(column, row, levelData.height);

                if (tiles != nullptr)
                {
                    renderer->AddQuad(position, tileSize, TileFrameFor(levelData, column, row));
                    continue;
                }

                renderer->AddQuad(position, tileSize, tile == TileType::Wall ? WALL_COLOR : FLOOR_COLOR);
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
