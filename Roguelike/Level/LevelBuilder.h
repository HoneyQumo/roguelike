#pragma once

#include "ItemCatalog.h"
#include "PropCatalog.h"
#include "Level.h"
#include "LevelData.h"
#include "LevelZones.h"
#include <Vector.h>

namespace RoguelikeGame
{
    class RoomWakeComponent;

    class LevelBuilder
    {
    public:
        static Level Build(const LevelData& levelData, const ItemCatalog& items = ItemCatalog::Empty(),
            const PropCatalog& props = PropCatalog::Empty());

    private:
        static int BuildTiles(const LevelData& levelData, Level& level);
        static RoomWakeComponent* CreateRoomWake(const std::vector<LevelZone>& zones, Level& level);
        static void PutToSleep(RoomWakeComponent* rooms, const std::vector<LevelZone>& zones, int column, int row, XYZEngine::GameObject* enemy);
        static int BuildDoors(const LevelData& levelData, const ItemCatalog& items, Level& level);
        static int BuildFixtures(const LevelData& levelData, Level& level);
        static int BuildProps(const LevelData& levelData, const PropCatalog& props, const ItemCatalog& items, Level& level);
        static int BuildItems(const LevelData& levelData, const ItemCatalog& items, Level& level);
        static XYZEngine::GameObject* CreateBossObject(const LevelData& levelData, const XYZEngine::Vector2Df& position, Level& level);
        static void LockExit(Level& level);
        static XYZEngine::Vector2Df TileToWorldPosition(int column, int row, int levelHeight);
    };
}
