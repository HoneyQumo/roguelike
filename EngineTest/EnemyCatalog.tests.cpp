#include "pch.h"
#include "EnemyCatalog.h"
#include <set>
#include <string>

using namespace RoguelikeGame;

TEST(EnemyCatalogTests, EveryEnemyIsFoundByItsTile)
{
	for (const EnemyDefinition& enemy : ENEMIES)
	{
		EXPECT_EQ(FindEnemy(enemy.tile), &enemy) << enemy.tileName;
		EXPECT_EQ(FindEnemyConfig(enemy.tile), &enemy.config) << enemy.tileName;
	}
}

TEST(EnemyCatalogTests, NonEnemyTilesHaveNoConfig)
{
	EXPECT_EQ(FindEnemyConfig(TileType::Empty), nullptr);
	EXPECT_EQ(FindEnemyConfig(TileType::Floor), nullptr);
	EXPECT_EQ(FindEnemyConfig(TileType::Wall), nullptr);
	EXPECT_EQ(FindEnemyConfig(TileType::PlayerSpawn), nullptr);
}

TEST(EnemyCatalogTests, TilesSymbolsAndNamesAreUnique)
{
	std::set<TileType> tiles;
	std::set<char> symbols;
	std::set<std::string> names;

	for (const EnemyDefinition& enemy : ENEMIES)
	{
		EXPECT_TRUE(tiles.insert(enemy.tile).second) << enemy.tileName;
		EXPECT_TRUE(symbols.insert(enemy.levelSymbol).second) << enemy.tileName;
		EXPECT_TRUE(names.insert(enemy.tileName).second) << enemy.tileName;
	}
}

TEST(EnemyCatalogTests, SymbolsDoNotClashWithBuiltInTiles)
{
	for (const EnemyDefinition& enemy : ENEMIES)
	{
		EXPECT_NE(enemy.levelSymbol, '#') << enemy.tileName;
		EXPECT_NE(enemy.levelSymbol, '.') << enemy.tileName;
		EXPECT_NE(enemy.levelSymbol, '@') << enemy.tileName;
		EXPECT_NE(enemy.levelSymbol, ' ') << enemy.tileName;
	}
}

TEST(EnemyCatalogTests, ConfigsAreSane)
{
	for (const EnemyDefinition& enemy : ENEMIES)
	{
		const EnemyConfig& config = enemy.config;
		EXPECT_STRNE(config.objectName, "") << enemy.tileName;
		EXPECT_STRNE(config.textureMapName, "") << enemy.tileName;
		EXPECT_GT(config.maxHealth, 0.f) << enemy.tileName;
		EXPECT_GT(config.speed, 0.f) << enemy.tileName;

		if (config.attackRange > 0.f)
		{
			EXPECT_GT(config.attackDamage, 0.f) << enemy.tileName;
			EXPECT_GT(config.attackCooldown, 0.f) << enemy.tileName;
		}

		if (!IsMelee(config.weapon) && config.attackRange > 0.f)
		{
			EXPECT_GT(config.projectileSpeed, 0.f) << enemy.tileName;
		}
	}
}

TEST(EnemyCatalogTest, EveryEnemyHasALootTable)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		ASSERT_NE(enemy.config.lootTable, nullptr) << enemy.tileName;
		EXPECT_STRNE(enemy.config.lootTable, "") << enemy.tileName;
	}
}

TEST(EnemyCatalogTest, MarauderIsTheWeakGunman)
{
	const RoguelikeGame::EnemyDefinition* marauder = RoguelikeGame::FindEnemy(RoguelikeGame::TileType::MarauderSpawn);
	const RoguelikeGame::EnemyDefinition* grunt = RoguelikeGame::FindEnemy(RoguelikeGame::TileType::GruntSpawn);

	ASSERT_NE(marauder, nullptr);
	ASSERT_NE(grunt, nullptr);

	EXPECT_EQ(marauder->levelSymbol, 'm');
	EXPECT_EQ(marauder->config.weapon, RoguelikeGame::WeaponId::Glock);
	EXPECT_LT(marauder->config.detectionRadius, grunt->config.detectionRadius);
	EXPECT_LT(marauder->config.maxHealth, grunt->config.maxHealth);
	EXPECT_STREQ(marauder->config.lootTable, "marauder");
}
