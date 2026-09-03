#pragma once

#include <memory>
#include <vector>
#include "LevelData.h"
#include "Player.h"
#include "Enemy.h"
#include "Wall.h"
#include <VertexArrayRendererComponent.h>

namespace RoguelikeGame
{
    class LevelBuilder
    {
    public:
        void Build(const LevelData& levelData);
        void Clear();

        Player* GetPlayer() const;

    private:
        std::vector<std::unique_ptr<Wall>> walls;
        std::vector<std::unique_ptr<Enemy>> enemies;
        std::unique_ptr<Player> player;

        XYZEngine::GameObject* tilesObject = nullptr;
        int tilesCount = 0;

        void BuildTiles(const LevelData& levelData);
        static XYZEngine::Vector2Df TileToWorldPosition(int column, int row, int levelHeight);
    };
}
