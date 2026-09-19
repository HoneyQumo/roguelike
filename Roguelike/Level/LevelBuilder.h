#pragma once

#include "ItemCatalog.h"
#include "PropCatalog.h"
#include "Level.h"
#include "LevelData.h"
#include "LevelZones.h"
#include <optional>
#include <Vector.h>

namespace XYZEngine
{
    class VertexArrayRendererComponent;
}

namespace RoguelikeGame
{
    class DoorComponent;
    class RoomWakeComponent;
    class TileFogComponent;

    class LevelBuilder
    {
    public:
        static Level Build(const LevelData& levelData, const ItemCatalog& items = ItemCatalog::Empty(),
            const PropCatalog& props = PropCatalog::Empty());

    private:
        static int BuildTiles(const LevelData& levelData, Level& level);
        static int BuildOverlay(const LevelData& levelData, Level& level);
        static void BuildFog(Level& level);
        static TileFogComponent* AddFog(XYZEngine::GameObject* chunk,
            XYZEngine::VertexArrayRendererComponent* renderer);
        static RoomWakeComponent* CreateRoomWake(const std::vector<LevelZone>& zones, Level& level);
        static void PutToSleep(RoomWakeComponent* rooms, const std::vector<LevelZone>& zones, int column, int row, XYZEngine::GameObject* enemy);
        static std::vector<DoorComponent*> BuildDoors(const LevelData& levelData, const ItemCatalog& items, Level& level);
        static int BuildFixtures(const LevelData& levelData, Level& level, const std::vector<DoorComponent*>& doors);
        static int BuildWaves(const LevelData& levelData, Level& level);
        static int BuildPursuit(const LevelData& levelData, Level& level);

        // Точки и способ рождения общие: волна и погоня отличаются тем, когда звать, а не кого.
        static std::vector<XYZEngine::Vector2Df> CollectSpawnPoints(const LevelData& levelData);
        static XYZEngine::GameObject* SpawnHunter(TileType enemy, const XYZEngine::Vector2Df& place, const FightStyle& style);
        static int BuildProps(const LevelData& levelData, const PropCatalog& props, const ItemCatalog& items, Level& level);
        static int BuildItems(const LevelData& levelData, const ItemCatalog& items, Level& level);
        static XYZEngine::GameObject* CreateBossObject(const LevelData& levelData, const XYZEngine::Vector2Df& position, Level& level);
        static void LockExit(Level& level);
        static XYZEngine::Vector2Df TileToWorldPosition(int column, int row, int levelHeight);
    };
}
