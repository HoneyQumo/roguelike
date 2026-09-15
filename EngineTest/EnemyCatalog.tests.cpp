#include "pch.h"
#include "BossEffects.h"
#include "EnemyCatalog.h"
#include "Vision.h"
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

namespace
{
	const RoguelikeGame::TileType BY_STRENGTH[] = {
		RoguelikeGame::TileType::GruntSpawn,
		RoguelikeGame::TileType::MarauderSpawn,
		RoguelikeGame::TileType::ShieldSpawn,
		RoguelikeGame::TileType::AssaultSpawn,
		RoguelikeGame::TileType::HeavySpawn,
		RoguelikeGame::TileType::RadioSpawn,
		RoguelikeGame::TileType::BossSpawn,
	};
}

TEST(EnemyCatalogTest, StrongerEnemySeesWider)
{
	float previous = 0.f;
	for (RoguelikeGame::TileType tile : BY_STRENGTH)
	{
		const RoguelikeGame::EnemyConfig* config = RoguelikeGame::FindEnemyConfig(tile);
		ASSERT_NE(config, nullptr);

		EXPECT_GT(config->visionHalfAngle, previous);
		previous = config->visionHalfAngle;
	}
}

TEST(EnemyCatalogTest, NobodySeesAllAround)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		EXPECT_LT(enemy.config.visionHalfAngle, 180.f) << enemy.tileName;
		EXPECT_LT(enemy.config.alertHalfAngle, 180.f) << enemy.tileName;
	}
}

TEST(EnemyCatalogTest, AlertWidensTheConeOfEveryEnemy)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		EXPECT_GT(enemy.config.alertHalfAngle, enemy.config.visionHalfAngle) << enemy.tileName;
		EXPECT_GT(enemy.config.alertTime, 0.f) << enemy.tileName;
	}
}

TEST(EnemyCatalogTest, EveryEnemyCanBeApproachedFromBehind)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		RoguelikeGame::VisionRange range;
		range.maxDistance = enemy.config.detectionRadius;
		range.calmHalfAngle = enemy.config.visionHalfAngle;
		range.alertHalfAngle = enemy.config.alertHalfAngle;

		XYZEngine::Vector2Df facing = {1.f, 0.f};
		XYZEngine::Vector2Df behind = {-0.5f * enemy.config.detectionRadius, 0.f};

		EXPECT_FALSE(RoguelikeGame::CanSeeTarget(RoguelikeGame::ConeFor(range, false), facing, behind, false)) << enemy.tileName;
		EXPECT_FALSE(RoguelikeGame::CanSeeTarget(RoguelikeGame::ConeFor(range, true), facing, behind, false)) << enemy.tileName;
	}
}

TEST(EnemyCatalogTest, StrongerEnemySearchesLonger)
{
	float previous = 0.f;
	for (RoguelikeGame::TileType tile : BY_STRENGTH)
	{
		const RoguelikeGame::EnemyConfig* config = RoguelikeGame::FindEnemyConfig(tile);
		ASSERT_NE(config, nullptr);

		EXPECT_GT(config->searchTime, previous);
		previous = config->searchTime;
	}
}

TEST(EnemyCatalogTest, EveryEnemyLooksForALostTarget)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		EXPECT_GT(enemy.config.searchTime, 0.f) << enemy.tileName;
	}
}

TEST(NoiseTest, ShootingEnemiesReachFurtherWhenProvoked)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		bool isMelee = enemy.config.weapon == RoguelikeGame::WeaponId::Knife;
		if (isMelee)
		{
			EXPECT_FLOAT_EQ(enemy.config.provokedRangeScale, 1.f) << enemy.config.objectName;
			continue;
		}

		EXPECT_GT(enemy.config.provokedRangeScale, 1.f) << enemy.config.objectName;
		EXPECT_GT(enemy.config.attackRange * enemy.config.provokedRangeScale, enemy.config.detectionRadius)
			<< enemy.config.objectName;
	}
}

TEST(NoiseTest, TheBossCallsSomebodyWhoShoots)
{
	const RoguelikeGame::EnemyConfig* minion = RoguelikeGame::FindEnemyConfig(RoguelikeGame::BOSS_MINION_TILE);

	ASSERT_NE(minion, nullptr);
	EXPECT_NE(minion->weapon, RoguelikeGame::WeaponId::Knife) << minion->objectName;
}

TEST(EnemyCatalogTest, ArmorIsEitherAbsentOrWorthShowing)
{
	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		if (enemy.config.armor <= 0.f)
		{
			continue;
		}

		EXPECT_GE(enemy.config.armor, 20.f) << enemy.config.objectName << " armor is gone in one burst";
		EXPECT_LE(enemy.config.armor, enemy.config.maxHealth) << enemy.config.objectName << " is more armor than flesh";
	}
}

TEST(EnemyCatalogTest, TheShieldCarriesTheMostArmorOfTheRankAndFile)
{
	const RoguelikeGame::EnemyConfig* shield = RoguelikeGame::FindEnemyConfig(RoguelikeGame::TileType::ShieldSpawn);
	ASSERT_NE(shield, nullptr);

	for (const RoguelikeGame::EnemyDefinition& enemy : RoguelikeGame::ENEMIES)
	{
		if (enemy.tile == RoguelikeGame::TileType::ShieldSpawn || enemy.tile == RoguelikeGame::TileType::BossSpawn)
		{
			continue;
		}

		EXPECT_LT(enemy.config.armor, shield->armor) << enemy.config.objectName << " out-armors the shield";
	}
}
