#include "LevelBuilder.h"
#include "GameSettings.h"

namespace RoguelikeGame
{
	void LevelBuilder::Build(const LevelData& levelData)
	{
		Clear();

		// Game objects are rendered in creation order, so the floor goes first and never covers walls.
		for (int row = 0; row < levelData.height; row++)
		{
			for (int column = 0; column < (int)levelData.tiles[row].size(); column++)
			{
				TileType tile = levelData.tiles[row][column];
				if (tile == TileType::Empty || tile == TileType::Wall)
				{
					continue;
				}

				floors.push_back(std::make_unique<Floor>(TileToWorldPosition(column, row, levelData.height)));
			}
		}

		for (int row = 0; row < levelData.height; row++)
		{
			for (int column = 0; column < (int)levelData.tiles[row].size(); column++)
			{
				auto position = TileToWorldPosition(column, row, levelData.height);

				switch (levelData.tiles[row][column])
				{
				case TileType::Wall:
					walls.push_back(std::make_unique<Wall>(position));
					break;
				case TileType::PlayerSpawn:
					player = std::make_unique<Player>(position);
					break;
				case TileType::EnemySpawn:
					enemies.push_back(std::make_unique<Enemy>(position));
					break;
				default:
					break;
				}
			}
		}
	}

	void LevelBuilder::Clear()
	{
		player.reset();
		enemies.clear();
		walls.clear();
		floors.clear();
	}

	Player* LevelBuilder::GetPlayer() const
	{
		return player.get();
	}

	// The level file is read top to bottom, while the world axis Y points up.
	XYZEngine::Vector2Df LevelBuilder::TileToWorldPosition(int column, int row, int levelHeight)
	{
		return { column * TILE_SIZE, (levelHeight - 1 - row) * TILE_SIZE };
	}
}
