#include "LevelBuilder.h"
#include "Chasm.h"
#include "GameResources.h"
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
#include "GameResources.h"
#include "HealthComponent.h"
#include "DamageInfo.h"
#include "Prop.h"
#include "GuardFacing.h"
#include "PropAlign.h"
#include "TileAnimationComponent.h"
#include "TileFogComponent.h"
#include "FogOfWar.h"
#include "FogRevealComponent.h"
#include "PursuitComponent.h"
#include "WaveDirectorComponent.h"
#include "LevelExit.h"
#include "LevelExitComponent.h"
#include "Wall.h"
#include "Door.h"
#include "Fixture.h"
#include "EscapeCarComponent.h"
#include "HatchComponent.h"
#include "Openable.h"
#include "SwitchComponent.h"
#include "LevelExitComponent.h"
#include "DoorComponent.h"
#include "RoomWakeComponent.h"
#include "Tileset.h"
#include <GameWorld.h>
#include <RectangleRendererComponent.h>
#include <VertexArrayRendererComponent.h>
#include <LoggerRegistry.h>
#include <algorithm>
#include <cassert>
#include <MathUtils.h>
#include <optional>

namespace RoguelikeGame
{
    Level LevelBuilder::Build(const LevelData& levelData, const ItemCatalog& items, const PropCatalog& props)
    {
        LevelGrid::SetCurrent(LevelGrid::Build(levelData, GameResources::GetProps()));
        FogOfWar::Reset(levelData.width, levelData.height, levelData.info.fogRadius);
        PathService::Reset();
        PatrolRoutes::SetCurrent(PatrolRoutes::Build(levelData, LevelGrid::Current()));

        Level level;

        if (CountTiles(levelData, TileType::PlayerSpawn) == 0 && CountTiles(levelData, TileType::Entrance) == 0)
        {
            LOG_ERROR("Level has no player spawn point and no entrance, nothing is built");
            return level;
        }

        BuildFog(level);

        int tilesCount = BuildTiles(levelData, level);
        int overlayCount = BuildOverlay(levelData, level);
        int wallsCount = 0;
        int enemiesCount = 0;

        std::optional<float> guardFacing = FacingAgainstTheRun(levelData);
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
                    case TileType::Water:
                    case TileType::Empty:
                        if (IsChasmEdge(levelData, column, row))
                        {
                            level.Add(CreateWall(position));
                            wallsCount++;
                        }
                        break;
                    case TileType::WaveSpawn:
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
                            ApplyFightStyle(enemyObject, levelData.info.style);

                            // Заслон ждёт бегущего: стоять к нему спиной ему незачем.
                            if (guardFacing.has_value())
                            {
                                enemyObject->GetTransform()->SetWorldRotation(*guardFacing);
                            }

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
        std::vector<DoorComponent*> doors = BuildDoors(levelData, items, level);
        int fixturesCount = BuildFixtures(levelData, level, doors);
        int wavesCount = BuildWaves(levelData, level);
        int pursuitCount = BuildPursuit(levelData, level);

        LOG_INFO("Level built: tiles " + std::to_string(tilesCount)
            + ", overlay " + std::to_string(overlayCount)
            + ", walls " + std::to_string(wallsCount)
            + ", enemies " + std::to_string(enemiesCount)
            + ", items " + std::to_string(itemsCount)
            + ", props " + std::to_string(propsCount)
            + ", doors " + std::to_string(doors.size())
            + ", fixtures " + std::to_string(fixturesCount)
            + ", waves " + std::to_string(wavesCount)
            + ", pursuit " + std::to_string(pursuitCount)
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

        const ItemDefinition* prize = levelData.info.boss.drop.empty()
            ? nullptr
            : GameResources::GetItems().Find(levelData.info.boss.drop);

        if (!levelData.info.boss.drop.empty() && prize == nullptr)
        {
            LOG_ERROR("Level boss drops unknown item: " + levelData.info.boss.drop);
        }

        if (prize != nullptr)
        {
            auto health = bossObject->GetComponent<HealthComponent>();
            if (health != nullptr)
            {
                health->SubscribeDeath([bossObject, prize](const DeathInfo& death)
                {
                    if (CreateItem(*prize, death.position, bossObject) != nullptr)
                    {
                        LOG_INFO("Boss dropped " + prize->id);
                    }
                });
            }
        }

        return bossObject;
    }

    void LevelBuilder::LockExit(Level& level)
    {
        bool isGuarded = level.GetBoss() != nullptr || level.GetWaveDirector() != nullptr;
        if (!isGuarded || level.GetExit() == nullptr)
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
        rooms->SetAhead(ROOM_WAKE_AHEAD);

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

    std::vector<DoorComponent*> LevelBuilder::BuildDoors(const LevelData& levelData, const ItemCatalog& items, Level& level)
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

        return doors;
    }

    std::vector<XYZEngine::Vector2Df> LevelBuilder::CollectSpawnPoints(const LevelData& levelData)
    {
        std::vector<XYZEngine::Vector2Df> points;

        for (int row = 0; row < levelData.height; row++)
        {
            for (int column = 0; column < static_cast<int>(levelData.tiles[row].size()); column++)
            {
                if (levelData.tiles[row][column] == TileType::WaveSpawn)
                {
                    points.push_back(TileToWorldPosition(column, row, levelData.height));
                }
            }
        }

        return points;
    }

    // Кого ни зови - волну или погоню, - он приходит по душу игрока, а не стоять в карауле.
    XYZEngine::GameObject* LevelBuilder::SpawnHunter(TileType enemy, const XYZEngine::Vector2Df& place, const FightStyle& style)
    {
        const EnemyConfig* config = FindEnemyConfig(enemy);
        if (config == nullptr)
        {
            return nullptr;
        }

        XYZEngine::GameObject* born = CreateEnemy(*config, place);
        ApplyFightStyle(born, style);
        SendAfterPlayer(born, place);

        return born;
    }

    int LevelBuilder::BuildPursuit(const LevelData& levelData, Level& level)
    {
        if (levelData.pursuit.IsEmpty())
        {
            return 0;
        }

        std::vector<XYZEngine::Vector2Df> points = CollectSpawnPoints(levelData);
        if (points.empty())
        {
            LOG_ERROR("Level has a pursuit but no spawn points");
            return 0;
        }

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(PURSUIT_OBJECT_NAME);
        auto pursuit = gameObject->AddComponent<PursuitComponent>();
        pursuit->SetSpec(levelData.pursuit);
        pursuit->SetPoints(points);
        FightStyle pursuitStyle = levelData.pursuit.style;
        pursuit->SetSpawner([pursuitStyle](TileType enemy, const XYZEngine::Vector2Df& place)
        {
            return SpawnHunter(enemy, place, pursuitStyle);
        });

        level.Add(gameObject);
        level.SetPursuit(gameObject);

        LOG_INFO("Pursuit ready: keeps " + std::to_string(levelData.pursuit.keep)
            + ", grows to " + std::to_string(levelData.pursuit.grow)
            + " over " + std::to_string(points.size()) + " points");

        return levelData.pursuit.keep;
    }

    int LevelBuilder::BuildWaves(const LevelData& levelData, Level& level)
    {
        if (levelData.waves.empty())
        {
            return 0;
        }

        std::vector<XYZEngine::Vector2Df> points = CollectSpawnPoints(levelData);
        if (points.empty())
        {
            LOG_ERROR("Level has waves but no spawn points");
            return 0;
        }

        auto gameObject = XYZEngine::GameWorld::Instance()->CreateGameObject(WAVE_DIRECTOR_OBJECT_NAME);
        auto director = gameObject->AddComponent<WaveDirectorComponent>();
        director->SetWaves(levelData.waves);
        director->SetPoints(points);
        FightStyle wavesStyle = levelData.wavesStyle;
        director->SetSpawner([wavesStyle](TileType enemy, const XYZEngine::Vector2Df& place)
        {
            return SpawnHunter(enemy, place, wavesStyle);
        });

        level.Add(gameObject);
        level.SetWaveDirector(gameObject);

        LOG_INFO("Waves ready: " + std::to_string(levelData.waves.size()) + " over "
            + std::to_string(points.size()) + " points");

        return static_cast<int>(levelData.waves.size());
    }

    int LevelBuilder::BuildFixtures(const LevelData& levelData, Level& level, const std::vector<DoorComponent*>& doors)
    {
        std::vector<SwitchComponent*> levers;
        std::vector<HatchComponent*> hatches;

        for (const FixturePlacement& placement : levelData.hatches)
        {
            auto position = TileToWorldPosition(placement.column, placement.row, levelData.height);

            XYZEngine::GameObject* gameObject = CreateHatch(placement.id, position);
            if (level.Add(gameObject))
            {
                hatches.push_back(gameObject->GetComponent<HatchComponent>());
            }
        }

        for (const FixturePlacement& placement : levelData.escapes)
        {
            auto position = TileToWorldPosition(placement.column, placement.row, levelData.height);

            XYZEngine::GameObject* car = CreateEscapeCar(placement.id, position);
            if (!level.Add(car))
            {
                continue;
            }

            level.SetEscapeCar(car);

            if (!levelData.waves.empty())
            {
                car->GetComponent<EscapeCarComponent>()->SetReady(false);
            }
        }

        for (const FixturePlacement& placement : levelData.levers)
        {
            auto position = TileToWorldPosition(placement.column, placement.row, levelData.height);

            XYZEngine::GameObject* gameObject = CreateLever(placement.id, position);
            if (level.Add(gameObject))
            {
                levers.push_back(gameObject->GetComponent<SwitchComponent>());
            }
        }

        // Рычаг открывает всё, у чего тот же id: и люк, и дверь за фальшивой стеной.
        std::vector<Openable> openables;
        AddOpenables(openables, hatches);
        AddOpenables(openables, doors);
        LinkSwitches(levers, openables);

        if (!hatches.empty() && level.GetExit() == nullptr)
        {
            XYZEngine::GameObject* hidden = CreateLevelExit(
                hatches.front()->GetGameObject()->GetTransform()->GetWorldPosition());

            for (XYZEngine::Component* part : hidden->GetComponents<XYZEngine::Component>())
            {
                if (dynamic_cast<XYZEngine::RectangleRendererComponent*>(part) != nullptr)
                {
                    part->SetEnabled(false);
                }
            }

            level.Add(hidden);
            level.SetExit(hidden);
        }

        if (!hatches.empty() && level.GetExit() != nullptr)
        {
            auto exitComponent = level.GetExit()->GetComponent<LevelExitComponent>();
            if (exitComponent != nullptr)
            {
                exitComponent->SetLocked(true);
                exitComponent->SetManual(true);
                for (HatchComponent* hatch : hatches)
                {
                    hatch->SubscribeFled([exitComponent]() { exitComponent->Use(); });
                }

                LOG_INFO("Level exit is behind a hatch");
            }
        }

        return static_cast<int>(levers.size() + hatches.size() + levelData.escapes.size());
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
                XYZEngine::GameObject* gameObject = CreateProp(*definition, position, items);

                if (gameObject != nullptr && definition->isPanel)
                {
                    PanelSupport support = ReadPanelSupport(levelData, placement.propId, placement.column, placement.row);
                    gameObject->GetTransform()->SetWorldRotation(PanelAngle(support));
                }
                else if (gameObject != nullptr)
                {
                    gameObject->GetTransform()->SetWorldRotation(PropAngle(*definition, placement));
                }

                if (level.Add(gameObject))
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

    /**
    *	Верхний слой: накладка поверх пола. Отдельный массив вершин и один
    *	лишний вызов отрисовки на всю карту - зато клетка перестаёт быть
    *	«либо асфальт, либо вода» и под дырой видно то, что лежит ниже.
    *
    *	Проходимость слой не меняет: коллизии и сетка путей строятся по нижнему.
    */
    /**
    *	Туман живёт отдельным объектом уровня, а не компонентом игрока:
    *	в катсценах игроку выключают все компоненты, и туман застыл бы вместе с ним.
    */
    void LevelBuilder::BuildFog(Level& level)
    {
        if (!FogOfWar::Current().IsEnabled())
        {
            return;
        }

        auto fogObject = XYZEngine::GameWorld::Instance()->CreateGameObject("Fog");
        level.Add(fogObject);

        fogObject->AddComponent<FogRevealComponent>()->SetTargetName(PLAYER_OBJECT_NAME);
    }

    TileFogComponent* LevelBuilder::AddFog(XYZEngine::GameObject* chunk,
        XYZEngine::VertexArrayRendererComponent* renderer)
    {
        if (!FogOfWar::Current().IsEnabled())
        {
            return nullptr;
        }

        TileFogComponent* fog = chunk->AddComponent<TileFogComponent>();
        fog->SetRenderer(renderer);

        return fog;
    }

    int LevelBuilder::BuildOverlay(const LevelData& levelData, Level& level)
    {
        if (levelData.overlay.empty())
        {
            return 0;
        }

        const sf::Texture* tiles = LoadTileset(levelData.info.tileset);
        if (tiles == nullptr)
        {
            return 0;
        }

        const XYZEngine::Vector2Df tileSize = {TILE_SIZE, TILE_SIZE};
        int total = 0;

        for (int chunkRow = 0; chunkRow < levelData.height; chunkRow += TILE_CHUNK)
        {
            for (int chunkColumn = 0; chunkColumn < levelData.width; chunkColumn += TILE_CHUNK)
            {
                XYZEngine::VertexArrayRendererComponent* renderer = nullptr;
                TileFogComponent* fog = nullptr;

                for (int row = chunkRow; row < std::min(chunkRow + TILE_CHUNK, levelData.height); row++)
                {
                    for (int column = chunkColumn; column < std::min(chunkColumn + TILE_CHUNK, levelData.width); column++)
                    {
                        if (OverlayAt(levelData, column, row) == TileType::Empty)
                        {
                            continue;
                        }

                        if (renderer == nullptr)
                        {
                            auto overlayObject = XYZEngine::GameWorld::Instance()->CreateGameObject("LevelOverlay");
                            overlayObject->SetRenderLayer(OVERLAY_RENDER_LAYER);
                            level.Add(overlayObject);

                            renderer = overlayObject->AddComponent<XYZEngine::VertexArrayRendererComponent>();
                            renderer->SetTexture(tiles);

                            fog = AddFog(overlayObject, renderer);
                        }

                        if (fog != nullptr)
                        {
                            fog->AddCell(renderer->GetQuadsCount(), column, row, sf::Color::White);
                        }

                        renderer->AddQuad(TileToWorldPosition(column, row, levelData.height), tileSize,
                            OverlayFrameFor(levelData, column, row));
                    }
                }

                total += renderer != nullptr ? static_cast<int>(renderer->GetQuadsCount()) : 0;
            }
        }

        return total;
    }

    int LevelBuilder::BuildTiles(const LevelData& levelData, Level& level)
    {
        const sf::Texture* tiles = LoadTileset(levelData.info.tileset);
        const XYZEngine::Vector2Df tileSize = {TILE_SIZE, TILE_SIZE};
        int total = 0;
        int chunks = 0;

        for (int chunkRow = 0; chunkRow < levelData.height; chunkRow += TILE_CHUNK)
        {
            for (int chunkColumn = 0; chunkColumn < levelData.width; chunkColumn += TILE_CHUNK)
            {
                XYZEngine::VertexArrayRendererComponent* renderer = nullptr;
                TileAnimationComponent* water = nullptr;
                TileFogComponent* fog = nullptr;

                for (int row = chunkRow; row < std::min(chunkRow + TILE_CHUNK, levelData.height); row++)
                {
                    int rowWidth = static_cast<int>(levelData.tiles[row].size());
                    for (int column = chunkColumn; column < std::min(chunkColumn + TILE_CHUNK, rowWidth); column++)
                    {
                        TileType tile = levelData.tiles[row][column];
                        if (tile == TileType::Empty)
                        {
                            continue;
                        }

                        if (renderer == nullptr)
                        {
                            auto tilesObject = XYZEngine::GameWorld::Instance()->CreateGameObject("LevelTiles");
                            tilesObject->SetRenderLayer(GROUND_RENDER_LAYER);
                            level.Add(tilesObject);

                            renderer = tilesObject->AddComponent<XYZEngine::VertexArrayRendererComponent>();
                            renderer->SetTexture(tiles);

                            // Вода живёт в массиве своего куска: помним её квады, чтобы менять им кадр.
                            water = tilesObject->AddComponent<TileAnimationComponent>();
                            water->SetRenderer(renderer);
                            water->SetStrip(TILE_WATER_ROW, TILE_FLOOR_FRAMES, WATER_FRAME_TIME);

                            fog = AddFog(tilesObject, renderer);
                        }

                        auto position = TileToWorldPosition(column, row, levelData.height);
                        sf::Color base = tiles != nullptr ? sf::Color::White
                            : tile == TileType::Wall ? WALL_COLOR : FLOOR_COLOR;

                        if (fog != nullptr)
                        {
                            fog->AddCell(renderer->GetQuadsCount(), column, row, base);
                        }

                        if (tiles != nullptr)
                        {
                            if (tile == TileType::Water)
                            {
                                water->AddCell(renderer->GetQuadsCount(), TileHash(column, row));
                            }

                            renderer->AddQuad(position, tileSize, TileFrameFor(levelData, column, row));
                            continue;
                        }

                        renderer->AddQuad(position, tileSize, base);
                    }
                }

                if (renderer != nullptr)
                {
                    total += static_cast<int>(renderer->GetQuadsCount());
                    chunks++;
                }
            }
        }

        LOG_INFO("Level tiles: " + std::to_string(total) + " quads in " + std::to_string(chunks) + " chunks");

        return total;
    }

    // The level file is read top to bottom, while the world axis Y points up.
    XYZEngine::Vector2Df LevelBuilder::TileToWorldPosition(int column, int row, int levelHeight)
    {
        assert(column >= 0 && row >= 0 && row < levelHeight);
        return {column * TILE_SIZE, (levelHeight - 1 - row) * TILE_SIZE};
    }
}
